/* Parameters for execution on VxWorks 68k's, for GDB, the GNU debugger.
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

#define	GDBINIT_FILENAME	".vxgdbinit"

#define	DEFAULT_PROMPT		"(vxgdb) "

/* Kludge... */
#include "tm-sun3.h"

/* We have more complex, useful breakpoints on the target.  */
#undef DECR_PC_AFTER_BREAK
#define	DECR_PC_AFTER_BREAK	0
