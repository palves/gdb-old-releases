/* ELF32/HPPA support

   This file contains ELF32/HPPA relocation support as specified
   in the Stratus FTX/Golf Object File Format (SED-1762) dated
   November 19, 1992.

   Copyright (C) 1990-1991 Free Software Foundation, Inc.

   Written by:

   Center for Software Science
   Department of Computer Science
   University of Utah

   This file is part of BFD, the Binary File Descriptor library.

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

#ifndef _ELF32_HPPA_H
#define _ELF32_HPPA_H

#include "libelf.h"

/* ELF/HPPA relocation types */

typedef enum
  {
    /* 9.3.4. Address relocation types
       These relocation types do simple base + offset relocations.

       By convention, relocation type zero is always "no relocation",
       while type one is 32-bit word relocation. */

    R_HPPA_NONE,	/*              -                       -       */
    R_HPPA_32,		/*              Symbol + Addend         32      */
    R_HPPA_11,		/*              Symbol + Addend         11      */
    R_HPPA_14,		/*              Symbol + Addend         11      */
    R_HPPA_17,		/*              Symbol + Addend         11      */
    R_HPPA_L21,		/*              L (Symbol, Addend)      21      */
    R_HPPA_R11,		/*              R (Symbol, Addend)      11      */
    R_HPPA_R14,		/*              R (Symbol, Addend)      14      */
    R_HPPA_R17,		/*              R (Symbol, Addend)      17      */
    R_HPPA_LS21,	/*              LS(Symbol, Addend)      21      */
    R_HPPA_RS11,	/*              RS(Symbol, Addend)      11      */
    R_HPPA_RS14,	/*              RS(Symbol, Addend)      14      */
    R_HPPA_RS17,	/*              RS(Symbol, Addend)      17      */
    R_HPPA_LD21,	/*              LD(Symbol, Addend)      21      */
    R_HPPA_RD11,	/*              RD(Symbol, Addend)      11      */
    R_HPPA_RD14,	/*              RD(Symbol, Addend)      14      */
    R_HPPA_RD17,	/*              RD(Symbol, Addend)      17      */
    R_HPPA_LR21,	/*              LR(Symbol, Addend)      21      */
    R_HPPA_RR14,	/*              RR(Symbol, Addend)      14      */
    R_HPPA_RR17,	/*              RR(Symbol, Addend)      17      */

    /* 9.3.5. GOTOFF address relocation types

       The Global Offset Table (GOT) is a table of pointers to data, but
       its address can also be used as a base pointer to address data,
       similar to the way the DP is used in HP/UX.  The expression
       calculation yields a signed offset of an address from the GOT.  */

    R_HPPA_GOTOFF_11,		/*      Symbol - GOT + Addend   11      */
    R_HPPA_GOTOFF_14,		/*      Symbol - GOT + Addend   14      */
    R_HPPA_GOTOFF_L21,		/*      L (Sym - GOT, Addend)   21      */
    R_HPPA_GOTOFF_R11,		/*      R (Sym - GOT, Addend)   11      */
    R_HPPA_GOTOFF_R14,		/*      R (Sym - GOT, Addend)   14      */
    R_HPPA_GOTOFF_LS21,		/*      LS(Sym - GOT, Addend)   21      */
    R_HPPA_GOTOFF_RS11,		/*      RS(Sym - GOT, Addend)   11      */
    R_HPPA_GOTOFF_RS14,		/*      RS(Sym - GOT, Addend)   14      */
    R_HPPA_GOTOFF_LD21,		/*      LD(Sym - GOT, Addend)   21      */
    R_HPPA_GOTOFF_RD11,		/*      RD(Sym - GOT, Addend)   11      */
    R_HPPA_GOTOFF_RD14,		/*      RD(Sym - GOT, Addend)   14      */
    R_HPPA_GOTOFF_LR21,		/*      LR(Sym - GOT, Addend)   21      */
    R_HPPA_GOTOFF_RR14,		/*      RR(Sym - GOT, Addend)   14      */

    /* 9.3.6. Absolute call relocation types

       Relocations of function calls must be accompanied by parameter
       relocation information.  This information is carried in the ten
       high-order bits of the addend field.  The remaining 22 bits of
       of the addend field are sign-extended to form the Addend.

       Note the code to build argument relocations depends on the 
       addend being zero.  A consequence of this limitation is GAS
       can not perform relocation reductions for function symbols.  */

    R_HPPA_ABS_CALL_11,		/*      Symbol + Addend         11      */
    R_HPPA_ABS_CALL_14,		/*      Symbol + Addend         14      */
    R_HPPA_ABS_CALL_17,		/*      Symbol + Addend         17      */
    R_HPPA_ABS_CALL_L21,	/*      L (Symbol, Addend)      21      */
    R_HPPA_ABS_CALL_R11,	/*      R (Symbol, Addend)      11      */
    R_HPPA_ABS_CALL_R14,	/*      R (Symbol, Addend)      14      */
    R_HPPA_ABS_CALL_R17,	/*      R (Symbol, Addend)      17      */
    R_HPPA_ABS_CALL_LS21,	/*      LS(Symbol, Addend)      21      */
    R_HPPA_ABS_CALL_RS11,	/*      RS(Symbol, Addend)      11      */
    R_HPPA_ABS_CALL_RS14,	/*      RS(Symbol, Addend)      14      */
    R_HPPA_ABS_CALL_RS17,	/*      RS(Symbol, Addend)      17      */
    R_HPPA_ABS_CALL_LD21,	/*      LD(Symbol, Addend)      21      */
    R_HPPA_ABS_CALL_RD11,	/*      RD(Symbol, Addend)      11      */
    R_HPPA_ABS_CALL_RD14,	/*      RD(Symbol, Addend)      14      */
    R_HPPA_ABS_CALL_RD17,	/*      RD(Symbol, Addend)      17      */
    R_HPPA_ABS_CALL_LR21,	/*      LR(Symbol, Addend)      21      */
    R_HPPA_ABS_CALL_RR14,	/*      RR(Symbol, Addend)      14      */
    R_HPPA_ABS_CALL_RR17,	/*      RR(Symbol, Addend)      17      */

    /* 9.3.7. PC-relative call relocation types

       PC-relative relocation calculates the difference between an address
       and the location being relocated.  This is most often used to
       relocate pc-relative calls.  They are otherwise identical to 
       their corresponding absolute call relocations.  */

    R_HPPA_PCREL_CALL_11,	/*      Symbol - PC + Addend    11      */
    R_HPPA_PCREL_CALL_14,	/*      Symbol - PC + Addend    14      */
    R_HPPA_PCREL_CALL_17,	/*      Symbol - PC + Addend    17      */
    R_HPPA_PCREL_CALL_12,	/*      Symbol - PC + Addend    12      */
    R_HPPA_PCREL_CALL_L21,	/*      L (Symbol - PC, Addend) 21      */
    R_HPPA_PCREL_CALL_R11,	/*      R (Symbol - PC, Addend) 11      */
    R_HPPA_PCREL_CALL_R14,	/*      R (Symbol - PC, Addend) 14      */
    R_HPPA_PCREL_CALL_R17,	/*      R (Symbol - PC, Addend) 17      */
    R_HPPA_PCREL_CALL_LS21,	/*      LS(Symbol - PC, Addend) 21      */
    R_HPPA_PCREL_CALL_RS11,	/*      RS(Symbol - PC, Addend) 11      */
    R_HPPA_PCREL_CALL_RS14,	/*      RS(Symbol - PC, Addend) 14      */
    R_HPPA_PCREL_CALL_RS17,	/*      RS(Symbol - PC, Addend) 17      */
    R_HPPA_PCREL_CALL_LD21,	/*      LD(Symbol - PC, Addend) 21      */
    R_HPPA_PCREL_CALL_RD11,	/*      RD(Symbol - PC, Addend) 11      */
    R_HPPA_PCREL_CALL_RD14,	/*      RD(Symbol - PC, Addend) 14      */
    R_HPPA_PCREL_CALL_RD17,	/*      RD(Symbol - PC, Addend) 17      */
    R_HPPA_PCREL_CALL_LR21,	/*      LR(Symbol - PC, Addend) 21      */
    R_HPPA_PCREL_CALL_RR14,	/*      RR(Symbol - PC, Addend) 14      */
    R_HPPA_PCREL_CALL_RR17,	/*      RR(Symbol - PC, Addend) 17      */

    /* 9.3.8. Plabel relocation types

       Plabels are designed to allow code pointers to be passed between
       spaces.  

       Plabels are procedure markers.  They are used to denote relocations
       which involve procedures (call, loading the address of a procedure,
       etc).  They are necessary for the proper functioning of argument
       relocations. The addend of the relocation should be either 0 (no 
       static link) or 2 (static link required).  These relocations
       correspond to the P%, LP% and RP% field selectors.  */

    R_HPPA_PLABEL_32,		/*      F(Plabel(Symbol,Addend),0)  32	*/
    R_HPPA_PLABEL_11,		/*      F(Plabel(Symbol,Addend),0)  11	*/
    R_HPPA_PLABEL_14,		/*      F(Plabel(Symbol,Addend),0)  14	*/
    R_HPPA_PLABEL_L21,		/*      L(Plabel(Symbol,Addend),0)  21	*/
    R_HPPA_PLABEL_R11,		/*      R(Plabel(Symbol,Addend),0)  11	*/
    R_HPPA_PLABEL_R14,		/*      R(Plabel(Symbol,Addend),0)  14	*/

    /* 9.3.9. Data linkage table (DLT) relocation types

       SOM DLT_REL fixup requests are used to for static data references
       from position-independent code within shared libraries.  They are
       similar to the GOT relocation types in some SVR4 implementations.  */

    R_HPPA_DLT_32,		/*      F(DLTOFF)               32	*/
    R_HPPA_DLT_11,		/*      F(DLTOFF)               11	*/
    R_HPPA_DLT_14,		/*      F(DLTOFF)               14	*/
    R_HPPA_DLT_L21,		/*      L(DLTOFF)               21	*/
    R_HPPA_DLT_R11,		/*      R(DLTOFF)               11	*/
    R_HPPA_DLT_R14,		/*      R(DLTOFF)               14	*/

    /* 9.3.10. Relocations for unwinder tables

       The unwinder table consists of a series of four-word entries, the
       first two of which are a pair of code addresses.  While it would be
       possible to relocate this table using just R_HPPA_32, the amount of
       relocation data resulting would be very large.  To reduce that data,
       the following relocation types have been defined.

       The first, R_HPPA_UNWIND_ENTRY, merely compresses two R_HPPA_32
       operations into one.  It is designed for use in .rel-type
       relocations, where the two 32-bit addends are taken from the unwind
       section itself.

       The second, which is designed for use in .rela-type relocations, is
       designed to relocate an entire unwinder table with one relocation
       entry.  It has the effect of multiple R_HPPA_UNWIND_ENTRY
       relocations applied to successive unwinder table entries.  The
       number of entries to be relocated is given in the r_addend field of
       the relocation entry.  The rest of the relocation entry is used in
       a normal way--r_offset is the offset of the first unwind entry in
       the section, while ELF32_R_SYM(r_info) is the code section that all
       the code addresses should be relocated from.

       Why can't we use begin/end markers + unwind description bits like
       som?  FIXME!  */
       
    R_HPPA_UNWIND_ENTRY,	/*                            "128"   */
    R_HPPA_UNWIND_ENTRIES,	/*                    Addend * "128"  */

    /*  9.3.11. Relocation types for complex expressions

	New-format SOM fixups support complex expressions by spreading 
	the parts of the expression across multiple entries.  ELF for 
	HPPA will have a similar mechanism, although support for it may 
	be optional.  There are two main reasons for defining it:  first,
	the need to translate complex SOM fixup expressions to ELF, and
	second, to cover combinations of expression, field and format not
	available with other relocation types.

	ELF expression relocation entries are interpreted as postfix-form
	expressions.  They may be evaluated using a push-down stack.

	Usually, the addend field of these expression relocation entries is
	unused, with the following exceptions:

	R_HPPA_PUSH_CONST: The addend field contains the constant.

	R_HPPA_PUSH_PROC: The high-order 10 bits of the addend field
	contain parameter relocation information.  The rest ofthe addend 
	field is unused.

	R_HPPA_LSHIFT, R_HPPA_ARITH_RSHIFT and R_HPPA_LOGIC_RSHIFT:
	The addend field normally gives the amount to shift.
	However, if that amount is zero, the shift amount is
	popped from the top of the stack prior to popping the
	amount to be shifted.  */

    R_HPPA_PUSH_CONST,		/*      push Addend                  -   -   */
    R_HPPA_PUSH_PC,		/*      push PC + Addend             -   -   */
    R_HPPA_PUSH_SYM,		/*      push Symbol + Addend         -   -   */
    R_HPPA_PUSH_GOTOFF,		/*      push Symbol - GOT + Addend   -   -   */
    R_HPPA_PUSH_ABS_CALL,	/*      push Symbol + Addend         -   -   */
    R_HPPA_PUSH_PCREL_CALL,	/*      push Symbol - PC + Addend    -   -   */
    R_HPPA_PUSH_PLABEL,		/*      push Plabel(Symbol)          -   -   */
    R_HPPA_MAX,			/*      pop A and B, push max(B,A)   -   -   */
    R_HPPA_MIN,			/*      pop A and B, push min(B,A)   -   -   */
    R_HPPA_ADD,			/*      pop A and B, push B + A      -   -   */
    R_HPPA_SUB,			/*      pop A and B, push B - A      -   -   */
    R_HPPA_MULT,		/*      pop A and B, push B * A      -   -   */
    R_HPPA_DIV,			/*      pop A and B, push B / A      -   -   */
    R_HPPA_MOD,			/*      pop A and B, push B % A      -   -   */
    R_HPPA_AND,			/*      pop A and B, push B & A      -   -   */
    R_HPPA_OR,			/*      pop A and B, push B | A      -   -   */
    R_HPPA_XOR,			/*      pop A and B, push B ^ A      -   -   */
    R_HPPA_NOT,			/*      pop A, push ~A               -   -   */
    R_HPPA_LSHIFT,		/*      pop A, push A << Addend      -   -   */
    R_HPPA_ARITH_RSHIFT,	/*      pop A, push A >> Addend      -   -   */
    R_HPPA_LOGIC_RSHIFT,	/*      pop A, push A >> Addend      -   -   */
    R_HPPA_EXPR_F,		/*      pop A, push A + Addend       F   -   */
    R_HPPA_EXPR_L,		/*      pop A, push L(A,Addend)      L   -   */
    R_HPPA_EXPR_R,		/*      pop A, push R(A,Addend)      R   -   */
    R_HPPA_EXPR_LS,		/*      pop A, push LS(A,Addend)     LS  -   */
    R_HPPA_EXPR_RS,		/*      pop A, push RS(A,Addend)     RS  -   */
    R_HPPA_EXPR_LD,		/*      pop A, push LD(A,Addend)     LD  -   */
    R_HPPA_EXPR_RD,		/*      pop A, push RD(A,Addend)     RD  -   */
    R_HPPA_EXPR_LR,		/*      pop A, push LR(A,Addend)     LR  -   */
    R_HPPA_EXPR_RR,		/*      pop A, push RR(A,Addend)     RR  -   */
    R_HPPA_EXPR_32,		/*      pop                          -   32  */
    R_HPPA_EXPR_21,		/*      pop                          -   21  */
    R_HPPA_EXPR_11,		/*      pop                          -   11  */
    R_HPPA_EXPR_14,		/*      pop                          -   14  */
    R_HPPA_EXPR_17,		/*      pop                          -   17  */
    R_HPPA_EXPR_12,		/*      pop                          -   12  */
    R_HPPA_STUB_CALL_17,	/*      Symbol + Addend              -   17  */
    R_HPPA_UNIMPLEMENTED	/*      N/A                                  */
  }
elf32_hppa_reloc_type;

#define ELF_HOWTO_TABLE_SIZE	R_HPPA_UNIMPLEMENTED + 1
#define N_HPPA_RELOCS		R_HPPA_UNIMPLEMENTED + 1

/* Define groups of basic relocations.  FIXME:  These should
   be the only basic relocations created by GAS.  The rest
   should be internal to the BFD backend.  

   The idea is both SOM and ELF define these basic relocation
   types so they map into a SOM or ELF specific relocation 
   as appropriate.  This allows GAS to share much more code
   between the two target object formats.  */

#define	R_HPPA				R_HPPA_32
#define	R_HPPA_GOTOFF			R_HPPA_GOTOFF_11
#define	R_HPPA_ABS_CALL			R_HPPA_ABS_CALL_11
#define	R_HPPA_PCREL_CALL		R_HPPA_PCREL_CALL_11
#define	R_HPPA_PLABEL			R_HPPA_PLABEL_32
#define	R_HPPA_DLT			R_HPPA_DLT_32
#define R_HPPA_UNWIND			R_HPPA_UNWIND_ENTRY
#define	R_HPPA_COMPLEX			R_HPPA_PUSH_CONST
#define	R_HPPA_COMPLEX_PCREL_CALL	R_HPPA_PUSH_CONST + 1
#define	R_HPPA_COMPLEX_ABS_CALL		R_HPPA_PUSH_CONST + 2

/* HPPA Section types */
#define SHT_HPPA_SYMEXTN		SHT_LOPROC

/* HPPA Symbol types */
#define STT_HPPA_PLABEL			STT_LOPROC

/* HPPA symbol table extension entry types */
enum elf32_hppa_symextn_types
{
  HPPA_SXT_NULL,
  HPPA_SXT_SYMNDX,
  HPPA_SXT_ARG_RELOC,
};

/* These macros compose and decompose the value of a symextn entry:
 
   entry_type = ELF32_HPPA_SX_TYPE(word);
   entry_value = ELF32_HPPA_SX_VAL(word);
   word = ELF32_HPPA_SX_WORD(type,val);  */

#define ELF32_HPPA_SX_TYPE(p)		((p) >> 24)
#define ELF32_HPPA_SX_VAL(p)		((p) & 0xFFFFFF)
#define ELF32_HPPA_SX_WORD(type,val)	(((type) << 24) + (val & 0xFFFFFF))

/* The following was added facilitate implementation of the .hppa_symextn
   section.  This section is built after the symbol table is built in the
   elf_write_object_contents routine (called from bfd_close).  It is built
   so late because it requires information that is not known until
   the symbol and string table sections have been allocated, and
   the symbol table has been built. */

/* Number of "hand-made" target specific sections.  */
#define ELF_TC_FAKE_SECTIONS 1
#define SYMEXTN_SECTION_NAME ".hppa_symextn"

/* FIXME.  Are these external?  (For example used by GAS?).  If so the
   names need to change to avoid namespace pollution, if not they should
   be moved into elf32-hppa.c.  */
typedef Elf32_Word symext_entryS;
struct symext_chain
  {
    symext_entryS entry;
    struct symext_chain *next;
  };

typedef struct symext_chain symext_chainS;

void elf_hppa_tc_symbol PARAMS ((bfd *, elf_symbol_type *, int));
void elf_hppa_tc_make_sections PARAMS ((bfd *, PTR));
void hppa_elf_stub_finish PARAMS ((bfd *));
elf32_hppa_reloc_type **hppa_elf_gen_reloc_type PARAMS ((bfd *, 
							 elf32_hppa_reloc_type,
							 int, int));

#endif /* _ELF32_HPPA_H */
