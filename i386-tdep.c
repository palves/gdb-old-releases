/* Intel 386 stuff.
   Copyright (C) 1988, 1989 Free Software Foundation, Inc.

This file is part of GDB.

GDB is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GDB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GDB; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <stdio.h>
#include "defs.h"
#include "param.h"
#include "frame.h"
#include "inferior.h"
#include "gdbcore.h"

#ifdef USG
#include <sys/types.h>
#endif

#include <sys/param.h>
#include <sys/dir.h>
#include <signal.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#ifndef N_SET_MAGIC
#ifdef COFF_FORMAT
#define N_SET_MAGIC(exec, val) ((exec).magic = (val))
#else
#define N_SET_MAGIC(exec, val) ((exec).a_magic = (val))
#endif
#endif

#include <sys/file.h>
#include <sys/stat.h>

/* I don't know whether this is right for cross-debugging even if you
   do somehow manage to get the right include file.  */
#if defined (USE_MACHINE_REG_H)
#include <machine/reg.h>
#else
#include <sys/reg.h>
#endif

extern int errno;

/* helper functions for m-i386.h */

/* stdio style buffering to minimize calls to ptrace */
static CORE_ADDR codestream_next_addr;
static CORE_ADDR codestream_addr;
static unsigned char codestream_buf[sizeof (int)];
static int codestream_off;
static int codestream_cnt;

#define codestream_tell() (codestream_addr + codestream_off)
#define codestream_peek() (codestream_cnt == 0 ? \
			   codestream_fill(1): codestream_buf[codestream_off])
#define codestream_get() (codestream_cnt-- == 0 ? \
			 codestream_fill(0) : codestream_buf[codestream_off++])

static unsigned char 
codestream_fill (peek_flag)
{
  codestream_addr = codestream_next_addr;
  codestream_next_addr += sizeof (int);
  codestream_off = 0;
  codestream_cnt = sizeof (int);
  read_memory (codestream_addr,
	       (unsigned char *)codestream_buf,
	       sizeof (int));
  
  if (peek_flag)
    return (codestream_peek());
  else
    return (codestream_get());
}

static void
codestream_seek (place)
{
  codestream_next_addr = place & -sizeof (int);
  codestream_cnt = 0;
  codestream_fill (1);
  while (codestream_tell() != place)
    codestream_get ();
}

static void
codestream_read (buf, count)
     unsigned char *buf;
{
  unsigned char *p;
  int i;
  p = buf;
  for (i = 0; i < count; i++)
    *p++ = codestream_get ();
}

/* next instruction is a jump, move to target */
static
i386_follow_jump ()
{
  int long_delta;
  short short_delta;
  char byte_delta;
  int data16;
  int pos;
  
  pos = codestream_tell ();
  
  data16 = 0;
  if (codestream_peek () == 0x66)
    {
      codestream_get ();
      data16 = 1;
    }
  
  switch (codestream_get ())
    {
    case 0xe9:
      /* relative jump: if data16 == 0, disp32, else disp16 */
      if (data16)
	{
	  codestream_read ((unsigned char *)&short_delta, 2);
	  pos += short_delta + 3; /* include size of jmp inst */
	}
      else
	{
	  codestream_read ((unsigned char *)&long_delta, 4);
	  pos += long_delta + 5;
	}
      break;
    case 0xeb:
      /* relative jump, disp8 (ignore data16) */
      codestream_read ((unsigned char *)&byte_delta, 1);
      pos += byte_delta + 2;
      break;
    }
  codestream_seek (pos + data16);
}

/*
 * find & return amound a local space allocated, and advance codestream to
 * first register push (if any)
 *
 * if entry sequence doesn't make sense, return -1, and leave 
 * codestream pointer random
 */
static long
i386_get_frame_setup (pc)
{
  unsigned char op;
  
  codestream_seek (pc);
  
  i386_follow_jump ();
  
  op = codestream_get ();
  
  if (op == 0x58)		/* popl %eax */
    {
      /*
       * this function must start with
       * 
       *    popl %eax		  0x58
       *    xchgl %eax, (%esp)  0x87 0x04 0x24
       * or xchgl %eax, 0(%esp) 0x87 0x44 0x24 0x00
       *
       * (the system 5 compiler puts out the second xchg
       * inst, and the assembler doesn't try to optimize it,
       * so the 'sib' form gets generated)
       * 
       * this sequence is used to get the address of the return
       * buffer for a function that returns a structure
       */
      int pos;
      unsigned char buf[4];
      static unsigned char proto1[3] = { 0x87,0x04,0x24 };
      static unsigned char proto2[4] = { 0x87,0x44,0x24,0x00 };
      pos = codestream_tell ();
      codestream_read (buf, 4);
      if (bcmp (buf, proto1, 3) == 0)
	pos += 3;
      else if (bcmp (buf, proto2, 4) == 0)
	pos += 4;
      
      codestream_seek (pos);
      op = codestream_get (); /* update next opcode */
    }
  
  if (op == 0x55)		/* pushl %ebp */
    {			
      /* check for movl %esp, %ebp - can be written two ways */
      switch (codestream_get ())
	{
	case 0x8b:
	  if (codestream_get () != 0xec)
	    return (-1);
	  break;
	case 0x89:
	  if (codestream_get () != 0xe5)
	    return (-1);
	  break;
	default:
	  return (-1);
	}
      /* check for stack adjustment 
       *
       *  subl $XXX, %esp
       *
       * note: you can't subtract a 16 bit immediate
       * from a 32 bit reg, so we don't have to worry
       * about a data16 prefix 
       */
      op = codestream_peek ();
      if (op == 0x83)
	{
	  /* subl with 8 bit immed */
	  codestream_get ();
	  if (codestream_get () != 0xec)
	    /* Some instruction starting with 0x83 other than subl.  */
	    {
	      codestream_seek (codestream_tell () - 2);
	      return 0;
	    }
	  /* subl with signed byte immediate 
	   * (though it wouldn't make sense to be negative)
	   */
	  return (codestream_get());
	}
      else if (op == 0x81)
	{
	  /* subl with 32 bit immed */
	  int locals;
	  codestream_get();
	  if (codestream_get () != 0xec)
	    /* Some instruction starting with 0x81 other than subl.  */
	    {
	      codestream_seek (codestream_tell () - 2);
	      return 0;
	    }
	  /* subl with 32 bit immediate */
	  codestream_read ((unsigned char *)&locals, 4);
	  return (locals);
	}
      else
	{
	  return (0);
	}
    }
  else if (op == 0xc8)
    {
      /* enter instruction: arg is 16 bit unsigned immed */
      unsigned short slocals;
      codestream_read ((unsigned char *)&slocals, 2);
      codestream_get (); /* flush final byte of enter instruction */
      return (slocals);
    }
  return (-1);
}

/* Return number of args passed to a frame.
   Can return -1, meaning no way to tell.  */

/* on the 386, the instruction following the call could be:
 *  popl %ecx        -  one arg
 *  addl $imm, %esp  -  imm/4 args; imm may be 8 or 32 bits
 *  anything else    -  zero args
 */

int
i386_frame_num_args (fi)
     struct frame_info fi;
{
  int retpc;						
  unsigned char op;					
  struct frame_info *pfi;

  int frameless;

  FRAMELESS_FUNCTION_INVOCATION (fi, frameless);
  if (frameless)
    /* In the absence of a frame pointer, GDB doesn't get correct values
       for nameless arguments.  Return -1, so it doesn't print any
       nameless arguments.  */
    return -1;

  pfi = get_prev_frame_info ((fi));			
  if (pfi == 0)
    {
      /* Note:  this can happen if we are looking at the frame for
	 main, because FRAME_CHAIN_VALID won't let us go into
	 start.  If we have debugging symbols, that's not really
	 a big deal; it just means it will only show as many arguments
	 to main as are declared.  */
      return -1;
    }
  else
    {
      retpc = pfi->pc;					
      op = read_memory_integer (retpc, 1);			
      if (op == 0x59)					
	/* pop %ecx */			       
	return 1;				
      else if (op == 0x83)
	{
	  op = read_memory_integer (retpc+1, 1);	
	  if (op == 0xc4)				
	    /* addl $<signed imm 8 bits>, %esp */	
	    return (read_memory_integer (retpc+2,1)&0xff)/4;
	  else
	    return 0;
	}
      else if (op == 0x81)
	{ /* add with 32 bit immediate */
	  op = read_memory_integer (retpc+1, 1);	
	  if (op == 0xc4)				
	    /* addl $<imm 32>, %esp */		
	    return read_memory_integer (retpc+2, 4) / 4;
	  else
	    return 0;
	}
      else
	{
	  return 0;
	}
    }
}

/*
 * parse the first few instructions of the function to see
 * what registers were stored.
 *
 * We handle these cases:
 *
 * The startup sequence can be at the start of the function,
 * or the function can start with a branch to startup code at the end.
 *
 * %ebp can be set up with either the 'enter' instruction, or 
 * 'pushl %ebp, movl %esp, %ebp' (enter is too slow to be useful,
 * but was once used in the sys5 compiler)
 *
 * Local space is allocated just below the saved %ebp by either the
 * 'enter' instruction, or by 'subl $<size>, %esp'.  'enter' has
 * a 16 bit unsigned argument for space to allocate, and the
 * 'addl' instruction could have either a signed byte, or
 * 32 bit immediate.
 *
 * Next, the registers used by this function are pushed.  In
 * the sys5 compiler they will always be in the order: %edi, %esi, %ebx
 * (and sometimes a harmless bug causes it to also save but not restore %eax);
 * however, the code below is willing to see the pushes in any order,
 * and will handle up to 8 of them.
 *
 * If the setup sequence is at the end of the function, then the
 * next instruction will be a branch back to the start.
 */

i386_frame_find_saved_regs (fip, fsrp)
     struct frame_info *fip;
     struct frame_saved_regs *fsrp;
{
  long locals;
  unsigned char *p;
  unsigned char op;
  CORE_ADDR dummy_bottom;
  CORE_ADDR adr;
  int i;
  
  bzero (fsrp, sizeof *fsrp);
  
  /* if frame is the end of a dummy, compute where the
   * beginning would be
   */
  dummy_bottom = fip->frame - 4 - REGISTER_BYTES - CALL_DUMMY_LENGTH;
  
  /* check if the PC is in the stack, in a dummy frame */
  if (dummy_bottom <= fip->pc && fip->pc <= fip->frame) 
    {
      /* all regs were saved by push_call_dummy () */
      adr = fip->frame;
      for (i = 0; i < NUM_REGS; i++) 
	{
	  adr -= REGISTER_RAW_SIZE (i);
	  fsrp->regs[i] = adr;
	}
      return;
    }
  
  locals = i386_get_frame_setup (get_pc_function_start (fip->pc));
  
  if (locals >= 0) 
    {
      adr = fip->frame - 4 - locals;
      for (i = 0; i < 8; i++) 
	{
	  op = codestream_get ();
	  if (op < 0x50 || op > 0x57)
	    break;
	  fsrp->regs[op - 0x50] = adr;
	  adr -= 4;
	}
    }
  
  fsrp->regs[PC_REGNUM] = fip->frame + 4;
  fsrp->regs[FP_REGNUM] = fip->frame;
}

/* return pc of first real instruction */
i386_skip_prologue (pc)
{
  unsigned char op;
  int i;
  
  if (i386_get_frame_setup (pc) < 0)
    return (pc);
  
  /* found valid frame setup - codestream now points to 
   * start of push instructions for saving registers
   */
  
  /* skip over register saves */
  for (i = 0; i < 8; i++)
    {
      op = codestream_peek ();
      /* break if not pushl inst */
      if (op < 0x50 || op > 0x57) 
	break;
      codestream_get ();
    }
  
  i386_follow_jump ();
  
  return (codestream_tell ());
}

i386_push_dummy_frame ()
{
  CORE_ADDR sp = read_register (SP_REGNUM);
  int regnum;
  char regbuf[MAX_REGISTER_RAW_SIZE];
  
  sp = push_word (sp, read_register (PC_REGNUM));
  sp = push_word (sp, read_register (FP_REGNUM));
  write_register (FP_REGNUM, sp);
  for (regnum = 0; regnum < NUM_REGS; regnum++)
    {
      read_register_gen (regnum, regbuf);
      sp = push_bytes (sp, regbuf, REGISTER_RAW_SIZE (regnum));
    }
  write_register (SP_REGNUM, sp);
}

i386_pop_frame ()
{
  FRAME frame = get_current_frame ();
  CORE_ADDR fp;
  int regnum;
  struct frame_saved_regs fsr;
  struct frame_info *fi;
  char regbuf[MAX_REGISTER_RAW_SIZE];
  
  fi = get_frame_info (frame);
  fp = fi->frame;
  get_frame_saved_regs (fi, &fsr);
  for (regnum = 0; regnum < NUM_REGS; regnum++) 
    {
      CORE_ADDR adr;
      adr = fsr.regs[regnum];
      if (adr)
	{
	  read_memory (adr, regbuf, REGISTER_RAW_SIZE (regnum));
	  write_register_bytes (REGISTER_BYTE (regnum), regbuf,
				REGISTER_RAW_SIZE (regnum));
	}
    }
  write_register (FP_REGNUM, read_memory_integer (fp, 4));
  write_register (PC_REGNUM, read_memory_integer (fp + 4, 4));
  write_register (SP_REGNUM, fp + 8);
  flush_cached_frames ();
  set_current_frame ( create_new_frame (read_register (FP_REGNUM),
					read_pc ()));
}
