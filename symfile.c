/* Generic symbol file reading for the GNU debugger, GDB.
   Copyright (C)  1990 Free Software Foundation, Inc.

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

#include <stdio.h>
#include "defs.h"
#include "symtab.h"
#include "param.h"
#include "gdbcore.h"
#include "frame.h"
#include "target.h"
#include "value.h"
#include "symfile.h"
#include "gdbcmd.h"
#include "breakpoint.h"

#include <obstack.h>
#include <assert.h>

#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

extern int info_verbose;

extern int close ();
extern void qsort ();
extern char *getenv ();

/* Functions this file defines */
static bfd *symfile_open();
static struct sym_fns *symfile_init();

extern void dbx_new_init(), dbx_symfile_init(), dbx_symfile_read();
extern void dbx_symfile_discard();

extern void coff_new_init(), coff_symfile_init(), coff_symfile_read();
extern void coff_symfile_discard();

struct sym_fns symtab_fns[] = {
  {"sunOs", 6,
		dbx_new_init, dbx_symfile_init,
		dbx_symfile_read, dbx_symfile_discard},
  {"a.out", 5,
		dbx_new_init, dbx_symfile_init,
		dbx_symfile_read, dbx_symfile_discard},
  {"b.out", 5,
		dbx_new_init, dbx_symfile_init,
		dbx_symfile_read, dbx_symfile_discard},
  {"coff", 4,
		coff_new_init, coff_symfile_init,
		coff_symfile_read, coff_symfile_discard},
  {0},
};

/* Saves the sym_fns of the current symbol table, so we can call
   the right sym_discard function when we free it.  */

static struct sym_fns *symfile_fns;

/* Allocate an obstack to hold objects that should be freed
   when we load a new symbol table.
   This includes the symbols made by dbxread
   and the types that are not permanent.  */

struct obstack obstack1;

struct obstack *symbol_obstack = &obstack1;

/* This obstack will be used for partial_symbol objects.  It can
   probably actually be the same as the symbol_obstack above, but I'd
   like to keep them seperate for now.  If I want to later, I'll
   replace one with the other.  */

struct obstack obstack2;

struct obstack *psymbol_obstack = &obstack2;

/* File name symbols were loaded from.  */

char *symfile = 0;

/* The modification date of the file when they were loaded.  */

int symfile_mtime = 0;

/* Structures with which to manage partial symbol allocation.  */

struct psymbol_allocation_list global_psymbols = {0}, static_psymbols = {0};

/* Structure to manage complaints about symbol file contents.  */

struct complaint complaint_root[1] = {
  {(char *)0, 0, complaint_root},
};


/* In the following sort, we always make sure that
   register debug symbol declarations always come before regular
   debug symbol declarations (as might happen when parameters are
   then put into registers by the compiler).  */

static int
compare_symbols (s1, s2)
     struct symbol **s1, **s2;
{
  register int namediff;

  /* Compare the initial characters.  */
  namediff = SYMBOL_NAME (*s1)[0] - SYMBOL_NAME (*s2)[0];
  if (namediff != 0) return namediff;

  /* If they match, compare the rest of the names.  */
  namediff = strcmp (SYMBOL_NAME (*s1), SYMBOL_NAME (*s2));
  if (namediff != 0) return namediff;

  /* For symbols of the same name, registers should come first.  */
  return ((SYMBOL_CLASS (*s2) == LOC_REGISTER)
	  - (SYMBOL_CLASS (*s1) == LOC_REGISTER));
}

/* Call sort_block_syms to sort alphabetically the symbols of one block.  */

void
sort_block_syms (b)
     register struct block *b;
{
  qsort (&BLOCK_SYM (b, 0), BLOCK_NSYMS (b),
	 sizeof (struct symbol *), compare_symbols);
}

/* Call sort_symtab_syms to sort alphabetically
   the symbols of each block of one symtab.  */

void
sort_symtab_syms (s)
     register struct symtab *s;
{
  register struct blockvector *bv = BLOCKVECTOR (s);
  int nbl = BLOCKVECTOR_NBLOCKS (bv);
  int i;
  register struct block *b;

  for (i = 0; i < nbl; i++)
    {
      b = BLOCKVECTOR_BLOCK (bv, i);
      if (BLOCK_SHOULD_SORT (b))
	sort_block_syms (b);
    }
}

void
sort_all_symtab_syms ()
{
  register struct symtab *s;

  for (s = symtab_list; s; s = s->next)
    {
      sort_symtab_syms (s);
    }
}

/* Make a copy of the string at PTR with SIZE characters in the symbol obstack
   (and add a null character at the end in the copy).
   Returns the address of the copy.  */

char *
obsavestring (ptr, size)
     char *ptr;
     int size;
{
  register char *p = (char *) obstack_alloc (symbol_obstack, size + 1);
  /* Open-coded bcopy--saves function call time.
     These strings are usually short.  */
  {
    register char *p1 = ptr;
    register char *p2 = p;
    char *end = ptr + size;
    while (p1 != end)
      *p2++ = *p1++;
  }
  p[size] = 0;
  return p;
}

/* Concatenate strings S1, S2 and S3; return the new string.
   Space is found in the symbol_obstack.  */

char *
obconcat (s1, s2, s3)
     char *s1, *s2, *s3;
{
  register int len = strlen (s1) + strlen (s2) + strlen (s3) + 1;
  register char *val = (char *) obstack_alloc (symbol_obstack, len);
  strcpy (val, s1);
  strcat (val, s2);
  strcat (val, s3);
  return val;
}

/* Accumulate the misc functions in bunches of 127.
   At the end, copy them all into one newly allocated structure.  */

#define MISC_BUNCH_SIZE 127

struct misc_bunch
{
  struct misc_bunch *next;
  struct misc_function contents[MISC_BUNCH_SIZE];
};

/* Bunch currently being filled up.
   The next field points to chain of filled bunches.  */

static struct misc_bunch *misc_bunch;

/* Number of slots filled in current bunch.  */

static int misc_bunch_index;

/* Total number of misc functions recorded so far.  */

static int misc_count;

void
init_misc_bunches ()
{
  misc_count = 0;
  misc_bunch = 0;
  misc_bunch_index = MISC_BUNCH_SIZE;
}

void
prim_record_misc_function (name, address, misc_type)
     char *name;
     CORE_ADDR address;
     enum misc_function_type misc_type;
{
  register struct misc_bunch *new;

  if (misc_bunch_index == MISC_BUNCH_SIZE)
    {
      new = (struct misc_bunch *) xmalloc (sizeof (struct misc_bunch));
      misc_bunch_index = 0;
      new->next = misc_bunch;
      misc_bunch = new;
    }
  misc_bunch->contents[misc_bunch_index].name = name;
  misc_bunch->contents[misc_bunch_index].address = address;
  misc_bunch->contents[misc_bunch_index].type = misc_type;
  misc_bunch_index++;
  misc_count++;
}

static int
compare_misc_functions (fn1, fn2)
     struct misc_function *fn1, *fn2;
{
  /* Return a signed result based on unsigned comparisons
     so that we sort into unsigned numeric order.  */
  if (fn1->address < fn2->address)
    return -1;
  if (fn1->address > fn2->address)
    return 1;
  return 0;
}

/* ARGSUSED */
void
discard_misc_bunches (foo)
     int foo;
{
  register struct misc_bunch *next;

  while (misc_bunch)
    {
      next = misc_bunch->next;
      free (misc_bunch);
      misc_bunch = next;
    }
}

/* INCLINK nonzero means bunches are from an incrementally-linked file.
   Add them to the existing bunches.
   Otherwise INCLINK is zero, and we start from scratch. */
void
condense_misc_bunches (inclink)
     int inclink;
{
  register int i, j;
  register struct misc_bunch *bunch;

  if (inclink)
    {
      misc_function_vector
	= (struct misc_function *)
	  xrealloc (misc_function_vector, (misc_count + misc_function_count)
		    * sizeof (struct misc_function));
      j = misc_function_count;
    }
  else
    {
      misc_function_vector
	= (struct misc_function *)
	  xmalloc (misc_count * sizeof (struct misc_function));
      j = 0;
    }

  bunch = misc_bunch;
  while (bunch)
    {
      for (i = 0; i < misc_bunch_index; i++, j++)
        {
	  misc_function_vector[j] = bunch->contents[i];
#ifdef NAMES_HAVE_UNDERSCORE
	  if (misc_function_vector[j].name[0] == '_')
	      misc_function_vector[j].name++;
#endif
	}
      bunch = bunch->next;
      misc_bunch_index = MISC_BUNCH_SIZE;
    }

  if (misc_function_count + misc_count != j)		/* DEBUG */
    printf_filtered ("Function counts are off!  %d + %d != %d\n",
      misc_function_count, misc_count, j);

  misc_function_count = j;

  /* Sort the misc functions by address.  */

  qsort (misc_function_vector, misc_function_count,
	 sizeof (struct misc_function),
	 compare_misc_functions);
}


/* Get the symbol table that corresponds to a partial_symtab.
   This is fast after the first time you do it.  In fact, there
   is an even faster macro PSYMTAB_TO_SYMTAB that does the fast
   case inline.  */

struct symtab *
psymtab_to_symtab (pst)
     register struct partial_symtab *pst;
{
  register struct symtab *result;

  /* If it's been looked up before, return it. */
  if (pst->symtab)
    return pst->symtab;

  /* If it has not yet been read in, read it.  */
  if (!pst->readin)
    { 
      (*pst->read_symtab) (pst);
    }

  /* Search through list for correct name. */
  for (result = symtab_list; result; result = result->next)
    if (!strcmp (result->filename, pst->filename))
      {
	pst->symtab = result;		/* Remember where it was.  */
	return result;
      }

  return 0;
}

/* Process a symbol file, as either the main file or as a dynamically
   loaded file.

   NAME is the file name (which will be tilde-expanded and made absolute
   herein).  FROM_TTY says how verbose to be.  MAINLINE specifies whether
   this is the main symbol file, or whether it's an extra symbol file
   such as dynamically loaded code.  If !mainline, ADDR is the address
   where the text segment was loaded.  */

void
symbol_file_add (name, from_tty, addr, mainline)
     char *name;
     int from_tty;
     CORE_ADDR addr;
     int mainline;
{
  bfd *sym_bfd;
  asection *text_sect;
  struct sym_fns *sf;
  char *realname;

  sym_bfd = symfile_open (name);

  entry_point = bfd_get_start_address (sym_bfd);

  if (mainline)
    symfile_mtime = bfd_get_mtime (sym_bfd);

  /* There is a distinction between having no symbol table
     (we refuse to read the file, leaving the old set of symbols around)
     and having no debugging symbols in your symbol table (we read
     the file and end up with a mostly empty symbol table).  */

  if (!(bfd_get_file_flags (sym_bfd) & HAS_SYMS))
    {
      error ("%s has no symbol-table", name);
    }

  if ((symtab_list || partial_symtab_list)
      && mainline
      && from_tty
      && !query ("Load new symbol table from \"%s\"? ", name))
    error ("Not confirmed.");

  if (from_tty)
    {
      printf ("Reading symbol data from %s...", name);
      fflush (stdout);
    }

  sf = symfile_init (sym_bfd);
  realname = bfd_get_filename (sym_bfd);
  realname = savestring (realname, strlen (realname));
  /* FIXME, this probably creates a storage leak... */

  if (mainline) 
    {
      /* Since no error yet, throw away the old symbol table.  */

      if (symfile)
	free (symfile);
      symfile = 0;
      free_all_symtabs ();
      free_all_psymtabs ();

      (*sf->sym_new_init) ();

      /* For mainline, caller didn't know the specified address of the
         text section.  We fix that here.  */
      text_sect = bfd_get_section_by_name (sym_bfd, ".text");
      addr = bfd_section_vma (sym_bfd, text_sect);
    }

  clear_complaints();	/* Allow complaints to appear for this new file. */

  (*sf->sym_read) (sf, addr, mainline);

  /* Don't allow char * to have a typename (else would get caddr_t.)  */
  /* Ditto void *.  FIXME should do this for all the builtin types.  */

  TYPE_NAME (lookup_pointer_type (builtin_type_char)) = 0;
  TYPE_NAME (lookup_pointer_type (builtin_type_void)) = 0;

  if (mainline)
    {
      /* OK, make it the "real" symbol file.  */
      symfile = realname;
      symfile_fns = sf;
    }

  if (from_tty)
    {
      printf ("done.\n");
      fflush (stdout);
    }
}

/* This is the symbol-file command.  Read the file, analyze its symbols,
   and add a struct symtab to symtab_list.  */

void
symbol_file_command (name, from_tty)
     char *name;
     int from_tty;
{

  dont_repeat ();

  if (name == 0)
    {
      if ((symtab_list || partial_symtab_list)
	  && from_tty
	  && !query ("Discard symbol table from `%s'? ", symfile))
	error ("Not confirmed.");
      if (symfile)
	free (symfile);
      symfile = 0;
      free_all_symtabs ();
      free_all_psymtabs ();
      /* FIXME, this does not account for the main file and subsequent
         files (shared libs, dynloads, etc) having different formats. 
         It only calls the cleanup routine for the main file's format.  */
      (*symfile_fns->sym_new_init) ();
      free (symfile_fns);
      symfile_fns = 0;
      return;
    }

  symbol_file_add (name, from_tty, (CORE_ADDR)0, 1);
}

/* Open NAME and hand it off to BFD for preliminary analysis.  Result
   is a BFD *, which includes a new copy of NAME dynamically allocated
   (which will be freed by the cleanup chain).  In case of trouble,
   error() is called.  */

static bfd *
symfile_open (name)
     char *name;
{
  bfd *sym_bfd;
  int desc;
  char *absolute_name;

  name = tilde_expand (name);
  make_cleanup (free, name);

  desc = openp (getenv ("PATH"), 1, name, O_RDONLY, 0, &absolute_name);
  if (desc < 0)
    perror_with_name (name);
  else
    {
      make_cleanup (free, absolute_name);
      name = absolute_name;
    }

  sym_bfd = bfd_fdopenr (name, NULL, desc);
  if (!sym_bfd)
    {
      close (desc);
      error ("Could not open `%s' to read symbols: %s",
	     name, bfd_errmsg (bfd_error));
    }
  make_cleanup (bfd_close, sym_bfd);

  if (!bfd_check_format (sym_bfd, bfd_object))
    error ("\"%s\": can't read symbols: %s.",
	   name, bfd_errmsg (bfd_error));

  return sym_bfd;
}

/* Initialize to read symbols from the symbol file sym_bfd.  It either
   returns or calls error().  The result is a malloc'd struct sym_fns
   that contains cached information about the symbol file.  */

static struct sym_fns *
symfile_init (sym_bfd)
     bfd *sym_bfd;
{
  struct sym_fns *sf, *sf2;

  for (sf = symtab_fns; sf->sym_name; sf++)
    {
      if (!strncmp (bfd_get_target (sym_bfd), sf->sym_name, sf->sym_namelen))
	{
	  sf2 = (struct sym_fns *)xmalloc (sizeof (*sf2));	
	  /* FIXME, who frees this? */
	  *sf2 = *sf;
	  sf2->sym_bfd = sym_bfd;
	  sf2->sym_private = 0;			/* Not alloc'd yet */
	  (*sf2->sym_init) (sf2);
	  return sf2;
	}
    }
  error ("I'm sorry, Dave, I can't do that.  Symbol format unknown.");
}

/* This function runs the add_file command of our current target.  */

void
add_file_target_command (arg, from_tty)
     char *arg;
     int from_tty;
{
  target_add_file (arg, from_tty);
}

/* This function allows the addition of incrementally linked object files.  */

void
add_file_addr_command (arg_string, from_tty)
     char* arg_string;
     int from_tty;
{
  char *name;
  CORE_ADDR text_addr;
  
  if (arg_string == 0)
    error ("add-file takes a file name and an address");

  arg_string = tilde_expand (arg_string);
  make_cleanup (free, arg_string);

  for( ; *arg_string == ' '; arg_string++ );
  name = arg_string;
  for( ; *arg_string && *arg_string != ' ' ; arg_string++ );
  *arg_string++ = (char) 0;

  if (name[0] == 0)
    error ("add-file takes a file name and an address");

  text_addr = parse_and_eval_address (arg_string);

  dont_repeat ();

  if (!query ("add symbol table from filename \"%s\" at text_addr = 0x%x\n",
	      name, text_addr))
    error ("Not confirmed.");

  symbol_file_add (name, 0, text_addr, 0);
}

/* Re-read symbols if the symbol-file has changed.  */
void
reread_symbols ()
{
  struct stat symstat;

  /* With the addition of shared libraries, this should be modified,
     the load time should be saved in the partial symbol tables, since
     different tables may come from different source files.  FIXME.
     This routine should then walk down each partial symbol table
     and see if the symbol table that it originates from has been changed
  */

  if (stat (symfile, &symstat) < 0)
    /* Can't read symbol-file.  Assume it is up to date.  */
    return;

  if (symstat.st_mtime > symfile_mtime)
    {
      printf_filtered ("Symbol file has changed; re-reading symbols.\n");
      symbol_file_command (symfile, 0);
      breakpoint_re_set ();
    }
}

/* Return name of file symbols were loaded from, or 0 if none..  */

char *
get_sym_file ()
{
  return symfile;
}

/* This function is really horrible, but to avoid it, there would need
   to be more filling in of forward references.  */
int
fill_in_vptr_fieldno (type)
     struct type *type;
{
  if (TYPE_VPTR_FIELDNO (type) < 0)
    TYPE_VPTR_FIELDNO (type) =
      fill_in_vptr_fieldno (TYPE_BASECLASS (type, 1));
  return TYPE_VPTR_FIELDNO (type);
}

/* Functions to handle complaints during symbol reading.  */

/* How many complaints about a particular thing should be printed before
   we stop whining about it?  */

static unsigned stop_whining = 1;

/* Print a complaint about the input symbols, and link the complaint block
   into a chain for later handling.  Result is 1 if the complaint was
   printed, 0 if it was suppressed.  */

int
complain (complaint, val)
     struct complaint *complaint;
     char *val;
{
  complaint->counter++;
  if (complaint->next == 0) {
    complaint->next = complaint_root->next;
    complaint_root->next = complaint;
  }
  if (complaint->counter > stop_whining)
    return 0;
  wrap_here ("");
  if (!info_verbose) {
    puts_filtered ("During symbol reading...");
  }
  printf_filtered (complaint->message, val);
  puts_filtered ("...");
  wrap_here("");
  if (!info_verbose)
    puts_filtered ("\n");
  return 1;
}

/* Clear out all complaint counters that have ever been incremented.  */

void
clear_complaints ()
{
  struct complaint *p;

  for (p = complaint_root->next; p != complaint_root; p = p->next)
    p->counter = 0;
}

void
_initialize_symfile ()
{

  add_com ("symbol-file", class_files, symbol_file_command,
	   "Load symbol table from executable file FILE.\n\
The `file' command can also load symbol tables, as well as setting the file\n\
to execute.");

  add_com ("add-file", class_files, add_file_target_command,
   "Load the symbols from FILE, assuming FILE has been dynamically loaded.\n\
For most environments, a second argument provides the starting address of\n\
the file's text.  For VxWorks, the file is actually loaded into the target\n\
system and dynamically linked, and no further argument is needed.");

  add_alias_cmd ("load", "add-file", class_files, 0, &cmdlist);

  add_show_from_set
    (add_set_cmd ("complaints", class_support, var_uinteger,
		  (char *)&stop_whining,
	  "Set max number of complaints about incorrect symbols.",
		  &setlist),
     &showlist);

  obstack_init (symbol_obstack);
  obstack_init (psymbol_obstack);
}
