/* CPU data for m32r.

THIS FILE IS MACHINE GENERATED WITH CGEN.

Copyright (C) 1996, 1997, 1998, 1999 Free Software Foundation, Inc.

This file is part of the GNU Binutils and/or GDB, the GNU debugger.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/

#include "sysdep.h"
#include <ctype.h>
#include <stdio.h>
#include "ansidecl.h"
#include "bfd.h"
#include "symcat.h"
#include "m32r-desc.h"
#include "m32r-opc.h"
#include "opintl.h"

/* Attributes.  */

static const CGEN_ATTR_ENTRY bool_attr[] =
{
  { "#f", 0 },
  { "#t", 1 },
  { 0, 0 }
};

static const CGEN_ATTR_ENTRY MACH_attr[] =
{
  { "base", MACH_BASE },
  { "m32r", MACH_M32R },
  { "max", MACH_MAX },
  { 0, 0 }
};

const CGEN_ATTR_TABLE m32r_cgen_ifield_attr_table[] =
{
  { "MACH", & MACH_attr[0] },
  { "VIRTUAL", &bool_attr[0], &bool_attr[0] },
  { "UNSIGNED", &bool_attr[0], &bool_attr[0] },
  { "PCREL-ADDR", &bool_attr[0], &bool_attr[0] },
  { "ABS-ADDR", &bool_attr[0], &bool_attr[0] },
  { "RESERVED", &bool_attr[0], &bool_attr[0] },
  { "SIGN-OPT", &bool_attr[0], &bool_attr[0] },
  { "RELOC", &bool_attr[0], &bool_attr[0] },
  { 0, 0, 0 }
};

const CGEN_ATTR_TABLE m32r_cgen_hardware_attr_table[] =
{
  { "MACH", & MACH_attr[0] },
  { "VIRTUAL", &bool_attr[0], &bool_attr[0] },
  { "UNSIGNED", &bool_attr[0], &bool_attr[0] },
  { "SIGNED", &bool_attr[0], &bool_attr[0] },
  { "CACHE-ADDR", &bool_attr[0], &bool_attr[0] },
  { "FUN-ACCESS", &bool_attr[0], &bool_attr[0] },
  { "PC", &bool_attr[0], &bool_attr[0] },
  { "PROFILE", &bool_attr[0], &bool_attr[0] },
  { 0, 0, 0 }
};

const CGEN_ATTR_TABLE m32r_cgen_operand_attr_table[] =
{
  { "MACH", & MACH_attr[0] },
  { "VIRTUAL", &bool_attr[0], &bool_attr[0] },
  { "UNSIGNED", &bool_attr[0], &bool_attr[0] },
  { "PCREL-ADDR", &bool_attr[0], &bool_attr[0] },
  { "ABS-ADDR", &bool_attr[0], &bool_attr[0] },
  { "SIGN-OPT", &bool_attr[0], &bool_attr[0] },
  { "NEGATIVE", &bool_attr[0], &bool_attr[0] },
  { "RELAX", &bool_attr[0], &bool_attr[0] },
  { "SEM-ONLY", &bool_attr[0], &bool_attr[0] },
  { "RELOC", &bool_attr[0], &bool_attr[0] },
  { "HASH-PREFIX", &bool_attr[0], &bool_attr[0] },
  { 0, 0, 0 }
};

const CGEN_ATTR_TABLE m32r_cgen_insn_attr_table[] =
{
  { "MACH", & MACH_attr[0] },
  { "ALIAS", &bool_attr[0], &bool_attr[0] },
  { "VIRTUAL", &bool_attr[0], &bool_attr[0] },
  { "UNCOND-CTI", &bool_attr[0], &bool_attr[0] },
  { "COND-CTI", &bool_attr[0], &bool_attr[0] },
  { "SKIP-CTI", &bool_attr[0], &bool_attr[0] },
  { "DELAY-SLOT", &bool_attr[0], &bool_attr[0] },
  { "RELAXABLE", &bool_attr[0], &bool_attr[0] },
  { "RELAX", &bool_attr[0], &bool_attr[0] },
  { "NO-DIS", &bool_attr[0], &bool_attr[0] },
  { "PBB", &bool_attr[0], &bool_attr[0] },
  { "FILL-SLOT", &bool_attr[0], &bool_attr[0] },
  { 0, 0, 0 }
};

CGEN_KEYWORD_ENTRY m32r_cgen_opval_h_gr_entries[] = 
{
  { "fp", 13 },
  { "lr", 14 },
  { "sp", 15 },
  { "r0", 0 },
  { "r1", 1 },
  { "r2", 2 },
  { "r3", 3 },
  { "r4", 4 },
  { "r5", 5 },
  { "r6", 6 },
  { "r7", 7 },
  { "r8", 8 },
  { "r9", 9 },
  { "r10", 10 },
  { "r11", 11 },
  { "r12", 12 },
  { "r13", 13 },
  { "r14", 14 },
  { "r15", 15 }
};

CGEN_KEYWORD m32r_cgen_opval_h_gr = 
{
  & m32r_cgen_opval_h_gr_entries[0],
  19
};

CGEN_KEYWORD_ENTRY m32r_cgen_opval_h_cr_entries[] = 
{
  { "psw", 0 },
  { "cbr", 1 },
  { "spi", 2 },
  { "spu", 3 },
  { "bpc", 6 },
  { "bbpsw", 8 },
  { "bbpc", 14 },
  { "cr0", 0 },
  { "cr1", 1 },
  { "cr2", 2 },
  { "cr3", 3 },
  { "cr4", 4 },
  { "cr5", 5 },
  { "cr6", 6 },
  { "cr7", 7 },
  { "cr8", 8 },
  { "cr9", 9 },
  { "cr10", 10 },
  { "cr11", 11 },
  { "cr12", 12 },
  { "cr13", 13 },
  { "cr14", 14 },
  { "cr15", 15 }
};

CGEN_KEYWORD m32r_cgen_opval_h_cr = 
{
  & m32r_cgen_opval_h_cr_entries[0],
  23
};



/* The hardware table.  */

#define A(a) (1 << (CONCAT2 (CGEN_HW_,a) - CGEN_ATTR_BOOL_OFFSET))
#define HW_ENT(n) m32r_cgen_hw_table[n]

const CGEN_HW_ENTRY m32r_cgen_hw_table[] =
{
  { HW_H_PC, & HW_ENT (HW_H_PC + 1), "h-pc", CGEN_ASM_KEYWORD, (PTR) 0, { CGEN_HW_NBOOL_ATTRS, 0|A(PROFILE)|A(PC), { (1<<MACH_BASE) } } },
  { HW_H_MEMORY, & HW_ENT (HW_H_MEMORY + 1), "h-memory", CGEN_ASM_KEYWORD, (PTR) 0, { CGEN_HW_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } } },
  { HW_H_SINT, & HW_ENT (HW_H_SINT + 1), "h-sint", CGEN_ASM_KEYWORD, (PTR) 0, { CGEN_HW_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } } },
  { HW_H_UINT, & HW_ENT (HW_H_UINT + 1), "h-uint", CGEN_ASM_KEYWORD, (PTR) 0, { CGEN_HW_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } } },
  { HW_H_ADDR, & HW_ENT (HW_H_ADDR + 1), "h-addr", CGEN_ASM_KEYWORD, (PTR) 0, { CGEN_HW_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } } },
  { HW_H_IADDR, & HW_ENT (HW_H_IADDR + 1), "h-iaddr", CGEN_ASM_KEYWORD, (PTR) 0, { CGEN_HW_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } } },
  { HW_H_HI16, & HW_ENT (HW_H_HI16 + 1), "h-hi16", CGEN_ASM_KEYWORD, (PTR) 0, { CGEN_HW_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } } },
  { HW_H_SLO16, & HW_ENT (HW_H_SLO16 + 1), "h-slo16", CGEN_ASM_KEYWORD, (PTR) 0, { CGEN_HW_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } } },
  { HW_H_ULO16, & HW_ENT (HW_H_ULO16 + 1), "h-ulo16", CGEN_ASM_KEYWORD, (PTR) 0, { CGEN_HW_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } } },
  { HW_H_GR, & HW_ENT (HW_H_GR + 1), "h-gr", CGEN_ASM_KEYWORD, (PTR) & m32r_cgen_opval_h_gr, { CGEN_HW_NBOOL_ATTRS, 0|A(CACHE_ADDR)|A(PROFILE), { (1<<MACH_BASE) } } },
  { HW_H_CR, & HW_ENT (HW_H_CR + 1), "h-cr", CGEN_ASM_KEYWORD, (PTR) & m32r_cgen_opval_h_cr, { CGEN_HW_NBOOL_ATTRS, 0|A(FUN_ACCESS), { (1<<MACH_BASE) } } },
  { HW_H_ACCUM, & HW_ENT (HW_H_ACCUM + 1), "h-accum", CGEN_ASM_KEYWORD, (PTR) 0, { CGEN_HW_NBOOL_ATTRS, 0|A(FUN_ACCESS), { (1<<MACH_BASE) } } },
  { HW_H_COND, & HW_ENT (HW_H_COND + 1), "h-cond", CGEN_ASM_KEYWORD, (PTR) 0, { CGEN_HW_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } } },
  { HW_H_PSW, & HW_ENT (HW_H_PSW + 1), "h-psw", CGEN_ASM_KEYWORD, (PTR) 0, { CGEN_HW_NBOOL_ATTRS, 0|A(FUN_ACCESS), { (1<<MACH_BASE) } } },
  { HW_H_BPSW, & HW_ENT (HW_H_BPSW + 1), "h-bpsw", CGEN_ASM_KEYWORD, (PTR) 0, { CGEN_HW_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } } },
  { HW_H_BBPSW, & HW_ENT (HW_H_BBPSW + 1), "h-bbpsw", CGEN_ASM_KEYWORD, (PTR) 0, { CGEN_HW_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } } },
  { HW_H_LOCK, & HW_ENT (HW_H_LOCK + 1), "h-lock", CGEN_ASM_KEYWORD, (PTR) 0, { CGEN_HW_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } } },
  { 0 }
};

/* don't undef HW_ENT, used later */
#undef A

/* The instruction field table.  */

#define A(a) (1 << (CONCAT2 (CGEN_IFLD_,a) - CGEN_ATTR_BOOL_OFFSET))

const CGEN_IFLD m32r_cgen_ifld_table[] =
{
  { M32R_F_NIL, "f-nil", 0, 0, 0, 0, { CGEN_IFLD_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }  },
  { M32R_F_OP1, "f-op1", 0, 32, 0, 4, { CGEN_IFLD_NBOOL_ATTRS, 0|A(UNSIGNED), { (1<<MACH_BASE) } }  },
  { M32R_F_OP2, "f-op2", 0, 32, 8, 4, { CGEN_IFLD_NBOOL_ATTRS, 0|A(UNSIGNED), { (1<<MACH_BASE) } }  },
  { M32R_F_COND, "f-cond", 0, 32, 4, 4, { CGEN_IFLD_NBOOL_ATTRS, 0|A(UNSIGNED), { (1<<MACH_BASE) } }  },
  { M32R_F_R1, "f-r1", 0, 32, 4, 4, { CGEN_IFLD_NBOOL_ATTRS, 0|A(UNSIGNED), { (1<<MACH_BASE) } }  },
  { M32R_F_R2, "f-r2", 0, 32, 12, 4, { CGEN_IFLD_NBOOL_ATTRS, 0|A(UNSIGNED), { (1<<MACH_BASE) } }  },
  { M32R_F_SIMM8, "f-simm8", 0, 32, 8, 8, { CGEN_IFLD_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }  },
  { M32R_F_SIMM16, "f-simm16", 0, 32, 16, 16, { CGEN_IFLD_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }  },
  { M32R_F_SHIFT_OP2, "f-shift-op2", 0, 32, 8, 3, { CGEN_IFLD_NBOOL_ATTRS, 0|A(UNSIGNED), { (1<<MACH_BASE) } }  },
  { M32R_F_UIMM4, "f-uimm4", 0, 32, 12, 4, { CGEN_IFLD_NBOOL_ATTRS, 0|A(UNSIGNED), { (1<<MACH_BASE) } }  },
  { M32R_F_UIMM5, "f-uimm5", 0, 32, 11, 5, { CGEN_IFLD_NBOOL_ATTRS, 0|A(UNSIGNED), { (1<<MACH_BASE) } }  },
  { M32R_F_UIMM16, "f-uimm16", 0, 32, 16, 16, { CGEN_IFLD_NBOOL_ATTRS, 0|A(UNSIGNED), { (1<<MACH_BASE) } }  },
  { M32R_F_UIMM24, "f-uimm24", 0, 32, 8, 24, { CGEN_IFLD_NBOOL_ATTRS, 0|A(RELOC)|A(ABS_ADDR)|A(UNSIGNED), { (1<<MACH_BASE) } }  },
  { M32R_F_HI16, "f-hi16", 0, 32, 16, 16, { CGEN_IFLD_NBOOL_ATTRS, 0|A(SIGN_OPT)|A(UNSIGNED), { (1<<MACH_BASE) } }  },
  { M32R_F_DISP8, "f-disp8", 0, 32, 8, 8, { CGEN_IFLD_NBOOL_ATTRS, 0|A(RELOC)|A(PCREL_ADDR), { (1<<MACH_BASE) } }  },
  { M32R_F_DISP16, "f-disp16", 0, 32, 16, 16, { CGEN_IFLD_NBOOL_ATTRS, 0|A(RELOC)|A(PCREL_ADDR), { (1<<MACH_BASE) } }  },
  { M32R_F_DISP24, "f-disp24", 0, 32, 8, 24, { CGEN_IFLD_NBOOL_ATTRS, 0|A(RELOC)|A(PCREL_ADDR), { (1<<MACH_BASE) } }  },
  { 0 }
};

#undef A

/* The operand table.  */

#define A(a) (1 << (CONCAT2 (CGEN_OPERAND_,a) - CGEN_ATTR_BOOL_OFFSET))
#define OPERAND(op) CONCAT2 (M32R_OPERAND_,op)

const CGEN_OPERAND m32r_cgen_operand_table[MAX_OPERANDS] =
{
/* pc: program counter */
  { "pc", & HW_ENT (HW_H_PC), 0, 0,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(SEM_ONLY), { (1<<MACH_BASE) } }  },
/* sr: source register */
  { "sr", & HW_ENT (HW_H_GR), 12, 4,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(UNSIGNED), { (1<<MACH_BASE) } }  },
/* dr: destination register */
  { "dr", & HW_ENT (HW_H_GR), 4, 4,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(UNSIGNED), { (1<<MACH_BASE) } }  },
/* src1: source register 1 */
  { "src1", & HW_ENT (HW_H_GR), 4, 4,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(UNSIGNED), { (1<<MACH_BASE) } }  },
/* src2: source register 2 */
  { "src2", & HW_ENT (HW_H_GR), 12, 4,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(UNSIGNED), { (1<<MACH_BASE) } }  },
/* scr: source control register */
  { "scr", & HW_ENT (HW_H_CR), 12, 4,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(UNSIGNED), { (1<<MACH_BASE) } }  },
/* dcr: destination control register */
  { "dcr", & HW_ENT (HW_H_CR), 4, 4,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(UNSIGNED), { (1<<MACH_BASE) } }  },
/* simm8: 8 bit signed immediate */
  { "simm8", & HW_ENT (HW_H_SINT), 8, 8,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(HASH_PREFIX), { (1<<MACH_BASE) } }  },
/* simm16: 16 bit signed immediate */
  { "simm16", & HW_ENT (HW_H_SINT), 16, 16,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(HASH_PREFIX), { (1<<MACH_BASE) } }  },
/* uimm4: 4 bit trap number */
  { "uimm4", & HW_ENT (HW_H_UINT), 12, 4,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(HASH_PREFIX)|A(UNSIGNED), { (1<<MACH_BASE) } }  },
/* uimm5: 5 bit shift count */
  { "uimm5", & HW_ENT (HW_H_UINT), 11, 5,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(HASH_PREFIX)|A(UNSIGNED), { (1<<MACH_BASE) } }  },
/* uimm16: 16 bit unsigned immediate */
  { "uimm16", & HW_ENT (HW_H_UINT), 16, 16,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(HASH_PREFIX)|A(UNSIGNED), { (1<<MACH_BASE) } }  },
/* hash: # prefix */
  { "hash", & HW_ENT (HW_H_SINT), 0, 0,
    { CGEN_OPERAND_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }  },
/* hi16: high 16 bit immediate, sign optional */
  { "hi16", & HW_ENT (HW_H_HI16), 16, 16,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(SIGN_OPT)|A(UNSIGNED), { (1<<MACH_BASE) } }  },
/* slo16: 16 bit signed immediate, for low() */
  { "slo16", & HW_ENT (HW_H_SLO16), 16, 16,
    { CGEN_OPERAND_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }  },
/* ulo16: 16 bit unsigned immediate, for low() */
  { "ulo16", & HW_ENT (HW_H_ULO16), 16, 16,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(UNSIGNED), { (1<<MACH_BASE) } }  },
/* uimm24: 24 bit address */
  { "uimm24", & HW_ENT (HW_H_ADDR), 8, 24,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(HASH_PREFIX)|A(RELOC)|A(ABS_ADDR)|A(UNSIGNED), { (1<<MACH_BASE) } }  },
/* disp8: 8 bit displacement */
  { "disp8", & HW_ENT (HW_H_IADDR), 8, 8,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(RELAX)|A(RELOC)|A(PCREL_ADDR), { (1<<MACH_BASE) } }  },
/* disp16: 16 bit displacement */
  { "disp16", & HW_ENT (HW_H_IADDR), 16, 16,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(RELOC)|A(PCREL_ADDR), { (1<<MACH_BASE) } }  },
/* disp24: 24 bit displacement */
  { "disp24", & HW_ENT (HW_H_IADDR), 8, 24,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(RELAX)|A(RELOC)|A(PCREL_ADDR), { (1<<MACH_BASE) } }  },
/* condbit: condition bit */
  { "condbit", & HW_ENT (HW_H_COND), 0, 0,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(SEM_ONLY), { (1<<MACH_BASE) } }  },
/* accum: accumulator */
  { "accum", & HW_ENT (HW_H_ACCUM), 0, 0,
    { CGEN_OPERAND_NBOOL_ATTRS, 0|A(SEM_ONLY), { (1<<MACH_BASE) } }  },
};

#undef A

#define A(a) (1 << (CONCAT2 (CGEN_INSN_,a) - CGEN_ATTR_BOOL_OFFSET))
#define OP(field) CGEN_SYNTAX_MAKE_FIELD (OPERAND (field))

/* The instruction table.  */

static const CGEN_IBASE m32r_cgen_insn_table[MAX_INSNS] =
{
  /* Special null first entry.
     A `num' value of zero is thus invalid.
     Also, the special `invalid' insn resides here.  */
  { 0, 0, 0 },
/* add $dr,$sr */
  {
    M32R_INSN_ADD, "add", "add", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* add3 $dr,$sr,$hash$slo16 */
  {
    M32R_INSN_ADD3, "add3", "add3", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* and $dr,$sr */
  {
    M32R_INSN_AND, "and", "and", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* and3 $dr,$sr,$uimm16 */
  {
    M32R_INSN_AND3, "and3", "and3", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* or $dr,$sr */
  {
    M32R_INSN_OR, "or", "or", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* or3 $dr,$sr,$hash$ulo16 */
  {
    M32R_INSN_OR3, "or3", "or3", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* xor $dr,$sr */
  {
    M32R_INSN_XOR, "xor", "xor", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* xor3 $dr,$sr,$uimm16 */
  {
    M32R_INSN_XOR3, "xor3", "xor3", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* addi $dr,$simm8 */
  {
    M32R_INSN_ADDI, "addi", "addi", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* addv $dr,$sr */
  {
    M32R_INSN_ADDV, "addv", "addv", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* addv3 $dr,$sr,$simm16 */
  {
    M32R_INSN_ADDV3, "addv3", "addv3", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* addx $dr,$sr */
  {
    M32R_INSN_ADDX, "addx", "addx", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* bc.s $disp8 */
  {
    M32R_INSN_BC8, "bc8", "bc.s", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(COND_CTI), { (1<<MACH_BASE) } }
  },
/* bc.l $disp24 */
  {
    M32R_INSN_BC24, "bc24", "bc.l", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(COND_CTI), { (1<<MACH_BASE) } }
  },
/* beq $src1,$src2,$disp16 */
  {
    M32R_INSN_BEQ, "beq", "beq", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(COND_CTI), { (1<<MACH_BASE) } }
  },
/* beqz $src2,$disp16 */
  {
    M32R_INSN_BEQZ, "beqz", "beqz", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(COND_CTI), { (1<<MACH_BASE) } }
  },
/* bgez $src2,$disp16 */
  {
    M32R_INSN_BGEZ, "bgez", "bgez", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(COND_CTI), { (1<<MACH_BASE) } }
  },
/* bgtz $src2,$disp16 */
  {
    M32R_INSN_BGTZ, "bgtz", "bgtz", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(COND_CTI), { (1<<MACH_BASE) } }
  },
/* blez $src2,$disp16 */
  {
    M32R_INSN_BLEZ, "blez", "blez", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(COND_CTI), { (1<<MACH_BASE) } }
  },
/* bltz $src2,$disp16 */
  {
    M32R_INSN_BLTZ, "bltz", "bltz", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(COND_CTI), { (1<<MACH_BASE) } }
  },
/* bnez $src2,$disp16 */
  {
    M32R_INSN_BNEZ, "bnez", "bnez", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(COND_CTI), { (1<<MACH_BASE) } }
  },
/* bl.s $disp8 */
  {
    M32R_INSN_BL8, "bl8", "bl.s", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(FILL_SLOT)|A(UNCOND_CTI), { (1<<MACH_BASE) } }
  },
/* bl.l $disp24 */
  {
    M32R_INSN_BL24, "bl24", "bl.l", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(UNCOND_CTI), { (1<<MACH_BASE) } }
  },
/* bnc.s $disp8 */
  {
    M32R_INSN_BNC8, "bnc8", "bnc.s", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(COND_CTI), { (1<<MACH_BASE) } }
  },
/* bnc.l $disp24 */
  {
    M32R_INSN_BNC24, "bnc24", "bnc.l", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(COND_CTI), { (1<<MACH_BASE) } }
  },
/* bne $src1,$src2,$disp16 */
  {
    M32R_INSN_BNE, "bne", "bne", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(COND_CTI), { (1<<MACH_BASE) } }
  },
/* bra.s $disp8 */
  {
    M32R_INSN_BRA8, "bra8", "bra.s", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(FILL_SLOT)|A(UNCOND_CTI), { (1<<MACH_BASE) } }
  },
/* bra.l $disp24 */
  {
    M32R_INSN_BRA24, "bra24", "bra.l", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(UNCOND_CTI), { (1<<MACH_BASE) } }
  },
/* cmp $src1,$src2 */
  {
    M32R_INSN_CMP, "cmp", "cmp", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* cmpi $src2,$simm16 */
  {
    M32R_INSN_CMPI, "cmpi", "cmpi", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* cmpu $src1,$src2 */
  {
    M32R_INSN_CMPU, "cmpu", "cmpu", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* cmpui $src2,$simm16 */
  {
    M32R_INSN_CMPUI, "cmpui", "cmpui", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* div $dr,$sr */
  {
    M32R_INSN_DIV, "div", "div", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* divu $dr,$sr */
  {
    M32R_INSN_DIVU, "divu", "divu", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* rem $dr,$sr */
  {
    M32R_INSN_REM, "rem", "rem", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* remu $dr,$sr */
  {
    M32R_INSN_REMU, "remu", "remu", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* jl $sr */
  {
    M32R_INSN_JL, "jl", "jl", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(FILL_SLOT)|A(UNCOND_CTI), { (1<<MACH_BASE) } }
  },
/* jmp $sr */
  {
    M32R_INSN_JMP, "jmp", "jmp", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(UNCOND_CTI), { (1<<MACH_BASE) } }
  },
/* ld $dr,@$sr */
  {
    M32R_INSN_LD, "ld", "ld", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* ld $dr,@($slo16,$sr) */
  {
    M32R_INSN_LD_D, "ld-d", "ld", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* ldb $dr,@$sr */
  {
    M32R_INSN_LDB, "ldb", "ldb", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* ldb $dr,@($slo16,$sr) */
  {
    M32R_INSN_LDB_D, "ldb-d", "ldb", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* ldh $dr,@$sr */
  {
    M32R_INSN_LDH, "ldh", "ldh", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* ldh $dr,@($slo16,$sr) */
  {
    M32R_INSN_LDH_D, "ldh-d", "ldh", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* ldub $dr,@$sr */
  {
    M32R_INSN_LDUB, "ldub", "ldub", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* ldub $dr,@($slo16,$sr) */
  {
    M32R_INSN_LDUB_D, "ldub-d", "ldub", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* lduh $dr,@$sr */
  {
    M32R_INSN_LDUH, "lduh", "lduh", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* lduh $dr,@($slo16,$sr) */
  {
    M32R_INSN_LDUH_D, "lduh-d", "lduh", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* ld $dr,@$sr+ */
  {
    M32R_INSN_LD_PLUS, "ld-plus", "ld", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* ld24 $dr,$uimm24 */
  {
    M32R_INSN_LD24, "ld24", "ld24", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* ldi8 $dr,$simm8 */
  {
    M32R_INSN_LDI8, "ldi8", "ldi8", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* ldi16 $dr,$hash$slo16 */
  {
    M32R_INSN_LDI16, "ldi16", "ldi16", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* lock $dr,@$sr */
  {
    M32R_INSN_LOCK, "lock", "lock", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* machi $src1,$src2 */
  {
    M32R_INSN_MACHI, "machi", "machi", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_M32R) } }
  },
/* maclo $src1,$src2 */
  {
    M32R_INSN_MACLO, "maclo", "maclo", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_M32R) } }
  },
/* macwhi $src1,$src2 */
  {
    M32R_INSN_MACWHI, "macwhi", "macwhi", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_M32R) } }
  },
/* macwlo $src1,$src2 */
  {
    M32R_INSN_MACWLO, "macwlo", "macwlo", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_M32R) } }
  },
/* mul $dr,$sr */
  {
    M32R_INSN_MUL, "mul", "mul", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* mulhi $src1,$src2 */
  {
    M32R_INSN_MULHI, "mulhi", "mulhi", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_M32R) } }
  },
/* mullo $src1,$src2 */
  {
    M32R_INSN_MULLO, "mullo", "mullo", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_M32R) } }
  },
/* mulwhi $src1,$src2 */
  {
    M32R_INSN_MULWHI, "mulwhi", "mulwhi", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_M32R) } }
  },
/* mulwlo $src1,$src2 */
  {
    M32R_INSN_MULWLO, "mulwlo", "mulwlo", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_M32R) } }
  },
/* mv $dr,$sr */
  {
    M32R_INSN_MV, "mv", "mv", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* mvfachi $dr */
  {
    M32R_INSN_MVFACHI, "mvfachi", "mvfachi", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_M32R) } }
  },
/* mvfaclo $dr */
  {
    M32R_INSN_MVFACLO, "mvfaclo", "mvfaclo", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_M32R) } }
  },
/* mvfacmi $dr */
  {
    M32R_INSN_MVFACMI, "mvfacmi", "mvfacmi", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_M32R) } }
  },
/* mvfc $dr,$scr */
  {
    M32R_INSN_MVFC, "mvfc", "mvfc", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* mvtachi $src1 */
  {
    M32R_INSN_MVTACHI, "mvtachi", "mvtachi", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_M32R) } }
  },
/* mvtaclo $src1 */
  {
    M32R_INSN_MVTACLO, "mvtaclo", "mvtaclo", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_M32R) } }
  },
/* mvtc $sr,$dcr */
  {
    M32R_INSN_MVTC, "mvtc", "mvtc", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* neg $dr,$sr */
  {
    M32R_INSN_NEG, "neg", "neg", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* nop */
  {
    M32R_INSN_NOP, "nop", "nop", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* not $dr,$sr */
  {
    M32R_INSN_NOT, "not", "not", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* rac */
  {
    M32R_INSN_RAC, "rac", "rac", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_M32R) } }
  },
/* rach */
  {
    M32R_INSN_RACH, "rach", "rach", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_M32R) } }
  },
/* rte */
  {
    M32R_INSN_RTE, "rte", "rte", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(UNCOND_CTI), { (1<<MACH_BASE) } }
  },
/* seth $dr,$hash$hi16 */
  {
    M32R_INSN_SETH, "seth", "seth", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* sll $dr,$sr */
  {
    M32R_INSN_SLL, "sll", "sll", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* sll3 $dr,$sr,$simm16 */
  {
    M32R_INSN_SLL3, "sll3", "sll3", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* slli $dr,$uimm5 */
  {
    M32R_INSN_SLLI, "slli", "slli", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* sra $dr,$sr */
  {
    M32R_INSN_SRA, "sra", "sra", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* sra3 $dr,$sr,$simm16 */
  {
    M32R_INSN_SRA3, "sra3", "sra3", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* srai $dr,$uimm5 */
  {
    M32R_INSN_SRAI, "srai", "srai", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* srl $dr,$sr */
  {
    M32R_INSN_SRL, "srl", "srl", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* srl3 $dr,$sr,$simm16 */
  {
    M32R_INSN_SRL3, "srl3", "srl3", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* srli $dr,$uimm5 */
  {
    M32R_INSN_SRLI, "srli", "srli", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* st $src1,@$src2 */
  {
    M32R_INSN_ST, "st", "st", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* st $src1,@($slo16,$src2) */
  {
    M32R_INSN_ST_D, "st-d", "st", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* stb $src1,@$src2 */
  {
    M32R_INSN_STB, "stb", "stb", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* stb $src1,@($slo16,$src2) */
  {
    M32R_INSN_STB_D, "stb-d", "stb", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* sth $src1,@$src2 */
  {
    M32R_INSN_STH, "sth", "sth", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* sth $src1,@($slo16,$src2) */
  {
    M32R_INSN_STH_D, "sth-d", "sth", 32,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* st $src1,@+$src2 */
  {
    M32R_INSN_ST_PLUS, "st-plus", "st", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* st $src1,@-$src2 */
  {
    M32R_INSN_ST_MINUS, "st-minus", "st", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* sub $dr,$sr */
  {
    M32R_INSN_SUB, "sub", "sub", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* subv $dr,$sr */
  {
    M32R_INSN_SUBV, "subv", "subv", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* subx $dr,$sr */
  {
    M32R_INSN_SUBX, "subx", "subx", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
/* trap $uimm4 */
  {
    M32R_INSN_TRAP, "trap", "trap", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0|A(FILL_SLOT)|A(UNCOND_CTI), { (1<<MACH_BASE) } }
  },
/* unlock $src1,@$src2 */
  {
    M32R_INSN_UNLOCK, "unlock", "unlock", 16,
    { CGEN_INSN_NBOOL_ATTRS, 0, { (1<<MACH_BASE) } }
  },
};

#undef A
#undef MNEM
#undef OP

static void
init_tables ()
{
}

/* Initialize a cpu table and return a descriptor.
   It's much like opening a file, and must be the first function called.  */

CGEN_CPU_DESC
m32r_cgen_cpu_open (mach, endian)
     int mach;
     enum cgen_endian endian;
{
  CGEN_CPU_TABLE *cd = (CGEN_CPU_TABLE *) xmalloc (sizeof (CGEN_CPU_TABLE));
  static int init_p;

  if (! init_p)
    {
      init_tables ();
      init_p = 1;
    }

  memset (cd, 0, sizeof (*cd));

  cd->mach = mach;
  cd->endian = endian;
  /* FIXME: for the sparc case we can determine insn-endianness statically.
     The worry here is where both data and insn endian can be independently
     chosen, in which case this function will need another argument.
     Actually, will want to allow for more arguments in the future anyway.  */
  cd->insn_endian = endian;

  cd->int_insn_p = CGEN_INT_INSN_P;

  cd->max_insn_size = CGEN_MAX_INSN_SIZE;

  cd->hw_list = & m32r_cgen_hw_table[0];

  cd->ifld_table = & m32r_cgen_ifld_table[0];

  cd->operand_table = & m32r_cgen_operand_table[0];

  {
    int i;
    const CGEN_IBASE *ib = & m32r_cgen_insn_table[0];
    CGEN_INSN *insns = (CGEN_INSN *) xmalloc (MAX_INSNS * sizeof (CGEN_INSN));
    memset (insns, 0, MAX_INSNS * sizeof (CGEN_INSN));
    for (i = 0; i < MAX_INSNS; ++i)
      insns[i].base = &ib[i];
    cd->insn_table.init_entries = insns;
  }
  cd->insn_table.entry_size = sizeof (CGEN_IBASE);
  cd->insn_table.num_init_entries = MAX_INSNS;

  return (CGEN_CPU_DESC) cd;
}

/* Close a cpu table.  */

void
m32r_cgen_cpu_close (cd)
     CGEN_CPU_DESC cd;
{
  if (cd->insn_table.init_entries)
    free ((CGEN_INSN *) cd->insn_table.init_entries);
  free (cd);
}

