/* Low level interface to ptrace, for GDB when running under Unix.
   Copyright (C) 1986, 1987, 1989 Free Software Foundation, Inc.
   
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

/* Defining this means some system include files define some extra stuff.  */
#define WOPR
#include <sys/param.h>
#include <sys/dir.h>
#include <signal.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/trap.h>

#include "gdbcore.h"

#include <sys/file.h>
#include <sys/stat.h>

extern int errno;

#define INFERIOR_AR0(u)							\
  ((ptrace								\
    (PT_RUAREA, inferior_pid, ((char *) &u.u_ar0 - (char *) &u), 0))	\
   - KERNEL_U_ADDR)

static void
  fetch_inferior_register (regno, regaddr)
register int regno;
register unsigned int regaddr;
{
#ifndef HPUX_VERSION_5
  if (regno == PS_REGNUM)
    {
      union { int i; short s[2]; } ps_val;
      int regval;
      
      ps_val.i = (ptrace (PT_RUAREA, inferior_pid, regaddr, 0));
      regval = ps_val.s[0];
      supply_register (regno, &regval);
    }
  else
#endif /* not HPUX_VERSION_5 */
    {
      char buf[MAX_REGISTER_RAW_SIZE];
      register int i;
      
      for (i = 0; i < REGISTER_RAW_SIZE (regno); i += sizeof (int))
	{
	  *(int *) &buf[i] = ptrace (PT_RUAREA, inferior_pid, regaddr, 0);
	  regaddr += sizeof (int);
	}
      supply_register (regno, buf);
    }
  return;
}

static void
  store_inferior_register_1 (regno, regaddr, value)
int regno;
unsigned int regaddr;
int value;
{
  errno = 0;
  ptrace (PT_WUAREA, inferior_pid, regaddr, value);
#if 0
  /* HP-UX randomly sets errno to non-zero for regno == 25.
     However, the value is correctly written, so ignore errno. */
  if (errno != 0)
    {
      char string_buf[64];
      
      sprintf (string_buf, "writing register number %d", regno);
      perror_with_name (string_buf);
    }
#endif
  return;
}

static void
  store_inferior_register (regno, regaddr)
register int regno;
register unsigned int regaddr;
{
#ifndef HPUX_VERSION_5
  if (regno == PS_REGNUM)
    {
      union { int i; short s[2]; } ps_val;
      
      ps_val.i = (ptrace (PT_RUAREA, inferior_pid, regaddr, 0));
      ps_val.s[0] = (read_register (regno));
      store_inferior_register_1 (regno, regaddr, ps_val.i);
    }
  else
#endif /* not HPUX_VERSION_5 */
    {
      char buf[MAX_REGISTER_RAW_SIZE];
      register int i;
      extern char registers[];
      
      for (i = 0; i < REGISTER_RAW_SIZE (regno); i += sizeof (int))
	{
	  store_inferior_register_1
	    (regno, regaddr,
	     (*(int *) &registers[(REGISTER_BYTE (regno)) + i]));
	  regaddr += sizeof (int);
	}
    }
  return;
}

void
fetch_inferior_registers (regno)
     int regno;
{
  struct user u;
  register int regno;
  register unsigned int ar0_offset;
  
  ar0_offset = (INFERIOR_AR0 (u));
  if (regno == -1)
    {
      for (regno = 0; (regno < FP0_REGNUM); regno++)
	fetch_inferior_register (regno, (REGISTER_ADDR (ar0_offset, regno)));
      for (; (regno < NUM_REGS); regno++)
	fetch_inferior_register (regno, (FP_REGISTER_ADDR (u, regno)));
    }
  else
    fetch_inferior_register (regno,
			     (regno < FP0_REGNUM
			      ? REGISTER_ADDR (ar0_offset, regno)
			      : FP_REGISTER_ADDR (u, regno)));
}

/* Store our register values back into the inferior.
   If REGNO is -1, do this for all registers.
   Otherwise, REGNO specifies which register (so we can save time).  */

store_inferior_registers (regno)
     register int regno;
{
  struct user u;
  register unsigned int ar0_offset;
  extern char registers[];

  if (regno >= FP0_REGNUM)
    {
      store_inferior_register (regno, (FP_REGISTER_ADDR (u, regno)));
      return;
    }
  
  ar0_offset = (INFERIOR_AR0 (u));
  if (regno >= 0)
    {
      store_inferior_register (regno, (REGISTER_ADDR (ar0_offset, regno)));
      return;
    }

  for (regno = 0; (regno < FP0_REGNUM); regno++)
    store_inferior_register (regno, (REGISTER_ADDR (ar0_offset, regno)));
  for (; (regno < NUM_REGS); regno++)
    store_inferior_register (regno, (FP_REGISTER_ADDR (u, regno)));
  return;
}


/* Work with core dump and executable files, for GDB. 
   This code would be in core.c if it weren't machine-dependent. */

#ifdef HPUX_VERSION_5
#define e_PS e_regs[PS]
#define e_PC e_regs[PC]
#endif /* HPUX_VERSION_5 */

void
core_file_command (filename, from_tty)
     char *filename;
     int from_tty;
{
  int val;
  extern char registers[];

  /* Discard all vestiges of any previous core file
     and mark data and stack spaces as empty.  */

  if (corefile)
    free (corefile);
  corefile = 0;

  if (corechan >= 0)
    close (corechan);
  corechan = -1;

  data_start = 0;
  data_end = 0;
  stack_start = STACK_END_ADDR;
  stack_end = STACK_END_ADDR;

  /* Now, if a new core file was specified, open it and digest it.  */

  if (filename)
    {
      filename = tilde_expand (filename);
      make_cleanup (free, filename);
      
      if (have_inferior_p ())
	error ("To look at a core file, you must kill the inferior with \"kill\".");
      corechan = open (filename, O_RDONLY, 0);
      if (corechan < 0)
	perror_with_name (filename);
      /* 4.2-style (and perhaps also sysV-style) core dump file.  */
      {
	struct user u;

	int reg_offset;

	val = myread (corechan, &u, sizeof u);
	if (val < 0)
	  perror_with_name (filename);
	data_start = exec_data_start;

	data_end = data_start + NBPG * u.u_dsize;
	stack_start = stack_end - NBPG * u.u_ssize;
	data_offset = NBPG * UPAGES;
	stack_offset = NBPG * (UPAGES + u.u_dsize);
	reg_offset = (int) u.u_ar0 - KERNEL_U_ADDR;

	/* I don't know where to find this info.
	   So, for now, mark it as not available.  */
	core_aouthdr.a_magic = 0;

	/* Read the register values out of the core file and store
	   them where `read_register' will find them.  */

	{
	  register int regno;
	  struct exception_stack es;
	  int val;

	  val = lseek (corechan, (REGISTER_ADDR (reg_offset, 0)), 0);
	  if (val < 0)
	    perror_with_name (filename);
	  val = myread (corechan, es,
			((char *) &es.e_offset - (char *) &es.e_regs[R0]));
	  if (val < 0)
	    perror_with_name (filename);
	  for (regno = 0; (regno < PS_REGNUM); regno++)
	    supply_register (regno, &es.e_regs[regno + R0]);
	  val = es.e_PS;
	  supply_register (regno++, &val);
	  supply_register (regno++, &es.e_PC);
	  for (; (regno < NUM_REGS); regno++)
	    {
	      char buf[MAX_REGISTER_RAW_SIZE];

	      val = lseek (corechan, (FP_REGISTER_ADDR (u, regno)), 0);
	      if (val < 0)
		perror_with_name (filename);

 	      val = myread (corechan, buf, sizeof buf);
	      if (val < 0)
		perror_with_name (filename);
	      supply_register (regno, buf);
	    }
	}
      }
      if (filename[0] == '/')
	corefile = savestring (filename, strlen (filename));
      else
	{
	  corefile = concat (current_directory, "/", filename);
	}

      set_current_frame ( create_new_frame (read_register (FP_REGNUM),
					    read_pc ()));
      select_frame (get_current_frame (), 0);
      validate_files ();
    }
  else if (from_tty)
    printf ("No core file now.\n");
}
