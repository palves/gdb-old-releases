/* Simulate breakpoints by patching locations in the target system.
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

#include "defs.h"
#include "param.h"
#include "breakpoint.h"
#include "inferior.h"
#include "target.h"

/* This is the sequence of bytes we insert for a breakpoint.  */

static char break_insn[] = BREAKPOINT;

/* Insert a breakpoint on machines that don't have any better breakpoint
   support.  We read the contents of the target location and stash it,
   then overwrite it with a breakpoint instruction.  ADDR is the target
   location in the target machine.  CONTENTS_CACHE is a pointer to 
   memory allocated for saving the target contents.  It is guaranteed
   by the caller to be long enough to save sizeof BREAKPOINT bytes.
   FIXME: This size is t_arch dependent and should be available in
   the t_arch transfer vector.  */
int
memory_insert_breakpoint (addr, contents_cache)
     CORE_ADDR addr;
     char *contents_cache;
{
  int val;

  val = target_read_memory (addr, contents_cache, sizeof break_insn);

  if (val == 0)
    val = target_write_memory (addr, break_insn, sizeof break_insn);

  return val;
}


int
memory_remove_breakpoint (addr, contents_cache)
     CORE_ADDR addr;
     char *contents_cache;
{
  return target_write_memory (addr, contents_cache, sizeof break_insn);
}
