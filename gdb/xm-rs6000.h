/* Parameters for hosting on an RS6000, for GDB, the GNU debugger.
   Copyright (C) 1986, 1987, 1989, 1991 Free Software Foundation, Inc.
   Contributed by IBM Corporation.

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

/* Big end is at the low address */

#define	HOST_BYTE_ORDER	BIG_ENDIAN

#define	HAVE_TERMIO 1
#define	USG 1
#define	HAVE_SIGSETMASK	1

/* This system requires that we open a terminal with O_NOCTTY for it to
   not become our controlling terminal.  */

#define	USE_O_NOCTTY

/* Get rid of any system-imposed stack limit if possible.  */

#define SET_STACK_LIMIT_HUGE

/* Brain death inherited from PC's pervades.  */
#undef NULL
#define NULL 0

/* The IBM compiler requires this in order to properly compile alloca().  */
#pragma alloca

#define	vfork	fork

/* Do implement the attach and detach commands.  */

#define ATTACH_DETACH

/* Override copies of {fetch,store}_inferior_registers in infptrace.c.  */

#define FETCH_INFERIOR_REGISTERS

/* Setpgrp() takes arguments, unlike ordinary Sys V's.  */

#define	SETPGRP_ARGS

/* RS6000/AIXCOFF does not support PT_STEP. Has to be simulated. */

#define NO_SINGLE_STEP

/* Interface between xcoff symbol reading code and AIX shared library
   handling code.  FIXME, this probably needs generalizing.  */

#define XCOFF_INIT_LOADINFO()	xcoff_init_loadinfo()
#define XCOFF_ADD_TOC_TO_LOADINFO(x)	xcoff_add_toc_to_loadinfo (x)
