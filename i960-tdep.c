/* Miscellaneous i80960-dependent routines.
 * Some replace macros normally defined in "param.h".
 */

#include <stdio.h>
#include "defs.h"
#include "param.h"
#include "symtab.h"
#include "value.h"
#include "frame.h"




/* Return address in user program of a variable *symP relative to stack
 * frame *fiP.  We assume that the type is known to be a memory argument,
 * either a LOC_ARG (relative to fp) or a LOC_ARG_BLOCK (relative to g14).
 *
 * We have no ability to locate the latter unless: (1) register g14 is non-zero
 * (and this is not a BAL routine -- which we have to assume), meaning that the
 * arguments wouldn't all fit in registers AND (2) we were actually executing
 * the routine, i.e., this is the active frame.  Even then we can't be SURE the
 * routine hasn't copied and then clobbered g14.
 */
#ifdef FIXME
CORE_ADDR
arg_address( symP, fiP )
    struct symbol *symP;
    struct frame_info *fiP;
{
	CORE_ADDR addr;

	addr = 0;

	switch ( SYMBOL_CLASS(symP) ){
	case LOC_ARG:
		addr = fiP->frame;
		break;
#ifdef LOC_ARG_BLOCK    /* FIXME */
	case LOC_ARG_BLOCK:
		if ( fiP->next_frame == 0 ){  /* This was the executing frame */
			addr = read_register(G14_REGNUM);
		}
		break;
#endif
	default:
		error( "arg_address received non-memarg" );
		break;
	}

	if ( addr == 0 ){
		dunno = 1;
	} else {
		addr += SYMBOL_VALUE(symP);
	}
	return addr;
}
#endif

/*
 * 'start_frame' is a variable in the NINDY runtime startup routine
 * that contains the frame pointer of the 'start' routine (the routine
 * that calls 'main').  By reading its contents out of remote memory,
 * we can tell where the frame chain ends:  backtraces should halt before
 * they display this frame.
 */

int
i960_frame_chain_valid( chain, curframe )
    unsigned int chain;
    FRAME curframe;
{
	struct symbol *sym;
	int i;

	/* crtnindy.o is an assembler module that is assumed to be linked
	 * first in an i80960 executable.  It contains the true entry point;
	 * it performs startup up initialization and then calls 'main'.
	 *
	 * 'sf' is the name of a variable in crtnindy.o that is set
	 *	during startup to the address of the first frame.
	 *
	 * 'a' is the address of that variable in 80960 memory.
	 */
	static char sf[] = "start_frame";
	CORE_ADDR a;


	chain &= ~0x3f; /* Zero low 6 bits because previous frame pointers
			 * contain return status info in them.
			 */
	if ( chain == 0 ){
		return 0;
	}

	sym = lookup_symbol(sf, 0, VAR_NAMESPACE, (int *)NULL, 
				  (struct symtab **)NULL);
	if ( sym != 0 ){
		a = sym->value.value;
	} else {
		for ( i = 0; strcmp(misc_function_vector[i].name,sf); i++ ){
			if ( i >= misc_function_count ){
				return 0;
			}
		}
		a = misc_function_vector[i].address;
	}

	return ( chain != read_memory_integer(a,4) );
}


/* Put here the code to store, into a struct 'saved_regs',
 * the addresses of the saved registers of frame described by 'fi'.
 * This includes special registers such as pc and fp saved in special
 * ways in the stack frame.  sp is even more special:
 * the address we return for it IS the sp for the next frame.
 *
 * Note that on the i80960, only r0-r15 are saved, but
 *	r0 is the previous frame pointer
 *	r1 is the stack pointer
 *	r2 is return instruction pointer
 *
 * Note that the i80960 implementation has different arguments than
 * other versions of GDB.
 */

void
i960_frame_find_saved_regs( frame, sr )
    register CORE_ADDR frame;
    register struct frame_saved_regs *sr;
{
	register int r;		/* Register number */

	bzero( sr, sizeof(*sr) );
	for ( r = R0_REGNUM; r <= R15_REGNUM; r++, frame += 4 ){
		sr->regs[r] =
			(r == SP_REGNUM) ? read_memory_integer(frame,4) : frame;
	}
}

/************************************************************************
 *
 * THE FOLLOWING CODE IS ALL RELATED TO RUNNING ON A HOST THAT USES
 * BIG-ENDIAN BYTE ORDERING.
 *
 * Note that the following gdb routines transfer data between the host and the
 * i80960:
 *
 *	read_memory_integer
 *	read_register
 *	write_register
 *		These routines always operate on an integer, so they can call
 *		the 'byteswap()' routine (below) themselves;  their callers
 *		don't need to do any additional work.
 *
 *	read_inferior_memory
 *	write_inferior_memory
 *		These routines are called only from read_memory() and
 *		write_memory(), after it's been determined that we need to go
 *		to the inferior process and not to a core file to get data.
 *		Since this is ALWAYS the case for the i960, it's sufficient
 *		for us to handle the read_memory() and write_memory() cases.
 *
 *	read_memory
 *	write_memory
 *	read_register_bytes
 *	write_register_bytes
 *		Every call to these routines has to be special-cased.
 *
 *		Callers transferring byte strings (e.g., text, or opcodes stored
 *		in character arrays) that don't require any byte-reordering need
 *		do nothing.
 *
 *		Callers transferring a single raw number (integer or floating
 *		point) need to call 'byteswap()' on the affected data before
 *		writing it or after reading it.
 *
 *		Callers transferring data into or out of a gdb 'value' structure
 *		need to call 'byteswap_val()' on the affected data before
 *		writing it or after reading it.
 *
 ************************************************************************/

static char bigend_host = 0;	/* True iff host uses bigend byte ordering */


/* gdb960 is always running on a non-960 host.  Check it's characteristics.
 * This routine must be called as part of gdb initialization.
 */
check_host()
{
	int i;
	short hmmm = 0x1234;

	static struct typestruct {
		int hostsize;		/* Size of type on host		*/
		int i960size;		/* Size of type on i960		*/
		char *typename;		/* Name of type, for error msg	*/
	} types[] = {
		{ sizeof(short),  2, "short" },
		{ sizeof(int),    4, "int" },
		{ sizeof(long),   4, "long" },
		{ sizeof(float),  4, "float" },
		{ sizeof(double), 8, "double" },
		{ sizeof(char *), 4, "pointer" },
	};
#define TYPELEN	(sizeof(types) / sizeof(struct typestruct))

	/* Make sure that host type sizes are same as i960
	 */
	for ( i = 0; i < TYPELEN; i++ ){
		if ( types[i].hostsize != types[i].i960size ){
			printf("sizeof(%s) != %d:  PROCEED AT YOUR OWN RISK!\n",
					types[i].typename, types[i].i960size );
		}

	}

	/* Check for big-endian host, set flag if it is one
	 */
	if ( *((char *) &hmmm) == 0x12 ){
		bigend_host = 1;
	}
}


/* Reverse the order of a string of bytes if (and only if) the host uses
 * bigendian byte ordering.  Converts data from i960 order to host order or
 * vice versa.
 */
byteswap( p, n )
    char *p;	/* Pointer to source/destination string	*/
    int n;	/* Number of bytes in string		*/
{
	int i;	/* Index into string			*/
	char c;	/* Temp holding place during 2-byte swap*/

	if ( bigend_host ){
		for ( n--, i = 0; i < n; i++, n-- ){
			c = p[i];
			p[i] = p[n];
			p[n] = c;
		}
	}
}

/* Reverse the order of all relevant strings of bytes in a value if (and only
 * if) the host uses bigendian byte ordering.  Converts a value from i960 order
 * to host order or vice versa.
 *
 * A 'value' is a gdb construct that can include complex data structures
 * like arrays and structures.  We can't just swap the entire structure around:
 * we have to find the individual integers and floating point values,
 * iterating on arrays and handling substructures.
 *
 * This routine and the accompanying 'reorder_val()' are based on gdb's
 * 'value_print()' and 'val_print()' routines.
 */
byteswap_val( val )
    value val;
{
	register int i, tlen;

	if ( !bigend_host ){
		return;
	}

	if ( VALUE_REPEATED(val) ){
		tlen = TYPE_LENGTH( VALUE_TYPE(val) );
		for ( i = 0; i < VALUE_REPETITIONS(val); i++ ){
			reorder_val( VALUE_TYPE(val),
					VALUE_CONTENTS(val)+(i*tlen));
		}
	} else {
		/* A simple (nonrepeated) value */
		reorder_val( VALUE_TYPE(val), VALUE_CONTENTS(val) );
	}
}

static
reorder_val( type, valaddr )
     struct type *type;
     char *valaddr;
{
	register int i;
	int len;
	struct type *elttype;
	int eltlen;


	switch ( TYPE_CODE(type) ){

	case TYPE_CODE_PTR:
	case TYPE_CODE_REF:
	case TYPE_CODE_INT:	/* Inlcludes char, short, int, and long */
	case TYPE_CODE_FLT:	/* Includes float and double		*/
	case TYPE_CODE_ENUM:
		if ( TYPE_LENGTH(type) > 1){
			byteswap( valaddr, TYPE_LENGTH(type) );
		}
		break;

	case TYPE_CODE_ARRAY:
		if ( TYPE_LENGTH(type) >= 0 ){

			elttype = TYPE_TARGET_TYPE(type);
			eltlen = TYPE_LENGTH(elttype);
			len = TYPE_LENGTH(type) / eltlen;

			/* For an array of chars, do nothing */
			if ( TYPE_CODE(elttype) == TYPE_CODE_INT && eltlen==1 ){
				break;
			}

			for ( i = 0; i < len; i++ ){
				reorder_val( elttype, valaddr+(i*eltlen) );
			}
		}
		break;

	case TYPE_CODE_STRUCT:
		if ( TYPE_FIELD_PACKED(type,0) ){
			/* Ignore bit fields */
			break;
		}
		len = TYPE_NFIELDS(type);
		for ( i = 0; i < len; i++ ){
			reorder_val(TYPE_FIELD_TYPE(type,i), 
				valaddr+TYPE_FIELD_BITPOS(type,i)/8 );
		}
		break;

	case TYPE_CODE_UNION:
		error( "Can't handle unions on bigend host" );
		break;

	default:
		error( "Invalid type code in 'reorder_val'" );
	}
}


/* Return address to which the currently executing leafproc will return,
 * or 0 if ip is not in a leafproc (or if we can't tell if it is).
 *
 * Do this by finding the starting address of the routine in which address ip.
 * If the instruction there is "mov g14,gNN" (where NN is in [0,7]), this
 * is a leafproc and the return address is in register gNN.
 */
CORE_ADDR
leafproc_return( ip )
    CORE_ADDR ip;	/* ip from currently executing function	*/
{
	struct symbol *func;
	int miscfunc;
	CORE_ADDR func_addr;
	int first_instr;
	int regnum;

	func_addr = 0;
	func = find_pc_function( ip );
	if ( func ){
		func_addr = BLOCK_START(SYMBOL_BLOCK_VALUE(func))
							+ FUNCTION_START_OFFSET;
	} else {
		miscfunc = find_pc_misc_function( ip );
		if ( miscfunc >= 0 ){
			func_addr = misc_function_vector[miscfunc].address
							+ FUNCTION_START_OFFSET;
		}
	}

	if ( func_addr != 0 ){
		first_instr = read_memory_integer( func_addr, 4 );
		if ( (first_instr & ~0x00780000) == 0x5c80161e ){
			/* It's a "mov g14,g??" -- see if g?? is one of g0-g7 */
			regnum = (first_instr >> 19) & 0xf;
			if ( (0 <= regnum) && (regnum <= 7) ){
				if ( ip == func_addr ){
					/* mov hasn't happened yet */
					return read_register(G14_REGNUM);
				} else {
					return read_register(G0_REGNUM+regnum);
				}
			}
		}
	}

	return 0;
}
