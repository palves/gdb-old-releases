/* HPPA PA-RISC machine native support for BSD, for GDB.
   Copyright 1991, 1992 Free Software Foundation, Inc. 

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

#define U_REGS_OFFSET 0

/* What a coincidence! */
#define REGISTER_U_ADDR(addr, blockend, regno)				\
{ addr = (int)(blockend) + REGISTER_BYTE (regno);}

/* 3rd argument to ptrace is supposed to be a caddr_t.  */

#define	PTRACE_ARG3_TYPE caddr_t

/* attach/detach works to some extent under BSD and HPUX.  So long
   as the process you're attaching to isn't blocked waiting on io,
   blocked waiting on a signal, or in a system call things work 
   fine.  (The problems in those cases are related to the fact that
   the kernel can't provide complete register information for the
   target process...  Which really pisses off GDB.)  */

#define ATTACH_DETACH