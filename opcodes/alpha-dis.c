/* Instruction printing code for the Alpha
   Copyright (C) 1993 Free Software

Foundation, Inc. Contributed by Cygnus Support. 

Written by Steve Chamberlain (sac@cygnus.com) 

This file is part of libopcodes. 

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option)
any later version. 

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details. 

You should have received a copy of the GNU General Public License along with
This program; if not, write to the Free Software Foundation, Inc., 675
 Mass Ave, Cambridge, MA 02139, USA.  
*/

#include "dis-asm.h"
#define DEFINE_TABLE
#include "alpha-opc.h"


/*
 * Print one instruction from MEMADDR on STREAM. Return the size of the
 * instruction (always 4 on a29k).  
 */
int
print_insn_alpha(pc, info)
	bfd_vma         pc;
	struct disassemble_info *info;
{
  alpha_insn     *insn;
  unsigned  char            b[4];
  void           *stream = info->stream;
  fprintf_ftype   func = info->fprintf_func;
  int             given;
  int             status ;
  int found = 0;
  
  status = (*info->read_memory_func) (pc, (bfd_byte *) &b[0], 4, info);
  if (status != 0) {
    (*info->memory_error_func) (status, pc, info);
    return -1;
  }
  given = (b[0]) | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);

  fprintf(stream, "%08x %2x ", given, (given>>26) & 0x3f);
  
  for (insn = alpha_insn_set;
       insn->name && !found;
       insn++)
    {
      switch (insn->type)
	{
	case MEMORY_FORMAT_CODE:
	  if ((insn->i & MEMORY_FORMAT_MASK) 
	      ==(given & MEMORY_FORMAT_MASK))
	    {
	      fprintf(stream, "%s\t%s, %d(%s)",
		      insn->name,
		      alpha_regs[RA(given)],
		      DISP(given),
		      alpha_regs[RB(given)]);
	      found = 1;
	    }
	  break;
	case MEMORY_BRANCH_FORMAT_CODE:
	  if ((insn->i & MEMORY_BRANCH_FORMAT_MASK) 
	      == (given & MEMORY_BRANCH_FORMAT_MASK) )
	    {
	      fprintf(stream, "%s\t%s, 0x%lx\n",
		      insn->name,
		      alpha_regs[RA(given)],
		      (BDISP(given)*4) + pc);
	      found = 1;
	    }
	  break;

	case BRANCH_FORMAT_CODE:
	  if ((insn->i & BRANCH_FORMAT_MASK)
	      == (given & BRANCH_FORMAT_MASK))
	    {
	      if (given & (1<<15)) 
		{
		  fprintf(stream, "%s\t%s, (%s), 1", insn->name,
			  alpha_regs[RA(given)],
			  alpha_regs[RB(given)],
			  alpha_regs[RC(given)]);
		} 
	      else 
		{
		  fprintf(stream, "%s\t%s, (%s), 0x%lx(zero)",
			  alpha_regs[RA(given)],
			  alpha_regs[RB(given)],
			  JUMP_HINT(given) << 2 + pc);
		}
	      found =1 ;
	    }

	  break;
	case OPERATE_FORMAT_CODE:
	  if ((insn->i & OPERATE_FORMAT_MASK)
	      == (given & OPERATE_FORMAT_MASK)) 
	    {
	      if (OP_OPTYPE(insn->i) == OP_OPTYPE(given)) 
		{
		  if (OP_IS_CONSTANT(given)) {
		    fprintf(stream, "%s\t%s, 0x%x, %s", insn->name,
			    alpha_regs[RA(given)],
			    LITERAL(given),
			    alpha_regs[RC(given)]);
		  } else {
		    fprintf(stream, "%s\t%s, %s, %s", insn->name,
			    alpha_regs[RA(given)],
			    alpha_regs[RB(given)],
			    alpha_regs[RC(given)]);
		  }
		  found = 1;
		}
	    }
	      
	  break;
	case FLOAT_FORMAT_CODE:
	  if ((insn->i & OPERATE_FORMAT_MASK)
	      == (given & OPERATE_FORMAT_MASK)) 
	    {
	      fprintf(stream, "%s\t%s, %s, %s", insn->name,
		      alpha_regs[RA(given)],
		      alpha_regs[RB(given)],
		      alpha_regs[RC(given)]);
	    }
	  found = 1;
	}
    }
    
  return 4;
}
