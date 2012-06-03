/* Print in infix form a struct expression.
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

#include <stdio.h>
#include "defs.h"
#include "symtab.h"
#include "param.h"
#include "expression.h"
#include "value.h"


/* These codes indicate operator precedences, least tightly binding first.  */
/* Adding 1 to a precedence value is done for binary operators,
   on the operand which is more tightly bound, so that operators
   of equal precedence within that operand will get parentheses.  */
/* PREC_HYPER and PREC_ABOVE_COMMA are not the precedence of any operator;
   they are used as the "surrounding precedence" to force
   various kinds of things to be parenthesized.  */
enum precedence
{ PREC_NULL, PREC_COMMA, PREC_ABOVE_COMMA, PREC_ASSIGN, PREC_OR, PREC_AND,
  PREC_LOGIOR, PREC_LOGAND, PREC_LOGXOR, PREC_EQUAL, PREC_ORDER,
  PREC_SHIFT, PREC_ADD, PREC_MUL, PREC_REPEAT,
  PREC_HYPER, PREC_PREFIX, PREC_SUFFIX };

/* Table mapping opcodes into strings for printing operators
   and precedences of the operators.  */

struct op_print
{
  char *string;
  enum exp_opcode opcode;
  /* Precedence of operator.  These values are used only by comparisons.  */
  enum precedence precedence;
  int right_assoc;
};

const static struct op_print op_print_tab[] =
  {
    {",", BINOP_COMMA, PREC_COMMA, 0},
    {"=", BINOP_ASSIGN, PREC_ASSIGN, 1},
    {"||", BINOP_OR, PREC_OR, 0},
    {"&&", BINOP_AND, PREC_AND, 0},
    {"|", BINOP_LOGIOR, PREC_LOGIOR, 0},
    {"&", BINOP_LOGAND, PREC_LOGAND, 0},
    {"^", BINOP_LOGXOR, PREC_LOGXOR, 0},
    {"==", BINOP_EQUAL, PREC_EQUAL, 0},
    {"!=", BINOP_NOTEQUAL, PREC_EQUAL, 0},
    {"<=", BINOP_LEQ, PREC_ORDER, 0},
    {">=", BINOP_GEQ, PREC_ORDER, 0},
    {">", BINOP_GTR, PREC_ORDER, 0},
    {"<", BINOP_LESS, PREC_ORDER, 0},
    {">>", BINOP_RSH, PREC_SHIFT, 0},
    {"<<", BINOP_LSH, PREC_SHIFT, 0},
    {"+", BINOP_ADD, PREC_ADD, 0},
    {"-", BINOP_SUB, PREC_ADD, 0},
    {"*", BINOP_MUL, PREC_MUL, 0},
    {"/", BINOP_DIV, PREC_MUL, 0},
    {"%", BINOP_REM, PREC_MUL, 0},
    {"@", BINOP_REPEAT, PREC_REPEAT, 0},
    {"-", UNOP_NEG, PREC_PREFIX, 0},
    {"!", UNOP_ZEROP, PREC_PREFIX, 0},
    {"~", UNOP_LOGNOT, PREC_PREFIX, 0},
    {"*", UNOP_IND, PREC_PREFIX, 0},
    {"&", UNOP_ADDR, PREC_PREFIX, 0},
    {"sizeof ", UNOP_SIZEOF, PREC_PREFIX, 0},
    {"++", UNOP_PREINCREMENT, PREC_PREFIX, 0},
    {"--", UNOP_PREDECREMENT, PREC_PREFIX, 0},
    /* C++  */
    {"::", BINOP_SCOPE, PREC_PREFIX, 0},
  };

static void print_subexp ();

void
print_expression (exp, stream)
     struct expression *exp;
     FILE *stream;
{
  int pc = 0;
  print_subexp (exp, &pc, stream, PREC_NULL);
}

/* Print the subexpression of EXP that starts in position POS, on STREAM.
   PREC is the precedence of the surrounding operator;
   if the precedence of the main operator of this subexpression is less,
   parentheses are needed here.  */

static void
print_subexp (exp, pos, stream, prec)
     register struct expression *exp;
     register int *pos;
     FILE *stream;
     enum precedence prec;
{
  register unsigned tem;
  register int pc;
  unsigned nargs;
  register char *op_str;
  int assign_modify = 0;
  enum exp_opcode opcode;
  enum precedence myprec;
  /* Set to 1 for a right-associative operator.  */
  int assoc;
  value val;

  pc = (*pos)++;
  opcode = exp->elts[pc].opcode;
  switch (opcode)
    {
    case OP_SCOPE:
      myprec = PREC_PREFIX;
      assoc = 0;
      (*pos) += 2;
      print_subexp (exp, pos, stream,
		    (enum precedence) ((int) myprec + assoc));
      fputs_filtered (" :: ", stream);
      nargs = strlen (&exp->elts[pc + 2].string);
      (*pos) += 1 + (nargs + sizeof (union exp_element)) / sizeof (union exp_element);

      fputs_filtered (&exp->elts[pc + 2].string, stream);
      return;

    case OP_LONG:
      (*pos) += 3;
      value_print (value_from_long (exp->elts[pc + 1].type,
				    exp->elts[pc + 2].longconst),
		   stream, 0, Val_no_prettyprint);
      return;

    case OP_DOUBLE:
      (*pos) += 3;
      value_print (value_from_double (exp->elts[pc + 1].type,
				      exp->elts[pc + 2].doubleconst),
		   stream, 0, Val_no_prettyprint);
      return;

    case OP_VAR_VALUE:
      (*pos) += 2;
      fputs_filtered (SYMBOL_NAME (exp->elts[pc + 1].symbol), stream);
      return;

    case OP_LAST:
      (*pos) += 2;
      fprintf_filtered (stream, "$%d",
			longest_to_int (exp->elts[pc + 1].longconst));
      return;

    case OP_REGISTER:
      (*pos) += 2;
      fprintf_filtered (stream, "$%s",
	       longest_to_int (reg_names[exp->elts[pc + 1].longconst]));
      return;

    case OP_INTERNALVAR:
      (*pos) += 2;
      fprintf_filtered (stream, "$%s",
	       internalvar_name (exp->elts[pc + 1].internalvar));
      return;

    case OP_FUNCALL:
      (*pos) += 2;
      nargs = longest_to_int (exp->elts[pc + 1].longconst);
      print_subexp (exp, pos, stream, PREC_SUFFIX);
      fputs_filtered (" (", stream);
      for (tem = 0; tem < nargs; tem++)
	{
	  if (tem != 0)
	    fputs_filtered (", ", stream);
	  print_subexp (exp, pos, stream, PREC_ABOVE_COMMA);
	}
      fputs_filtered (")", stream);
      return;

    case OP_STRING:
      nargs = strlen (&exp->elts[pc + 1].string);
      (*pos) += 2 + (nargs + sizeof (union exp_element)) / sizeof (union exp_element);
      fputs_filtered ("\"", stream);
      for (tem = 0; tem < nargs; tem++)
	printchar ((&exp->elts[pc + 1].string)[tem], stream, '"');
      fputs_filtered ("\"", stream);
      return;

    case TERNOP_COND:
      if ((int) prec > (int) PREC_COMMA)
	fputs_filtered ("(", stream);
      /* Print the subexpressions, forcing parentheses
	 around any binary operations within them.
	 This is more parentheses than are strictly necessary,
	 but it looks clearer.  */
      print_subexp (exp, pos, stream, PREC_HYPER);
      fputs_filtered (" ? ", stream);
      print_subexp (exp, pos, stream, PREC_HYPER);
      fputs_filtered (" : ", stream);
      print_subexp (exp, pos, stream, PREC_HYPER);
      if ((int) prec > (int) PREC_COMMA)
	fputs_filtered (")", stream);
      return;

    case STRUCTOP_STRUCT:
      tem = strlen (&exp->elts[pc + 1].string);
      (*pos) += 2 + (tem + sizeof (union exp_element)) / sizeof (union exp_element);
      print_subexp (exp, pos, stream, PREC_SUFFIX);
      fputs_filtered (".", stream);
      fputs_filtered (&exp->elts[pc + 1].string, stream);
      return;

    case STRUCTOP_PTR:
      tem = strlen (&exp->elts[pc + 1].string);
      (*pos) += 2 + (tem + sizeof (union exp_element)) / sizeof (union exp_element);
      print_subexp (exp, pos, stream, PREC_SUFFIX);
      fputs_filtered ("->", stream);
      fputs_filtered (&exp->elts[pc + 1].string, stream);
      return;

    case BINOP_SUBSCRIPT:
      print_subexp (exp, pos, stream, PREC_SUFFIX);
      fputs_filtered ("[", stream);
      print_subexp (exp, pos, stream, PREC_ABOVE_COMMA);
      fputs_filtered ("]", stream);
      return;

    case UNOP_POSTINCREMENT:
      print_subexp (exp, pos, stream, PREC_SUFFIX);
      fputs_filtered ("++", stream);
      return;

    case UNOP_POSTDECREMENT:
      print_subexp (exp, pos, stream, PREC_SUFFIX);
      fputs_filtered ("--", stream);
      return;

    case UNOP_CAST:
      (*pos) += 2;
      if ((int) prec > (int) PREC_PREFIX)
        fputs_filtered ("(", stream);
      fputs_filtered ("(", stream);
      type_print (exp->elts[pc + 1].type, "", stream, 0);
      fputs_filtered (") ", stream);
      print_subexp (exp, pos, stream, PREC_PREFIX);
      if ((int) prec > (int) PREC_PREFIX)
        fputs_filtered (")", stream);
      return;

    case UNOP_MEMVAL:
      (*pos) += 2;
      if ((int) prec > (int) PREC_PREFIX)
        fputs_filtered ("(", stream);
      if (exp->elts[pc + 1].type->code == TYPE_CODE_FUNC &&
	  exp->elts[pc + 3].opcode == OP_LONG) {
	/* We have a misc function vector fn, probably.  It's encoded
	   as a UNOP_MEMVAL (function-type) of an OP_LONG (int, address).
	   Swallow the OP_LONG (including both its opcodes); ignore
	   its type; print the value in the type of the MEMVAL.  */
	(*pos) += 4;
	val = value_at_lazy (exp->elts[pc + 1].type,
			     exp->elts[pc + 5].longconst);
	value_print (val, stream, 0, Val_no_prettyprint);
      } else {
	fputs_filtered ("{", stream);
	type_print (exp->elts[pc + 1].type, "", stream, 0);
	fputs_filtered ("} ", stream);
        print_subexp (exp, pos, stream, PREC_PREFIX);
      }
      if ((int) prec > (int) PREC_PREFIX)
        fputs_filtered (")", stream);
      return;

    case BINOP_ASSIGN_MODIFY:
      opcode = exp->elts[pc + 1].opcode;
      (*pos) += 2;
      myprec = PREC_ASSIGN;
      assoc = 1;
      assign_modify = 1;
      for (tem = 0; tem < sizeof op_print_tab / sizeof op_print_tab[0]; tem++)
	if (op_print_tab[tem].opcode == opcode)
	  {
	    op_str = op_print_tab[tem].string;
	    break;
	  }
      break;

    case OP_THIS:
      ++(*pos);
      fputs_filtered ("this", stream);
      return;

    default:
      for (tem = 0; tem < sizeof op_print_tab / sizeof op_print_tab[0]; tem++)
	if (op_print_tab[tem].opcode == opcode)
	  {
	    op_str = op_print_tab[tem].string;
	    myprec = op_print_tab[tem].precedence;
	    assoc = op_print_tab[tem].right_assoc;
	    break;
	  }
    }

  if ((int) myprec < (int) prec)
    fputs_filtered ("(", stream);
  if ((int) opcode > (int) BINOP_END)
    {
      /* Unary prefix operator.  */
      fputs_filtered (op_str, stream);
      print_subexp (exp, pos, stream, PREC_PREFIX);
    }
  else
    {
      /* Binary operator.  */
      /* Print left operand.
	 If operator is right-associative,
	 increment precedence for this operand.  */
      print_subexp (exp, pos, stream,
		    (enum precedence) ((int) myprec + assoc));
      /* Print the operator itself.  */
      if (assign_modify)
	fprintf_filtered (stream, " %s= ", op_str);
      else if (op_str[0] == ',')
	fprintf_filtered (stream, "%s ", op_str);
      else
	fprintf_filtered (stream, " %s ", op_str);
      /* Print right operand.
	 If operator is left-associative,
	 increment precedence for this operand.  */
      print_subexp (exp, pos, stream,
		    (enum precedence) ((int) myprec + !assoc));
    }
  if ((int) myprec < (int) prec)
    fputs_filtered (")", stream);
}
