/* Target dependent code for the Motorola 68000 series.
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

/* FIXME, what an ugly hack.  If the target is a 68000 and the host also is,
   we will get these symbols defined as functions in m68k-xdep.c.
   If the target is a 68000 and the host isn't, the declarations below
   will cause them to be defined as "common" symbols.  If anyone
   jumps to them, they'll abort, so cross-debuggers have to take
   other precautions to handle 68881 conversions.  */

int convert_to_68881;
int convert_from_68881;
