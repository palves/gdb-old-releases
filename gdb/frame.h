/* Definitions for dealing with stack frames, for GDB, the GNU debugger.
   Copyright (C) 1986, 1989 Free Software Foundation, Inc.

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

#if !defined (FRAME_H)
#define FRAME_H 1
#include "param.h"

/*
 * FRAME is the type of the identifier of a specific stack frame.  It
 * is a pointer to the frame cache item corresponding to this frame.
 * Please note that frame id's are *not* constant over calls to the
 * inferior.  Use frame addresses, which are.
 *
 * FRAME_ADDR is the type of the address of a specific frame.  I
 * cannot imagine a case in which this would not be CORE_ADDR, so
 * maybe it's silly to give it it's own type.  Life's rough.
 *
 * FRAME_FP is a macro which converts from a frame identifier into a
 * frame_address.
 *
 * FRAME_INFO_ID is a macro which "converts" from a frame info pointer
 * to a frame id.  This is here in case I or someone else decides to
 * change the FRAME type again.
 *
 * This file and blockframe.c are the only places which are allowed to
 * use the equivalence between FRAME and struct frame_info *.  EXCEPTION:
 * value.h uses CORE_ADDR instead of FRAME_ADDR because the compiler
 * will accept that in the absence of this file.
 */
typedef struct frame_info *FRAME;
typedef CORE_ADDR	FRAME_ADDR;
#define FRAME_FP(fr)	((fr)->frame)
#define FRAME_INFO_ID(f)	(f)

/*
 * Caching structure for stack frames.  This is also the structure
 * used for extended info about stack frames.  May add more to this
 * structure as it becomes necessary.
 *
 * Note that the first entry in the cache will always refer to the
 * innermost executing frame.  This value should be set (is it?
 * Check) in something like normal_stop.
 */
struct frame_info
  {
    /* Nominal address of the frame described.  */
    FRAME_ADDR frame;
    /* Address at which execution is occurring in this frame.
       For the innermost frame, it's the current pc.
       For other frames, it is a pc saved in the next frame.  */
    CORE_ADDR pc;
    /* The frame called by the frame we are describing, or 0.
       This may be set even if there isn't a frame called by the one
       we are describing (.->next == 0); in that case it is simply the
       bottom of this frame */
    FRAME_ADDR next_frame;
    /* Anything extra for this structure that may have been defined
       in the machine depedent files. */
#ifdef EXTRA_FRAME_INFO
    EXTRA_FRAME_INFO
#endif
    /* Pointers to the next and previous frame_info's in this stack.  */
    FRAME next, prev;
  };

/* Describe the saved registers of a frame.  */

struct frame_saved_regs
  {
    /* For each register, address of where it was saved on entry to the frame,
       or zero if it was not saved on entry to this frame.  */
    CORE_ADDR regs[NUM_REGS];
  };

/* The stack frame that the user has specified for commands to act on.
   Note that one cannot assume this is the address of valid data.  */

extern FRAME selected_frame;

/* Level of the selected frame:
   0 for innermost, 1 for its caller, ...
   or -1 for frame specified by address with no defined level.  */

int selected_frame_level;

extern struct frame_info *get_frame_info ();
extern struct frame_info *get_prev_frame_info ();

extern FRAME create_new_frame ();
extern void  flush_cached_frames ();
extern void reinit_frame_cache ();

extern void get_frame_saved_regs ();

extern void  set_current_frame ();
extern FRAME get_prev_frame ();
extern FRAME get_current_frame ();
extern FRAME get_next_frame ();

extern struct block *get_frame_block ();
extern struct block *get_current_block ();
extern struct block *get_selected_block ();
extern struct symbol *get_frame_function ();
extern CORE_ADDR get_frame_pc ();
extern CORE_ADDR get_pc_function_start ();
struct block *block_for_pc ();

int frameless_look_for_prologue ();

void print_frame_args ();

/* In stack.c */
extern FRAME find_relative_frame ();
extern void print_stack_frame ();
extern void select_frame ();
extern void record_selected_frame ();

#endif /* frame.h not already included.  */
