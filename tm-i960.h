/* Parameters for target machine Intel 960, for GDB, the GNU debugger.
   Copyright (C) 1990 Free Software Foundation, Inc.
   Contributed by Intel Corporation.
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

/*****************************************************************************
 * Definitions to target GDB to an i960 debugged over a serial line.
 *
 ******************************************************************************/

#ifndef I80960
#define I80960
#endif

/* Override the standard gdb prompt when compiled for this target.  */

#define	DEFAULT_PROMPT	"(gdb960) "

/* Additional command line options accepted by i960 gdb's, for handling
   the remote-nindy.c interface.  These should really be target-specific
   rather than architecture-specific.  */

extern int nindy_old_protocol;	/* nonzero if old NINDY serial protocol */
extern int nindy_initial_brk;	/* Send a BREAK to reset board first */
extern char *nindy_ttyname;	/* Name of serial port to talk to nindy */

#define	ADDITIONAL_OPTIONS \
	{"O", 0, &nindy_old_protocol, 1},	\
	{"brk", 0, &nindy_initial_brk, 1},	\
	{"r", 1, 0, 1004},  /* 1004 is magic cookie for ADDL_CASES */

#define	ADDITIONAL_OPTION_CASES	\
	case 1004:	/* -r option:  remote nindy auto-start */	\
	  nindy_ttyname = optarg;	\
	  break;

#define	ADDITIONAL_OPTION_HELP \
	"\
  -O                Use old protocol to talk to a Nindy target\n\
  -brk              Send a break to a Nindy target to reset it.\n\
  -r SERIAL         Open remote Nindy session to SERIAL port.\n\
"

/* If specified on the command line, open tty for talking to nindy,
   and download the executable file if one was specified.  */

#define	ADDITIONAL_OPTION_HANDLER	\
	if (!setjmp (to_top_level) && nindy_ttyname) {		\
	  nindy_open (nindy_ttyname, !batch);			\
	  if ( !setjmp(to_top_level) && execarg ) {		\
		target_add_file (execarg, !batch);		\
	  }							\
	}

/* If configured for i960 target, we take control before main loop
   and demand that we configure for a nindy target.  */

#define	BEFORE_MAIN_LOOP_HOOK	\
  nindy_before_main_loop();

/* Byte order is configurable, but this machine runs little-endian.  */
#define	TARGET_BYTE_ORDER	LITTLE_ENDIAN

#define IEEE_FLOAT

extern int dunno;	/* See printcmd.c */

/* Define this if the C compiler puts an underscore at the front
 * of external names before giving them to the linker.
 */
#define NAMES_HAVE_UNDERSCORE


/* Offset from address of function to start of its code.
 * Zero on most machines.
 */
#define FUNCTION_START_OFFSET 0

/* Advance ip across any function entry prologue instructions
 * to reach some "real" code.  (Not needed for i960.)
 */
#define SKIP_PROLOGUE(ip)   ;

/* Immediately after a function call, return the saved ip.
 * Can't always go through the frames for this because on some machines
 * (but not on i960!) the new frame is not set up until the new function
 * executes some instructions.
 */

#define SAVED_PC_AFTER_CALL(frame) FRAME_SAVED_PC(frame)

/* Address of end of stack space.
 *	This probably doesn't matter for the i960, because it's only used
 *	in manipulation of core files, which we don't support.
 */

#define STACK_END_ADDR (0xfe000000)

/* Stack grows upward
 */

#define INNER_THAN >

/* Sequence of bytes for breakpoint instruction
 */

#define BREAKPOINT {0x00, 0x3e, 0x00, 0x66}

/* Amount ip must be decremented by after a breakpoint.
 * This is often the number of bytes in BREAKPOINT but not always.
 */

#define DECR_PC_AFTER_BREAK 0

/* Nonzero if instruction at ip is a return instruction.
 */
#define ABOUT_TO_RETURN(ip) (read_memory_integer(ip,4) == 0x0a000000)

/* Return 1 if P points to an invalid floating point value.
 * LEN is the length in bytes.
 */
#define INVALID_FLOAT(p, len) (0)

/* How long (ordinary) registers are
 */
#define REGISTER_TYPE long

/* Number of machine registers */
#define NUM_REGS 40

/* Initializer for an array of names of registers.
 * There should be NUM_REGS strings in this initializer.
 */
#define REGISTER_NAMES { \
	/*  0 */	"pfp", "sp",  "rip", "r3",  "r4",  "r5",  "r6",  "r7", \
	/*  8 */	"r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15",\
	/* 16 */	"g0",  "g1",  "g2",  "g3",  "g4",  "g5",  "g6",  "g7", \
	/* 24 */	"g8",  "g9",  "g10", "g11", "g12", "g13", "g14", "fp", \
	/* 32 */	"pc",  "ac",  "ip",  "tc",  "fp0", "fp1", "fp2", "fp3" \
}

/* Register numbers of various important registers (used to index
 * into arrays of register names and register values).
 */

#define FP_REGNUM  31	/* Contains address of executing stack frame */
#define SP_REGNUM   1	/* Contains address of top of stack */
#define IP_REGNUM  34	/* Instruction pointer */
#define FP0_REGNUM 36	/* First floating point register */
#define R0_REGNUM   0	/* First local register		*/
#define RIP_REGNUM  2	/* Return pointer, local r2	*/
#define R15_REGNUM 15	/* Last local register		*/
#define G0_REGNUM  16	/* First global register	*/
#define G13_REGNUM 29
#define G14_REGNUM 30

/* Some registers have more than one name
 */
#define PC_REGNUM  IP_REGNUM	/* GDB refers to ip as the Program Counter */
#define PFP_REGNUM R0_REGNUM	/* Previous frame pointer	*/

/* Not needed, because we don't support core files:
 *	#define KERNEL_U_ADDR
 *	#define REGISTER_U_ADDR(addr, blockend, regno)
 */

/* Total amount of space needed to store our copies of the machine's
 * register state, the array `registers'.
 */
#define REGISTER_BYTES ((36*4) + (4*8))

/* Index within `registers' of the first byte of the space for register N.
 */

#define REGISTER_BYTE(N) ( (N) < FP0_REGNUM ? \
				(4*(N)) : ((8*(N)) - (4*FP0_REGNUM)) )

/* The i960 has register windows.  */

#define HAVE_REGISTER_WINDOWS

/* Is this register part of the register window system?  A yes answer
   implies that 1) The name of this register will not be the same in
   other frames, and 2) This register is automatically "saved" upon
   subroutine calls and thus there is no need to search more than one
   stack frame for it. */

#define REGISTER_IN_WINDOW_P(regnum)	((regnum) <= R15_REGNUM)

/* Number of bytes of storage in the actual machine representation
 * for register N.  On the i960, all regs are 4 bytes except for floating
 * point, which are 10;  but NINDY only preserves and sends us 8 bytes,
 * so we'll just treat them as if that's their raw size.
 */

#define REGISTER_RAW_SIZE(N)		( (N) < FP0_REGNUM ? 4 : 8 )

/* Number of bytes of storage in the program's representation for register N.
 */

#define REGISTER_VIRTUAL_SIZE(N)	( (N) < FP0_REGNUM ? 4 : 8 )

/* Largest value REGISTER_RAW_SIZE can have.  */
#define MAX_REGISTER_RAW_SIZE 8

/* Largest value REGISTER_VIRTUAL_SIZE can have.  */
#define MAX_REGISTER_VIRTUAL_SIZE 8

/* Nonzero if register N requires conversion from raw format to virtual format.
   This would be as commented out below if NINDY really preserved the
   full extended float registers.  As it is, no regs need conversion.
 */
/* #define REGISTER_CONVERTIBLE(N) ((N) >= FP0_REGNUM) */
#define	REGISTER_CONVERTIBLE(N) 0

/* Convert data from raw format for register REGNUM to virtual format for
 * register REGNUM:  no conversion necessary.
 */
#define REGISTER_CONVERT_TO_VIRTUAL(REGNUM,FROM,TO)	\
			bcopy((FROM), (TO), REGNUM < FP0_REGNUM ? 4 : 8)

/* Convert data from virtual format for register REGNUM raw format for
 * register REGNUM:  no conversion necessary.
 */
#define REGISTER_CONVERT_TO_RAW(REGNUM,FROM,TO)		\
			bcopy((FROM), (TO), REGNUM < FP0_REGNUM ? 4 : 8)

/* Return the GDB type object for the "standard" data type
 * of data in register N.
 */
#define REGISTER_VIRTUAL_TYPE(N) ((N) < FP0_REGNUM ? \
					builtin_type_int : builtin_type_double)

/* Extract from an array REGBUF containing the (raw) register state
   a function return value of type TYPE, and copy that, in virtual format,
   into VALBUF.  This is only called if USE_STRUCT_CONVENTION for this
   type is 0.

   On the i960 we just take as many bytes as we need from G0 through G3.  */

#define EXTRACT_RETURN_VALUE(TYPE,REGBUF,VALBUF) \
	bcopy(REGBUF+REGISTER_BYTE(G0_REGNUM), VALBUF, TYPE_LENGTH (TYPE))

/* Write into appropriate registers a function return value
 * of type TYPE, given in virtual format.
 *
 * For 'return' command:  not (yet) implemented for i960.
 */
#define STORE_RETURN_VALUE(TYPE,VALBUF) \
    error ("Returning values from functions is not implemented in i960 gdb")

/* Does the specified function use the "struct returning" convention
   or the "value returning" convention?  The "value returning" convention
   almost invariably returns the entire value in registers.  The
   "struct returning" convention often returns the entire value in
   memory, and passes a pointer (out of or into the function) saying
   where the value (is or should go).

   Since this sometimes depends on whether it was compiled with GCC,
   this is also an argument.  This is used in call_function to build a
   stack, and in value_being_returned to print return values.

   On i960, a structure is returned in registers g0-g3, if it will fit.
   If it's more than 16 bytes long, g13 pointed to it on entry.  */

#define USE_STRUCT_CONVENTION(gcc_p, type) (TYPE_LENGTH (type) > 16)

/* If USE_STRUCT_CONVENTION produces a 1, 
   extract from an array REGBUF containing the (raw) register state
   the address in which a function should return its structure value,
   as a CORE_ADDR (or an expression that can be used as one).

   Address of where to put structure was passed in in global
   register g13 on entry.  God knows what's in g13 now.  The
   (..., 0) below is to make it appear to return a value, though
   actually all it does is call error().  */

#define EXTRACT_STRUCT_VALUE_ADDRESS(REGBUF) \
   (error("Don't know where large structure is returned on i960"), 0)

/* Store the address of the place in which to copy the structure the
   subroutine will return.  This is called from call_function. */

#define STORE_STRUCT_RETURN(ADDR, SP) \
    error ("Returning values from functions is not implemented in i960 gdb")

/* Take a FRAME * and produces the frame's chain-pointer
 * (FRAME_FP of previous frame).
 *
 * On the i960, this is done by getting the Previous Frame Pointer out of a
 * frame.  Be careful to clear low 4 bits, since status info is saved in them.
 */

#define FRAME_CHAIN(f) 		( read_memory_integer(FRAME_FP((f)),4) & ~0xf )

/*
 * FRAME_CHAIN_COMBINE takes the chain pointer and the frame's nominal address
 * and produces the nominal address of the caller frame.
 *
 * FRAME_CHAIN_VALID returns zero if the given frame is the outermost one
 * and has no caller.  In that case, FRAME_CHAIN_COMBINE is not used.
 */

#define FRAME_CHAIN_COMBINE(chain,thisframe)	(chain)

#define	FRAME_CHAIN_VALID(chain, thisframe) \
	i960_frame_chain_valid (chain, thisframe)

extern int i960_frame_chain_valid();		/* See i960-tdep.c */

/* Note that in the i960 architecture the return pointer is saved in the
 * *caller's* stack frame.
 *
 * Make sure to zero low-order bits because of bug in 960CA A-step part
 * (instruction addresses should always be word-aligned anyway).
 */
#define FRAME_SAVED_PC(frame) \
			((read_memory_integer(FRAME_CHAIN(frame)+8,4)) & ~3)

/* We use this instead of FRAME_ARGS_ADDR;  it takes a different set of
 * arguments.  FIXME.
 */
/* extern CORE_ADDR arg_address(); */		/* See i960-tdep.c */
/* FIXME FIXME, for now we use the same FRAME_LOCALS_ADDRESS.
   The gcc-1.2 compiler seems to output 'name:p' debug symbols for all symbols,
   whether relative to g14 or FP.  The right answer remains to be seen.  */

#define FRAME_ARGS_ADDRESS(fi) FRAME_LOCALS_ADDRESS (fi)

/* FIXME, this is the same except it should return Frame_unknown when
   it does not really know where the args are.  */

#define	FRAME_ARGS_ADDRESS_CORRECT(fi)  FRAME_ARGS_ADDRESS(fi)

#define FRAME_LOCALS_ADDRESS(fi)	(fi)->frame

/* Return number of args passed to a frame.
 * Can return -1, meaning no way to tell.
 */
#define FRAME_NUM_ARGS(numargs, fi)	(numargs = -1)

/* Return number of bytes at start of arglist that are not really args.  */

#define FRAME_ARGS_SKIP 0

/* Produce the positions of the saved registers in a stack frame.  */

#define FRAME_FIND_SAVED_REGS(frame_info_addr, sr) \
	i960_frame_find_saved_regs (frame_info_addr->frame, &sr)

extern void i960_frame_find_saved_regs();		/* See i960-tdep.c */


/* Print status when we get a random unexpected signal.  We have more
   kinds of signals than Unix does... */

#define	PRINT_RANDOM_SIGNAL(stop_signal) i960_print_fault (stop_signal)

/***********************************************************************
 *                                                                     *
 * THINGS NEEDED FOR MAKING CALLS TO FUNCTIONS IN THE INFERIOR PROCESS *
 *                                                                     *
 ***********************************************************************/

/* Push an empty stack frame, to record the current ip, etc.
 *
 * Not (yet?) implemented for i960.
 */
#define PUSH_DUMMY_FRAME	\
error("Function calls into the inferior process are not supported on the i960")

/* Discard from the stack the innermost frame, restoring all registers.
 *
 * Not (yet?) implemented for i960.
 */
#define POP_FRAME \
	error ("Premature returns are not supported for the i80960")


/* This sequence of words is the instructions
 *
 *	callx 0x00000000
 *	fmark
 */

/* #define CALL_DUMMY { 0x86003000, 0x00000000, 0x66003e00 } */

/* #define CALL_DUMMY_START_OFFSET 0 *//* Start execution at beginning of dummy */

/* Indicate that we don't support calling inferior child functions.  */

#undef CALL_DUMMY

/* Insert the specified number of args and function address
 * into a call sequence of the above form stored at 'dummyname'.
 *
 * Ignore arg count on i960.
 */

/* #define FIX_CALL_DUMMY(dummyname, fun, nargs) *(((int *)dummyname)+1) = fun */

#undef FIX_CALL_DUMMY


/*************************************************
 *                                               *
 * INTERFACE DEFINITIONS FOR KERNEL DEBUGGER KDB *
 *                                               *
 *************************************************/

/* Not relevant */
