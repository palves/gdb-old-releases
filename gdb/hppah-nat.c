/* Machine-dependent hooks for the unix child process stratum, for HPUX PA-RISC.

   Copyright 1986, 1987, 1989, 1990, 1991, 1992, 1993
   Free Software Foundation, Inc.

   Contributed by the Center for Software Science at the
   University of Utah (pa-gdb-bugs@cs.utah.edu).

This file is part of GDB.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */


#include "defs.h"
#include "inferior.h"
#include "target.h"
#include <sys/ptrace.h>

extern CORE_ADDR text_end;

static void fetch_register ();

/* This function simply calls ptrace with the given arguments.  
   It exists so that all calls to ptrace are isolated in this 
   machine-dependent file. */
int
call_ptrace (request, pid, addr, data)
     int request, pid;
     PTRACE_ARG3_TYPE addr;
     int data;
{
  return ptrace (request, pid, addr, data, 0);
}

void
kill_inferior ()
{
  if (inferior_pid == 0)
    return;
  ptrace (PT_EXIT, inferior_pid, (PTRACE_ARG3_TYPE) 0, 0, 0);
  wait ((int *)0);
  target_mourn_inferior ();
}

/* Start debugging the process whose number is PID.  */
int
attach (pid)
     int pid;
{
  errno = 0;
  ptrace (PT_ATTACH, pid, (PTRACE_ARG3_TYPE) 0, 0, 0);
  if (errno)
    perror_with_name ("ptrace");
  attach_flag = 1;
  return pid;
}

/* Stop debugging the process whose number is PID
   and continue it with signal number SIGNAL.
   SIGNAL = 0 means just continue it.  */

void
detach (signal)
     int signal;
{
  errno = 0;
  ptrace (PT_DETACH, inferior_pid, (PTRACE_ARG3_TYPE) 1, signal, 0);
  if (errno)
    perror_with_name ("ptrace");
  attach_flag = 0;
}

/* Fetch all registers, or just one, from the child process.  */

void
fetch_inferior_registers (regno)
     int regno;
{
  if (regno == -1)
    for (regno = 0; regno < NUM_REGS; regno++)
      fetch_register (regno);
  else
    fetch_register (regno);
}

/* Store our register values back into the inferior.
   If REGNO is -1, do this for all registers.
   Otherwise, REGNO specifies which register (so we can save time).  */

void
store_inferior_registers (regno)
     int regno;
{
  register unsigned int regaddr;
  char buf[80];
  extern char registers[];
  register int i;
  unsigned int offset = U_REGS_OFFSET;
  int scratch;

  if (regno >= 0)
    {
      regaddr = register_addr (regno, offset);
      errno = 0;
      if (regno == PCOQ_HEAD_REGNUM || regno == PCOQ_TAIL_REGNUM)
        {
          scratch = *(int *) &registers[REGISTER_BYTE (regno)] | 0x3;
          ptrace (PT_WUREGS, inferior_pid, (PTRACE_ARG3_TYPE) regaddr,
                  scratch, 0);
          if (errno != 0)
            {
              sprintf (buf, "writing register number %d(%d)", regno, i);
              perror_with_name (buf);
            }
        }
      else
	for (i = 0; i < REGISTER_RAW_SIZE (regno); i += sizeof(int))
	  {
	    errno = 0;
	    ptrace (PT_WUREGS, inferior_pid, (PTRACE_ARG3_TYPE) regaddr,
		    *(int *) &registers[REGISTER_BYTE (regno) + i], 0);
	    if (errno != 0)
	      {
		sprintf (buf, "writing register number %d(%d)", regno, i);
		perror_with_name (buf);
	      }
	    regaddr += sizeof(int);
	  }
    }
  else
    {
      for (regno = 0; regno < NUM_REGS; regno++)
	{
	  if (CANNOT_STORE_REGISTER (regno))
	    continue;
	  regaddr = register_addr (regno, offset);
          errno = 0;
          if (regno == PCOQ_HEAD_REGNUM || regno == PCOQ_TAIL_REGNUM)
            {
              scratch = *(int *) &registers[REGISTER_BYTE (regno)] | 0x3;
              ptrace (PT_WUREGS, inferior_pid, (PTRACE_ARG3_TYPE) regaddr,
                      scratch, 0);
              if (errno != 0)
                {
                  sprintf (buf, "writing register number %d(%d)", regno, i);
                  perror_with_name (buf);
                }
            }
          else
	    for (i = 0; i < REGISTER_RAW_SIZE (regno); i += sizeof(int))
	      {
		errno = 0;
		ptrace (PT_WUREGS, inferior_pid, (PTRACE_ARG3_TYPE) regaddr,
			*(int *) &registers[REGISTER_BYTE (regno) + i], 0);
		if (errno != 0)
		  {
		    sprintf (buf, "writing register number %d(%d)", regno, i);
		    perror_with_name (buf);
		  }
		regaddr += sizeof(int);
	      }
	}
    }
  return;
}

/* Fetch one register.  */

static void
fetch_register (regno)
     int regno;
{
  register unsigned int regaddr;
  char buf[MAX_REGISTER_RAW_SIZE];
  char mess[128];				/* For messages */
  register int i;

  /* Offset of registers within the u area.  */
  unsigned int offset;

  offset = U_REGS_OFFSET;

  regaddr = register_addr (regno, offset);
  for (i = 0; i < REGISTER_RAW_SIZE (regno); i += sizeof (int))
    {
      errno = 0;
      *(int *) &buf[i] = ptrace (PT_RUREGS, inferior_pid,
				 (PTRACE_ARG3_TYPE) regaddr, 0, 0);
      regaddr += sizeof (int);
      if (errno != 0)
	{
	  sprintf (mess, "reading register %s (#%d)", reg_names[regno], regno);
	  perror_with_name (mess);
	}
    }
  if (regno == PCOQ_HEAD_REGNUM || regno == PCOQ_TAIL_REGNUM)
    buf[3] &= ~0x3;
  supply_register (regno, buf);
}

/* Resume execution of the inferior process.
   If STEP is nonzero, single-step it.
   If SIGNAL is nonzero, give it that signal.  */

void
child_resume (step, signal)
     int step;
     int signal;
{
  errno = 0;

  /* An address of (PTRACE_ARG3_TYPE) 1 tells ptrace to continue from where
     it was. (If GDB wanted it to start some other way, we have already
     written a new PC value to the child.)  */

  if (step)
    ptrace (PT_SINGLE, inferior_pid, (PTRACE_ARG3_TYPE) 1, signal, 0);
  else
    ptrace (PT_CONTIN, inferior_pid, (PTRACE_ARG3_TYPE) 1, signal, 0);

  if (errno)
    perror_with_name ("ptrace");
}

/* Copy LEN bytes to or from inferior's memory starting at MEMADDR
   to debugger memory starting at MYADDR.   Copy to inferior if
   WRITE is nonzero.
  
   Returns the length copied, which is either the LEN argument or zero.
   This xfer function does not do partial moves, since child_ops
   doesn't allow memory operations to cross below us in the target stack
   anyway.  */

int
child_xfer_memory (memaddr, myaddr, len, write, target)
     CORE_ADDR memaddr;
     char *myaddr;
     int len;
     int write;
     struct target_ops *target;		/* ignored */
{
  register int i;
  /* Round starting address down to longword boundary.  */
  register CORE_ADDR addr = memaddr & - sizeof (int);
  /* Round ending address up; get number of longwords that makes.  */
  register int count
    = (((memaddr + len) - addr) + sizeof (int) - 1) / sizeof (int);
  /* Allocate buffer of that many longwords.  */
  register int *buffer = (int *) alloca (count * sizeof (int));

  if (write)
    {
      /* Fill start and end extra bytes of buffer with existing memory data.  */

      if (addr != memaddr || len < (int)sizeof (int)) {
	/* Need part of initial word -- fetch it.  */
        buffer[0] = ptrace (addr < text_end ? PT_RIUSER : PT_RDUSER, 
			    inferior_pid, (PTRACE_ARG3_TYPE) addr, 0, 0);
      }

      if (count > 1)		/* FIXME, avoid if even boundary */
	{
	  buffer[count - 1]
	    = ptrace (addr < text_end ? PT_RIUSER : PT_RDUSER, inferior_pid,
		      (PTRACE_ARG3_TYPE) (addr + (count - 1) * sizeof (int)),
		      0, 0);
	}

      /* Copy data to be written over corresponding part of buffer */

      bcopy (myaddr, (char *) buffer + (memaddr & (sizeof (int) - 1)), len);

      /* Write the entire buffer.  */

      for (i = 0; i < count; i++, addr += sizeof (int))
	{
/* The HP-UX kernel crashes if you use PT_WDUSER to write into the text
   segment.  FIXME -- does it work to write into the data segment using
   WIUSER, or do these idiots really expect us to figure out which segment
   the address is in, so we can use a separate system call for it??!  */
	  errno = 0;
	  ptrace (addr < text_end ? PT_WIUSER : PT_WDUSER, inferior_pid, 
		  (PTRACE_ARG3_TYPE) addr,
		  buffer[i], 0);
	  if (errno)
	    return 0;
	}
    }
  else
    {
      /* Read all the longwords */
      for (i = 0; i < count; i++, addr += sizeof (int))
	{
	  errno = 0;
	  buffer[i] = ptrace (addr < text_end ? PT_RIUSER : PT_RDUSER, 
			      inferior_pid, (PTRACE_ARG3_TYPE) addr, 0, 0);
	  if (errno)
	    return 0;
	  QUIT;
	}

      /* Copy appropriate bytes out of the buffer.  */
      bcopy ((char *) buffer + (memaddr & (sizeof (int) - 1)), myaddr, len);
    }
  return len;
}
