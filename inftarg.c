/* Subroutines for handling an "inferior" (child) process as a target
   for debugging, in GDB.
   Copyright (C) 1990 Free Software Foundation, Inc.

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
#include "frame.h"  /* required by inferior.h */
#include "inferior.h"
#include "target.h"
#include "wait.h"
#include "gdbcore.h"

extern int fetch_inferior_registers();
extern int store_inferior_registers();
extern int child_xfer_memory();
extern int memory_insert_breakpoint(), memory_remove_breakpoint();
extern void terminal_init_inferior(), terminal_ours(), terminal_inferior();
extern void terminal_ours_for_output(), child_terminal_info();
extern void kill_inferior(), add_file_addr_command();
extern struct value *call_function_by_hand();
extern void child_resume();
extern void child_create_inferior();
extern void child_mourn_inferior();
extern void child_open ();

/* Wait for child to do something.  Return pid of child, or -1 in case
   of error; store status through argument pointer STATUS.  */

int
child_wait (status)
     int *status;
{
  int pid;

  do {
    pid = wait (status);
    if (pid == -1)		/* No more children to wait for */
      {
	fprintf (stderr, "Child process unexpectedly missing.\n");
	*status = 42;	/* Claim it exited with signal 42 */
        return -1;
      }
  } while (pid != inferior_pid); /* Some other child died or stopped */
  return pid;
}


/*
 * child_detach()
 * takes a program previously attached to and detaches it.
 * The program resumes execution and will no longer stop
 * on signals, etc.  We better not have left any breakpoints
 * in the program or it'll die when it hits one.  For this
 * to work, it may be necessary for the process to have been
 * previously attached.  It *might* work if the program was
 * started via the normal ptrace (PTRACE_TRACEME).
 */

static void
child_detach (args, from_tty)
     char *args;
     int from_tty;
{
  int siggnal = 0;

#ifdef ATTACH_DETACH
  if (inferior_pid)
    {
      if (from_tty)
	{
	  char *exec_file = get_exec_file (0);
	  if (exec_file == 0)
	    exec_file = "";
	  printf ("Detaching program: %s pid %d\n",
		  exec_file, inferior_pid);
	  fflush (stdout);
	}
      if (args)
	siggnal = atoi (args);
      
      detach (siggnal);
      inferior_pid = 0;
    }
  else
#endif
    error ("Not currently attached to subsidiary or remote process.");
}

/* Get ready to modify the registers array.  On machines which store
   individual registers, this doesn't need to do anything.  On machines
   which store all the registers in one fell swoop, this makes sure
   that registers contains all the registers from the program being
   debugged.  */

void
child_prepare_to_store ()
{
#ifdef CHILD_PREPARE_TO_STORE
  CHILD_PREPARE_TO_STORE ();
#endif
}

/* Convert data from raw format for register REGNUM
   to virtual format for register REGNUM.  */

void
host_convert_to_virtual (regnum, from, to)
     int regnum;
     char *from;
     char *to;
{
  REGISTER_CONVERT_TO_VIRTUAL (regnum, from, to);
}

/* Convert data from virtual format for register REGNUM
   to raw format for register REGNUM.  */

void
host_convert_from_virtual (regnum, from, to)
     int regnum;
     char *from;
     char *to;
{
  REGISTER_CONVERT_TO_RAW (regnum, from, to);
}

/* Print status information about what we're accessing.  */

static void
child_files_info ()
{
  printf ("\tUsing the running image of pid %d, %s.\n",
	  inferior_pid,
	  attach_flag? "which we attached": "a child process of the debugger");
}

struct target_ops child_ops = {
	"child", "Unix child process",
	child_open, child_detach, 
	child_resume,
	child_wait,
	fetch_inferior_registers, store_inferior_registers,
	child_prepare_to_store,
	host_convert_to_virtual, host_convert_from_virtual,
	child_xfer_memory, child_files_info,
	memory_insert_breakpoint, memory_remove_breakpoint,
	terminal_init_inferior, terminal_inferior, 
	terminal_ours_for_output, terminal_ours, child_terminal_info,
	kill_inferior, add_file_addr_command,
	call_function_by_hand,
	0, /* lookup_symbol */
	child_create_inferior, child_mourn_inferior,
	0, /* next */
	1, 1, 1, 1, 1,	/* all mem, mem, stack, regs, exec */
	OPS_MAGIC,		/* Always the last thing */
};

void
_initialize_inftarg ()
{
  add_target (&child_ops);
}
