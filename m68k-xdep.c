/* Print m68k instructions for GDB, the GNU debugger.
   Copyright (C) 1986, 1987, 1989, 1990 Free Software Foundation, Inc.

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

#if !defined (RDB)

/* If the target doesn't define this, it won't need these functions.  */
#if !defined (HAVE_68881)

#include <setjmp.h>
#include <signal.h>

#include "defs.h"
#include "param.h"

/* Nonzero if the host system has a 68881 (or compatible)
   floating-point unit.  This does *not* indicate whether the target
   system has a co-processor, just whether the host system does.
   There might be a difference in the case of remote debugging.  */
static int have_fpu = 1;

/* This is not part of insn printing, but it is machine-specific,
   so this is a convenient place to put it.

   Convert a 68881 extended float to a double.
   FROM is the address of the extended float.
   Store the double in *TO.  */

convert_from_68881 (from, to)
     char *from;
     double *to;
{
  if (!have_fpu)
    {
      *to = 0.0;
      return;
    }
  else
    {
#ifdef HPUX_ASM
      asm ("mov.l 8(%a6),%a0");
      asm ("mov.l 12(%a6),%a1");
      asm ("fmove.x (%a0),%fp0");
      asm ("fmove.d %fp0,(%a1)");
#else /* not HPUX_ASM */
#if 0
      asm ("movl a6@(8),a0");
      asm ("movl a6@(12),a1");
      asm ("fmovex a0@,fp0");
      asm ("fmoved fp0,a1@");
#else
      /* Hand-assemble those insns since some assemblers lose
	 and some have different syntax.  */
      asm (".word 020156");
      asm (".word 8");
      asm (".word 021156");
      asm (".word 12");
      asm (".long 0xf2104800");
      asm (".long 0xf2117400");
#endif
#endif /* not HPUX_ASM */
    }
}

/* The converse: convert the double *FROM to an extended float
   and store where TO points.  */

convert_to_68881 (from, to)
     double *from;
     char *to;
{
  if (!have_fpu)
      return;
  else
    {
#ifdef HPUX_ASM
      asm ("mov.l 8(%a6),%a0");
      asm ("mov.l 12(%a6),%a1");
      asm ("fmove.d (%a0),%fp0");
      asm ("fmove.x %fp0,(%a1)");
#else /* not HPUX_ASM */
#if 0
      asm ("movl a6@(8),a0");
      asm ("movl a6@(12),a1");
      asm ("fmoved a0@,fp0");
      asm ("fmovex fp0,a1@");
#else
      /* Hand-assemble those insns since some assemblers lose.  */
      asm (".word 020156");
      asm (".word 8");
      asm (".word 021156");
      asm (".word 12");
      asm (".long 0xf2105400");
      asm (".long 0xf2116800");
#endif
#endif /* not HPUX_ASM */
    }
}

static jmp_buf	fpu_check;

void
sigemt()
{
    have_fpu = 0;
    longjmp (fpu_check, 1);
}

void
_initialize_pinsn()
{
  /* Want to figure out if we've got a coprocessor. The idea is to catch the
     signal that gets delivered if no coprocessor is around (SIGEMT) then
     execute a coprocessor instruction and see what happens. have_fpu is set
     to zero if the EMT signal arrives. Else it is left at 1.  */
  /* If this turns out not to be portable to all 68k machines, we'll
     have to move it to the dep files.  */
  void (*emthandler) ();

  emthandler = (void (*) ()) signal (SIGEMT, sigemt);
  if (!setjmp (fpu_check))
    {
#if defined(HPUX_ASM)
      asm (" long       0xf2000600");   /* fmovel fp0, d0 */
#else
      asm(".long	0xf2000600");	/* fmovel fp0, d0 */
#endif
    }
  signal(SIGEMT, emthandler);
}
#endif /* Target has 68881.  */

#endif /* Not remote stub.  */
