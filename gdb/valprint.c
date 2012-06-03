/* Print values for GDB, the GNU debugger.
   Copyright 1986, 1988, 1989, 1991 Free Software Foundation, Inc.

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
#include <string.h>
#include "symtab.h"
#include "gdbtypes.h"
#include "value.h"
#include "gdbcore.h"
#include "gdbcmd.h"
#include "target.h"
#include "obstack.h"
#include "language.h"
#include "demangle.h"

#include <errno.h>

/* Prototypes for local functions */

static void
print_hex_chars PARAMS ((FILE *, unsigned char *, unsigned int));

static void
show_print PARAMS ((char *, int));

static void
set_print PARAMS ((char *, int));

static void
set_radix PARAMS ((char *, int, struct cmd_list_element *));

static void
set_output_radix PARAMS ((char *, int, struct cmd_list_element *));

static void
value_print_array_elements PARAMS ((value, FILE *, int, enum val_prettyprint));

/* Maximum number of chars to print for a string pointer value
   or vector contents, or UINT_MAX for no limit.  */

unsigned int print_max;

/* Default input and output radixes, and output format letter.  */

unsigned input_radix = 10;
unsigned output_radix = 10;
int output_format = 0;

/* Print repeat counts if there are more than this many repetitions of an
   element in an array.  Referenced by the low level language dependent
   print routines. */

unsigned int repeat_count_threshold = 10;

int prettyprint_structs;	/* Controls pretty printing of structures */
int prettyprint_arrays;		/* Controls pretty printing of arrays.  */

/* If nonzero, causes unions inside structures or other unions to be
   printed. */

int unionprint;			/* Controls printing of nested unions.  */

/* If nonzero, causes machine addresses to be printed in certain contexts. */

int addressprint;		/* Controls printing of machine addresses */


/* Print data of type TYPE located at VALADDR (within GDB), which came from
   the inferior at address ADDRESS, onto stdio stream STREAM according to
   FORMAT (a letter, or 0 for natural format using TYPE).

   If DEREF_REF is nonzero, then dereference references, otherwise just print
   them like pointers.

   The PRETTY parameter controls prettyprinting.

   If the data are a string pointer, returns the number of string characters
   printed.

   FIXME:  The data at VALADDR is in target byte order.  If gdb is ever
   enhanced to be able to debug more than the single target it was compiled
   for (specific CPU type and thus specific target byte ordering), then
   either the print routines are going to have to take this into account,
   or the data is going to have to be passed into here already converted
   to the host byte ordering, whichever is more convenient. */


int
val_print (type, valaddr, address, stream, format, deref_ref, recurse, pretty)
     struct type *type;
     char *valaddr;
     CORE_ADDR address;
     FILE *stream;
     int format;
     int deref_ref;
     int recurse;
     enum val_prettyprint pretty;
{
  if (pretty == Val_pretty_default)
    {
      pretty = prettyprint_structs ? Val_prettyprint : Val_no_prettyprint;
    }
  
  QUIT;

  /* Ensure that the type is complete and not just a stub.  If the type is
     only a stub and we can't find and substitute its complete type, then
     print appropriate string and return.  Typical types that my be stubs
     are structs, unions, and C++ methods. */

  check_stub_type (type);
  if (TYPE_FLAGS (type) & TYPE_FLAG_STUB)
    {
      fprintf_filtered (stream, "<incomplete type>");
      fflush (stream);
      return (0);
    }
  
  return (LA_VAL_PRINT (type, valaddr, address, stream, format, deref_ref,
			recurse, pretty));
}

/* Print the value VAL in C-ish syntax on stream STREAM.
   FORMAT is a format-letter, or 0 for print in natural format of data type.
   If the object printed is a string pointer, returns
   the number of string bytes printed.  */

int
value_print (val, stream, format, pretty)
     value val;
     FILE *stream;
     int format;
     enum val_prettyprint pretty;
{
  register unsigned int n, typelen;

  if (val == 0)
    {
      printf_filtered ("<address of value unknown>");
      return 0;
    }
  if (VALUE_OPTIMIZED_OUT (val))
    {
      printf_filtered ("<value optimized out>");
      return 0;
    }

  /* A "repeated" value really contains several values in a row.
     They are made by the @ operator.
     Print such values as if they were arrays.  */

  if (VALUE_REPEATED (val))
    {
      n = VALUE_REPETITIONS (val);
      typelen = TYPE_LENGTH (VALUE_TYPE (val));
      fprintf_filtered (stream, "{");
      /* Print arrays of characters using string syntax.  */
      if (typelen == 1 && TYPE_CODE (VALUE_TYPE (val)) == TYPE_CODE_INT
	  && format == 0)
	LA_PRINT_STRING (stream, VALUE_CONTENTS (val), n, 0);
      else
	{
	  value_print_array_elements (val, stream, format, pretty);
	}
      fprintf_filtered (stream, "}");
      return (n * typelen);
    }
  else
    {
      struct type *type = VALUE_TYPE (val);

      /* If it is a pointer, indicate what it points to.

	 Print type also if it is a reference.

         C++: if it is a member pointer, we will take care
	 of that when we print it.  */
      if (TYPE_CODE (type) == TYPE_CODE_PTR ||
	  TYPE_CODE (type) == TYPE_CODE_REF)
	{
	  /* Hack:  remove (char *) for char strings.  Their
	     type is indicated by the quoted string anyway. */
          if (TYPE_CODE (type) == TYPE_CODE_PTR &&
	      TYPE_LENGTH (TYPE_TARGET_TYPE (type)) == sizeof(char) &&
	      TYPE_CODE (TYPE_TARGET_TYPE (type)) == TYPE_CODE_INT &&
	      !TYPE_UNSIGNED (TYPE_TARGET_TYPE (type)))
	    {
		/* Print nothing */
	    }
	  else
	    {
	      fprintf_filtered (stream, "(");
	      type_print (type, "", stream, -1);
	      fprintf_filtered (stream, ") ");
	    }
	}
      return (val_print (type, VALUE_CONTENTS (val),
			 VALUE_ADDRESS (val), stream, format, 1, 0, pretty));
    }
}

/*  Called by various <lang>_val_print routines to print TYPE_CODE_INT's */

void
val_print_type_code_int (type, valaddr, stream)
     struct type *type;
     char *valaddr;
     FILE *stream;
{
  char *p;
  /* Pointer to first (i.e. lowest address) nonzero character.  */
  char *first_addr;
  unsigned int len;

  if (TYPE_LENGTH (type) > sizeof (LONGEST))
    {
      if (TYPE_UNSIGNED (type))
	{
	  /* First figure out whether the number in fact has zeros
	     in all its bytes more significant than least significant
	     sizeof (LONGEST) ones.  */
	  len = TYPE_LENGTH (type);
	  
#if TARGET_BYTE_ORDER == BIG_ENDIAN
	  for (p = valaddr;
	       len > sizeof (LONGEST) && p < valaddr + TYPE_LENGTH (type);
	       p++)
#else		/* Little endian.  */
	  first_addr = valaddr;
	  for (p = valaddr + TYPE_LENGTH (type);
	       len > sizeof (LONGEST) && p >= valaddr;
	       p--)
#endif		/* Little endian.  */
	    {
	      if (*p == 0)
		{
		  len--;
		}
	      else
		{
		  break;
		}
	    }
#if TARGET_BYTE_ORDER == BIG_ENDIAN
	  first_addr = p;
#endif
	  if (len <= sizeof (LONGEST))
	    {
	      /* We can print it in decimal.  */
	      fprintf_filtered
		(stream, 
#if defined (LONG_LONG)
		 "%llu",
#else
		 "%lu",
#endif
		 unpack_long (BUILTIN_TYPE_LONGEST, first_addr));
	    }
	  else
	    {
	      /* It is big, so print it in hex.  */
	      print_hex_chars (stream, (unsigned char *) first_addr, len);
	    }
	}
      else
	{
	  /* Signed.  One could assume two's complement (a reasonable
	     assumption, I think) and do better than this.  */
	  print_hex_chars (stream, (unsigned char *) valaddr,
			   TYPE_LENGTH (type));
	}
    }
  else
    {
#ifdef PRINT_TYPELESS_INTEGER
      PRINT_TYPELESS_INTEGER (stream, type, unpack_long (type, valaddr));
#else
      fprintf_filtered (stream, TYPE_UNSIGNED (type) ?
#if defined (LONG_LONG)
			"%llu" : "%lld",
#else
			"%u" : "%d",
#endif
			unpack_long (type, valaddr));
#endif
    }
}			

/* Print a floating point value of type TYPE, pointed to in GDB by VALADDR,
   on STREAM.  */

void
print_floating (valaddr, type, stream)
     char *valaddr;
     struct type *type;
     FILE *stream;
{
  double doub;
  int inv;
  unsigned len = TYPE_LENGTH (type);
  
#if defined (IEEE_FLOAT)

  /* Check for NaN's.  Note that this code does not depend on us being
     on an IEEE conforming system.  It only depends on the target
     machine using IEEE representation.  This means (a)
     cross-debugging works right, and (2) IEEE_FLOAT can (and should)
     be defined for systems like the 68881, which uses IEEE
     representation, but is not IEEE conforming.  */

  {
    long low, high;
    /* Is the sign bit 0?  */
    int nonnegative;
    /* Is it is a NaN (i.e. the exponent is all ones and
       the fraction is nonzero)?  */
    int is_nan;

    if (len == sizeof (float))
      {
	/* It's single precision. */
	memcpy ((char *) &low, valaddr, sizeof (low));
	/* target -> host.  */
	SWAP_TARGET_AND_HOST (&low, sizeof (float));
	nonnegative = low >= 0;
	is_nan = ((((low >> 23) & 0xFF) == 0xFF) 
		  && 0 != (low & 0x7FFFFF));
	low &= 0x7fffff;
	high = 0;
      }
    else
      {
	/* It's double precision.  Get the high and low words.  */

#if TARGET_BYTE_ORDER == BIG_ENDIAN
	memcpy (&low, valaddr+4,  sizeof (low));
	memcpy (&high, valaddr+0, sizeof (high));
#else
	memcpy (&low, valaddr+0,  sizeof (low));
	memcpy (&high, valaddr+4, sizeof (high));
#endif
	SWAP_TARGET_AND_HOST (&low, sizeof (low));
	SWAP_TARGET_AND_HOST (&high, sizeof (high));
	nonnegative = high >= 0;
	is_nan = (((high >> 20) & 0x7ff) == 0x7ff
		  && ! ((((high & 0xfffff) == 0)) && (low == 0)));
	high &= 0xfffff;
      }

    if (is_nan)
      {
	/* The meaning of the sign and fraction is not defined by IEEE.
	   But the user might know what they mean.  For example, they
	   (in an implementation-defined manner) distinguish between
	   signaling and quiet NaN's.  */
	if (high)
	  fprintf_filtered (stream, "-NaN(0x%lx%.8lx)" + nonnegative,
			    high, low);
	else
	  fprintf_filtered (stream, "-NaN(0x%lx)" + nonnegative, low);
	return;
      }
  }
#endif /* IEEE_FLOAT.  */

  doub = unpack_double (type, valaddr, &inv);
  if (inv)
    fprintf_filtered (stream, "<invalid float value>");
  else
    fprintf_filtered (stream, len <= sizeof(float) ? "%.9g" : "%.17g", doub);
}

/* VALADDR points to an integer of LEN bytes.  Print it in hex on stream.  */

static void
print_hex_chars (stream, valaddr, len)
     FILE *stream;
     unsigned char *valaddr;
     unsigned len;
{
  unsigned char *p;
  
  fprintf_filtered (stream, "0x");
#if TARGET_BYTE_ORDER == BIG_ENDIAN
  for (p = valaddr;
       p < valaddr + len;
       p++)
#else /* Little endian.  */
  for (p = valaddr + len - 1;
       p >= valaddr;
       p--)
#endif
    {
      fprintf_filtered (stream, "%02x", *p);
    }
}

/*  Called by various <lang>_val_print routines to print elements of an
    array in the form "<elem1>, <elem2>, <elem3>, ...".

    (FIXME?)  Assumes array element separator is a comma, which is correct
    for all languages currently handled.
    (FIXME?)  Some languages have a notation for repeated array elements,
    perhaps we should try to use that notation when appropriate.
    */

void
val_print_array_elements (type, valaddr, address, stream, format, deref_ref,
			  recurse, pretty, i)
     struct type *type;
     char *valaddr;
     CORE_ADDR address;
     FILE *stream;
     int format;
     int deref_ref;
     int recurse;
     enum val_prettyprint pretty;
     unsigned int i;
{
  unsigned int things_printed = 0;
  unsigned len;
  struct type *elttype;
  unsigned eltlen;
  /* Position of the array element we are examining to see
     whether it is repeated.  */
  unsigned int rep1;
  /* Number of repetitions we have detected so far.  */
  unsigned int reps;
      
  elttype = TYPE_TARGET_TYPE (type);
  eltlen = TYPE_LENGTH (elttype);
  len = TYPE_LENGTH (type) / eltlen;
	      
  for (; i < len && things_printed < print_max; i++)
    {
      if (i != 0)
	{
	  if (prettyprint_arrays)
	    {
	      fprintf_filtered (stream, ",\n");
	      print_spaces_filtered (2 + 2 * recurse, stream);
	    }
	  else
	    {
	      fprintf_filtered (stream, ", ");
	    }
	}
      wrap_here (n_spaces (2 + 2 * recurse));
      
      rep1 = i + 1;
      reps = 1;
      while ((rep1 < len) && 
	     !memcmp (valaddr + i * eltlen, valaddr + rep1 * eltlen, eltlen))
	{
	  ++reps;
	  ++rep1;
	}
      
      if (reps > repeat_count_threshold)
	{
	  val_print (elttype, valaddr + i * eltlen, 0, stream, format,
		     deref_ref, recurse + 1, pretty);
	  fprintf_filtered (stream, " <repeats %u times>", reps);
	  i = rep1 - 1;
	  things_printed += repeat_count_threshold;
	}
      else
	{
	  val_print (elttype, valaddr + i * eltlen, 0, stream, format,
		     deref_ref, recurse + 1, pretty);
	  things_printed++;
	}
    }
  if (i < len)
    {
      fprintf_filtered (stream, "...");
    }
}

static void
value_print_array_elements (val, stream, format, pretty)
     value val;
     FILE *stream;
     int format;
     enum val_prettyprint pretty;
{
  unsigned int things_printed = 0;
  register unsigned int i, n, typelen;
  /* Position of the array elem we are examining to see if it is repeated.  */
  unsigned int rep1;
  /* Number of repetitions we have detected so far.  */
  unsigned int reps;
    
  n = VALUE_REPETITIONS (val);
  typelen = TYPE_LENGTH (VALUE_TYPE (val));
  for (i = 0; i < n && things_printed < print_max; i++)
    {
      if (i != 0)
	{
	  fprintf_filtered (stream, ", ");
	}
      wrap_here ("");
      
      rep1 = i + 1;
      reps = 1;
      while (rep1 < n && !memcmp (VALUE_CONTENTS (val) + typelen * i,
				  VALUE_CONTENTS (val) + typelen * rep1,
				  typelen))
	{
	  ++reps;
	  ++rep1;
	}
      
      if (reps > repeat_count_threshold)
	{
	  val_print (VALUE_TYPE (val), VALUE_CONTENTS (val) + typelen * i,
		     VALUE_ADDRESS (val) + typelen * i, stream, format, 1,
		     0, pretty);
	  fprintf (stream, " <repeats %u times>", reps);
	  i = rep1 - 1;
	  things_printed += repeat_count_threshold;
	}
      else
	{
	  val_print (VALUE_TYPE (val), VALUE_CONTENTS (val) + typelen * i,
		     VALUE_ADDRESS (val) + typelen * i, stream, format, 1,
		     0, pretty);
	  things_printed++;
	}
    }
  if (i < n)
    {
      fprintf_filtered (stream, "...");
    }
}

/*  Print a string from the inferior, starting at ADDR and printing up to LEN
    characters, to STREAM.  If LEN is zero, printing stops at the first null
    byte, otherwise printing proceeds (including null bytes) until either
    print_max or LEN characters have been printed.

    Always fetch print_max+1 characters, even though LA_PRINT_STRING might want
    to print more or fewer (with repeated characters).  This is so that we
    don't spend forever fetching if we print a long string consisting of the
    same character repeated.  Also so we can do it all in one memory operation,
    which is faster.  However, this will be slower if print_max is set high,
    e.g. if you set print_max to 1000, not only will it take a long time to
    fetch short strings, but if you are near the end of the address space, it
    might not work.

    If the number of characters we actually print is limited because of hitting
    print_max, when LEN would have explicitly or implicitly (in the case of a
    null terminated string with another non-null character available to print)
    allowed us to print more, we print ellipsis ("...") after the printed string
    to indicate that more characters were available to print but that we were
    limited by print_max.  To do this correctly requires that we always fetch
    one more than the number of characters we could potentially print, so that
    if we do print the maximum number, we can tell whether or not a null byte
    would have been the next character, in the case of C style strings.
    For non-C style strings, only the value of LEN is pertinent in deciding
    whether or not to print ellipsis.

    FIXME:  If LEN is nonzero and less than print_max, we could get away
    with only fetching the specified number of characters from the inferior. */

int
val_print_string (addr, len, stream)
    CORE_ADDR addr;
    unsigned int len;
    FILE *stream;
{
  int first_addr_err = 0;	/* Nonzero if first address out of bounds */
  int force_ellipsis = 0;	/* Force ellipsis to be printed if nonzero */
  int errcode;
  unsigned char c;
  char *string;

  /* Get first character.  */
  errcode = target_read_memory (addr, (char *)&c, 1);
  if (errcode != 0)
    {
      /* First address out of bounds.  */
      first_addr_err = 1;
    }
  else if (print_max < UINT_MAX)
    {
      string = (char *) alloca (print_max + 1);
      memset (string, 0, print_max + 1);
      
      QUIT;
      errcode = target_read_memory (addr, string, print_max + 1);
      if (errcode != 0)
	{
	  /* Try reading just one character.  If that succeeds, assume we hit
	     the end of the address space, but the initial part of the string
	     is probably safe. */
	  char x[1];
	  errcode = target_read_memory (addr, x, 1);
	}
      if (len == 0)
	{
	  /* When the length is unspecified, such as when printing C style
	     null byte terminated strings, then scan the string looking for
	     the terminator in the first print_max characters.  If a terminator
	     is found, then it determines the length, otherwise print_max
	     determines the length. */
	  for (;len < print_max; len++)
	    {
	      if (string[len] == '\0')
		{
		  break;
		}
	    }
	  /* If the first unprinted character is not the null terminator, set
	     the flag to force ellipses.  This is true whether or not we broke
	     out of the above loop because we found a terminator, or whether
	     we simply hit the limit on how many characters to print. */
	  if (string[len] != '\0')
	    {
	      force_ellipsis = 1;
	    }
	}
      else if (len > print_max)
	{
	  /* Printing less than the number of characters actually requested
	     always makes us print ellipsis. */
	  len = print_max;
	  force_ellipsis = 1;
	}
      QUIT;
      
      if (addressprint)
	{
	  fputs_filtered (" ", stream);
	}
      LA_PRINT_STRING (stream, string, len, force_ellipsis);
    }
  
  if (errcode != 0)
    {
      if (errcode == EIO)
	{
	  fprintf_filtered (stream,
			    (" <Address 0x%x out of bounds>" + first_addr_err),
			    addr + len);
	}
      else
	{
	  error ("Error reading memory address 0x%x: %s.", addr + len,
		 safe_strerror (errcode));
	}
    }
  fflush (stream);
  return (len);
}

#if 0
/* Validate an input or output radix setting, and make sure the user
   knows what they really did here.  Radix setting is confusing, e.g.
   setting the input radix to "10" never changes it!  */

/* ARGSUSED */
static void
set_input_radix (args, from_tty, c)
     char *args;
     int from_tty;
     struct cmd_list_element *c;
{
  unsigned radix = *(unsigned *)c->var;

  if (from_tty)
    printf_filtered ("Input radix set to decimal %d, hex %x, octal %o\n",
	radix, radix, radix);
}
#endif

/* ARGSUSED */
static void
set_output_radix (args, from_tty, c)
     char *args;
     int from_tty;
     struct cmd_list_element *c;
{
  unsigned radix = *(unsigned *)c->var;

  if (from_tty)
    printf_filtered ("Output radix set to decimal %d, hex %x, octal %o\n",
	radix, radix, radix);

  /* FIXME, we really should be able to validate the setting BEFORE
     it takes effect.  */
  switch (radix)
    {
    case 16:
      output_format = 'x';
      break;
    case 10:
      output_format = 0;
      break;
    case 8:
      output_format = 'o';		/* octal */
      break;
    default:
      output_format = 0;
      error ("Unsupported radix ``decimal %d''; using decimal output",
	      radix);
    }
}

/* Both at once */
static void
set_radix (arg, from_tty, c)
     char *arg;
     int from_tty;
     struct cmd_list_element *c;
{
  unsigned radix = *(unsigned *)c->var;

  if (from_tty)
    printf_filtered ("Radix set to decimal %d, hex %x, octal %o\n",
	radix, radix, radix);

  input_radix = radix;
  output_radix = radix;

  set_output_radix (arg, 0, c);
}

/*ARGSUSED*/
static void
set_print (arg, from_tty)
     char *arg;
     int from_tty;
{
  printf (
"\"set print\" must be followed by the name of a print subcommand.\n");
  help_list (setprintlist, "set print ", -1, stdout);
}

/*ARGSUSED*/
static void
show_print (args, from_tty)
     char *args;
     int from_tty;
{
  cmd_show_list (showprintlist, from_tty, "");
}

void
_initialize_valprint ()
{
  struct cmd_list_element *c;

  add_prefix_cmd ("print", no_class, set_print,
		  "Generic command for setting how things print.",
		  &setprintlist, "set print ", 0, &setlist);
  add_alias_cmd ("p", "print", no_class, 1, &setlist); 
  add_alias_cmd ("pr", "print", no_class, 1, &setlist); /* prefer set print
														   to     set prompt */
  add_prefix_cmd ("print", no_class, show_print,
		  "Generic command for showing print settings.",
		  &showprintlist, "show print ", 0, &showlist);
  add_alias_cmd ("p", "print", no_class, 1, &showlist); 
  add_alias_cmd ("pr", "print", no_class, 1, &showlist); 

  add_show_from_set
    (add_set_cmd ("elements", no_class, var_uinteger, (char *)&print_max,
		  "Set limit on string chars or array elements to print.\n\
\"set print elements 0\" causes there to be no limit.",
		  &setprintlist),
     &showprintlist);

  add_show_from_set
    (add_set_cmd ("repeats", no_class, var_uinteger,
		  (char *)&repeat_count_threshold,
		  "Set threshold for repeated print elements.\n\
\"set print repeats 0\" causes all elements to be individually printed.",
		  &setprintlist),
     &showprintlist);

  add_show_from_set
    (add_set_cmd ("pretty", class_support, var_boolean,
		  (char *)&prettyprint_structs,
		  "Set prettyprinting of structures.",
		  &setprintlist),
     &showprintlist);

  add_show_from_set
    (add_set_cmd ("union", class_support, var_boolean, (char *)&unionprint,
		  "Set printing of unions interior to structures.",
		  &setprintlist),
     &showprintlist);
  
  add_show_from_set
    (add_set_cmd ("array", class_support, var_boolean,
		  (char *)&prettyprint_arrays,
		  "Set prettyprinting of arrays.",
		  &setprintlist),
     &showprintlist);

  add_show_from_set
    (add_set_cmd ("address", class_support, var_boolean, (char *)&addressprint,
		  "Set printing of addresses.",
		  &setprintlist),
     &showprintlist);

#if 0
  /* The "show radix" cmd isn't good enough to show two separate values.
     The rest of the code works, but the show part is confusing, so don't
     let them be set separately 'til we work out "show".  */
  c = add_set_cmd ("input-radix", class_support, var_uinteger,
		   (char *)&input_radix,
		  "Set default input radix for entering numbers.",
		  &setlist);
  add_show_from_set (c, &showlist);
  c->function = set_input_radix;

  c = add_set_cmd ("output-radix", class_support, var_uinteger,
		   (char *)&output_radix,
		  "Set default output radix for printing of values.",
		  &setlist);
  add_show_from_set (c, &showlist);
  c->function = set_output_radix;
#endif 

  c = add_set_cmd ("radix", class_support, var_uinteger,
		   (char *)&output_radix,
		  "Set default input and output number radix.",
		  &setlist);
  add_show_from_set (c, &showlist);
  c->function.sfunc = set_radix;

  /* Give people the defaults which they are used to.  */
  prettyprint_structs = 0;
  prettyprint_arrays = 0;
  unionprint = 1;
  addressprint = 1;
  print_max = 200;
}
