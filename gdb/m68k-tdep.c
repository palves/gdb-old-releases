/* Target dependent code for the Motorola 68000 series.
   Copyright (C) 1990 Free Software Foundation, Inc.

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
#include "ieee-float.h"
#include "param.h"
#include "frame.h"

const struct ext_format ext_format_68881 = {
/* tot sbyte smask expbyte manbyte */
   12, 0,    0x80, 0,1,	   4,8		/* mc68881 */
};


/* Things needed for making the inferior call functions.
   It seems like every m68k based machine has almost identical definitions
   in the individual machine's configuration files.  Most other cpu types
   (mips, i386, etc) have routines in their *-tdep.c files to handle this
   for most configurations.  The m68k family should be able to do this as
   well.  These macros can still be overridden when necessary.  */

/* Push an empty stack frame, to record the current PC, etc.  */

void
m68k_push_dummy_frame ()
{
  register CORE_ADDR sp = read_register (SP_REGNUM);
  register int regnum;
  char raw_buffer[12];

  sp = push_word (sp, read_register (PC_REGNUM));
  sp = push_word (sp, read_register (FP_REGNUM));
  write_register (FP_REGNUM, sp);
#if defined (HAVE_68881)
  for (regnum = FP0_REGNUM + 7; regnum >= FP0_REGNUM; regnum--)
    {
      read_register_bytes (REGISTER_BYTE (regnum), raw_buffer, 12);
      sp = push_bytes (sp, raw_buffer, 12);
    }
#endif
  for (regnum = FP_REGNUM - 1; regnum >= 0; regnum--)
    {
      sp = push_word (sp, read_register (regnum));
    }
  sp = push_word (sp, read_register (PS_REGNUM));
  write_register (SP_REGNUM, sp);
}

/* Discard from the stack the innermost frame,
   restoring all saved registers.  */

void
m68k_pop_frame ()
{
  register FRAME frame = get_current_frame ();
  register CORE_ADDR fp;
  register int regnum;
  struct frame_saved_regs fsr;
  struct frame_info *fi;
  char raw_buffer[12];

  fi = get_frame_info (frame);
  fp = fi -> frame;
  get_frame_saved_regs (fi, &fsr);
#if defined (HAVE_68881)
  for (regnum = FP0_REGNUM + 7 ; regnum >= FP0_REGNUM ; regnum--)
    {
      if (fsr.regs[regnum])
	{
	  read_memory (fsr.regs[regnum], raw_buffer, 12);
	  write_register_bytes (REGISTER_BYTE (regnum), raw_buffer, 12);
	}
    }
#endif
  for (regnum = FP_REGNUM - 1 ; regnum >= 0 ; regnum--)
    {
      if (fsr.regs[regnum])
	{
	  write_register (regnum, read_memory_integer (fsr.regs[regnum], 4));
	}
    }
  if (fsr.regs[PS_REGNUM])
    {
      write_register (PS_REGNUM, read_memory_integer (fsr.regs[PS_REGNUM], 4));
    }
  write_register (FP_REGNUM, read_memory_integer (fp, 4));
  write_register (PC_REGNUM, read_memory_integer (fp + 4, 4));
  write_register (SP_REGNUM, fp + 8);
  flush_cached_frames ();
  set_current_frame (create_new_frame (read_register (FP_REGNUM),
				       read_pc ()));
}
