/* Definitions to make GDB run on Convex Unix (4bsd)
   Copyright (C) 1989 Free Software Foundation, Inc.

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

#define BYTE_ORDER BIG_ENDIAN

#define LONG_LONG
#define ATTACH_DETACH
#define HAVE_WAIT_STRUCT
#define NO_SIGINTERRUPT

/* Get rid of any system-imposed stack limit if possible.  */

#define SET_STACK_LIMIT_HUGE

/* Use SIGCONT rather than SIGTSTP because convex Unix occasionally
   turkeys SIGTSTP.  I think.  */

#define STOP_SIGNAL SIGCONT

/* Use csh to do argument expansion so we get ~ and such.  */

/* Doesn't work.  */
/* #define SHELL_FILE "/bin/csh" */

/* Compensate for lack of `vprintf' function.  */
#define MISSING_VPRINTF

/* Hook to call after creating inferior process.  */

#define CREATE_INFERIOR_HOOK create_inferior_hook


/* Interface definitions for kernel debugger KDB.  */

/* (no kdb) */
