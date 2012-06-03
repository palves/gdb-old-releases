/* Read dbx symbol tables and convert to internal format, for GDB.
   Copyright (C) 1986-1991 Free Software Foundation, Inc.

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

/* This module provides three functions: dbx_symfile_init,
   which initializes to read a symbol file; dbx_new_init, which 
   discards existing cached information when all symbols are being
   discarded; and dbx_symfile_read, which reads a symbol table
   from a file.

   dbx_symfile_read only does the minimum work necessary for letting the
   user "name" things symbolically; it does not read the entire symtab.
   Instead, it reads the external and static symbols and puts them in partial
   symbol tables.  When more extensive information is requested of a
   file, the corresponding partial symbol table is mutated into a full
   fledged symbol table by going back and reading the symbols
   for real.  dbx_psymtab_to_symtab() is the function that does this */

#include <stdio.h>
#include <string.h>
#include "defs.h"

#ifdef USG
#include <sys/types.h>
#include <fcntl.h>
#define L_SET 0
#define L_INCR 1
#endif

#include <obstack.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <ctype.h>
#include "symtab.h"
#include "breakpoint.h"
#include "command.h"
#include "target.h"
#include "gdbcore.h"		/* for bfd stuff */
#include "libaout.h"	 	/* FIXME Secret internal BFD stuff for a.out */
#include "symfile.h"
#include "buildsym.h"

#include "aout/aout64.h"
#include "aout/stab_gnu.h"	/* We always use GNU stabs, not native, now */

/* Information is passed among various dbxread routines for accessing
   symbol files.  A pointer to this structure is kept in the sym_private
   field of the struct sym_fns passed in by symfile.h.  */
 
struct dbx_symfile_info {
  asection *text_sect;		/* Text section accessor */
  int symcount;			/* How many symbols are there in the file */
  char *stringtab;		/* The actual string table */
  int stringtab_size;		/* Its size */
  off_t symtab_offset;		/* Offset in file to symbol table */
};


/* Each partial symbol table entry contains a pointer to private data for the
   read_symtab() function to use when expanding a partial symbol table entry
   to a full symbol table entry.

   For dbxread this structure contains the offset within the file symbol table
   of first local symbol for this file, and length (in bytes) of the section
   of the symbol table devoted to this file's symbols (actually, the section
   bracketed may contain more than just this file's symbols).  If ldsymlen is
   0, the only reason for this thing's existence is the dependency list.
   Nothing else will happen when it is read in. */

#define LDSYMOFF(p) (((struct symloc *)((p)->read_symtab_private))->ldsymoff)
#define LDSYMLEN(p) (((struct symloc *)((p)->read_symtab_private))->ldsymlen)

struct symloc {
  int ldsymoff;
  int ldsymlen;
};

extern void qsort ();
extern double atof ();

/* Forward declarations */

static void read_dbx_symtab ();
static void init_psymbol_list ();
static void process_one_symbol ();
void start_subfile ();
int hashname ();
static struct pending *copy_pending ();
static struct symtab *read_ofile_symtab ();
static void dbx_psymtab_to_symtab ();

/* Macro to determine which symbols to ignore when reading the first symbol
   of a file.  Some machines override this definition. */
#ifndef IGNORE_SYMBOL
/* This code is used on Ultrix systems.  Ignore it */
#define IGNORE_SYMBOL(type)  (type == (int)N_NSYMS)
#endif

/* Macro for name of symbol to indicate a file compiled with gcc. */
#ifndef GCC_COMPILED_FLAG_SYMBOL
#define GCC_COMPILED_FLAG_SYMBOL "gcc_compiled."
#endif

/* Define this as 1 if a pcc declaration of a char or short argument
   gives the correct address.  Otherwise assume pcc gives the
   address of the corresponding int, which is not the same on a
   big-endian machine.  */

#ifndef BELIEVE_PCC_PROMOTION
#define BELIEVE_PCC_PROMOTION 0
#endif

/* Nonzero means give verbose info on gdb action.  From main.c.  */
extern int info_verbose;

/* The BFD for this file -- implicit parameter to next_symbol_text.  */

static bfd *symfile_bfd;

/* The objfile for this file -- only good in process_one_symbol().  */

static struct objfile *our_objfile;

/* String table for the main symbol file.  It is kept in memory
   permanently, to speed up symbol reading.  Other files' symbol tables
   are read in on demand.  FIXME, this should be cleaner.  */

static char *symfile_string_table;
static int symfile_string_table_size;

/* The size of each symbol in the symbol file (in external form).
   This is set by dbx_symfile_read when building psymtabs, and by
   dbx_psymtab_to_symtab when building symtabs.  */

static unsigned symbol_size;

/* Complaints about the symbols we have encountered.  */

struct complaint lbrac_complaint = 
  {"bad block start address patched", 0, 0};

struct complaint string_table_offset_complaint =
  {"bad string table offset in symbol %d", 0, 0};

struct complaint unknown_symtype_complaint =
  {"unknown symbol type %s", 0, 0};

struct complaint lbrac_rbrac_complaint =
  {"block start larger than block end", 0, 0};

struct complaint lbrac_unmatched_complaint =
  {"unmatched N_LBRAC before symtab pos %d", 0, 0};

struct complaint lbrac_mismatch_complaint =
  {"N_LBRAC/N_RBRAC symbol mismatch at symtab pos %d", 0, 0};

/* During initial symbol readin, we need to have a structure to keep
   track of which psymtabs have which bincls in them.  This structure
   is used during readin to setup the list of dependencies within each
   partial symbol table. */

struct header_file_location
{
  char *name;			/* Name of header file */
  int instance;			/* See above */
  struct partial_symtab *pst;	/* Partial symtab that has the
				   BINCL/EINCL defs for this file */
};

/* The actual list and controling variables */
static struct header_file_location *bincl_list, *next_bincl;
static int bincls_allocated;

/* Free up old header file tables, and allocate new ones.
   We're reading a new symbol file now.  */

void
free_and_init_header_files ()
{
  register int i;
  for (i = 0; i < n_header_files; i++)
    free (header_files[i].name);
  if (header_files)			/* First time null */
    free (header_files);
  if (this_object_header_files)		/* First time null */
    free (this_object_header_files);

  n_allocated_header_files = 10;
  header_files = (struct header_file *) xmalloc (10 * sizeof (struct header_file));
  n_header_files = 0;

  n_allocated_this_object_header_files = 10;
  this_object_header_files = (int *) xmalloc (10 * sizeof (int));
}

/* Called at the start of each object file's symbols.
   Clear out the mapping of header file numbers to header files.  */

void
new_object_header_files ()
{
  /* Leave FILENUM of 0 free for builtin types and this file's types.  */
  n_this_object_header_files = 1;
  header_file_prev_index = -1;
}

/* Add header file number I for this object file
   at the next successive FILENUM.  */

static void
add_this_object_header_file (i)
     int i;
{
  if (n_this_object_header_files == n_allocated_this_object_header_files)
    {
      n_allocated_this_object_header_files *= 2;
      this_object_header_files
	= (int *) xrealloc (this_object_header_files,
			    n_allocated_this_object_header_files * sizeof (int));
    }

  this_object_header_files[n_this_object_header_files++] = i;
}

/* Add to this file an "old" header file, one already seen in
   a previous object file.  NAME is the header file's name.
   INSTANCE is its instance code, to select among multiple
   symbol tables for the same header file.  */

static void
add_old_header_file (name, instance)
     char *name;
     int instance;
{
  register struct header_file *p = header_files;
  register int i;

  for (i = 0; i < n_header_files; i++)
    if (!strcmp (p[i].name, name) && instance == p[i].instance)
      {
	add_this_object_header_file (i);
	return;
      }
  error ("Invalid symbol data: \"repeated\" header file that hasn't been seen before, at symtab pos %d.",
	 symnum);
}

/* Add to this file a "new" header file: definitions for its types follow.
   NAME is the header file's name.
   Most often this happens only once for each distinct header file,
   but not necessarily.  If it happens more than once, INSTANCE has
   a different value each time, and references to the header file
   use INSTANCE values to select among them.

   dbx output contains "begin" and "end" markers for each new header file,
   but at this level we just need to know which files there have been;
   so we record the file when its "begin" is seen and ignore the "end".  */

static void
add_new_header_file (name, instance)
     char *name;
     int instance;
{
  register int i;
  header_file_prev_index = -1;

  /* Make sure there is room for one more header file.  */

  if (n_header_files == n_allocated_header_files)
    {
      n_allocated_header_files *= 2;
      header_files = (struct header_file *)
	xrealloc (header_files,
		  (n_allocated_header_files
		   * sizeof (struct header_file)));
    }

  /* Create an entry for this header file.  */

  i = n_header_files++;
  header_files[i].name = savestring (name, strlen(name));
  header_files[i].instance = instance;
  header_files[i].length = 10;
  header_files[i].vector
    = (struct type **) xmalloc (10 * sizeof (struct type *));
  bzero (header_files[i].vector, 10 * sizeof (struct type *));

  add_this_object_header_file (i);
}

#if 0
static struct type **
explicit_lookup_type (real_filenum, index)
     int real_filenum, index;
{
  register struct header_file *f = &header_files[real_filenum];

  if (index >= f->length)
    {
      f->length *= 2;
      f->vector = (struct type **)
	xrealloc (f->vector, f->length * sizeof (struct type *));
      bzero (&f->vector[f->length / 2],
	     f->length * sizeof (struct type *) / 2);
    }
  return &f->vector[index];
}
#endif

static void
record_misc_function (name, address, type)
     char *name;
     CORE_ADDR address;
     int type;
{
  enum misc_function_type misc_type;

  switch (type &~ N_EXT) {
    case N_TEXT:  misc_type = mf_text; break;
    case N_DATA:  misc_type = mf_data; break;
    case N_BSS:   misc_type = mf_bss;  break;
    case N_ABS:   misc_type = mf_abs;  break;
#ifdef N_SETV
    case N_SETV:  misc_type = mf_data; break;
#endif
    default:      misc_type = mf_unknown; break;
  }

  prim_record_misc_function (obsavestring (name, strlen (name)),
			     address, misc_type);
}

/* Scan and build partial symbols for a symbol file.
   We have been initialized by a call to dbx_symfile_init, which 
   put all the relevant info into a "struct dbx_symfile_info"
   hung off the struct sym_fns SF.

   ADDR is the address relative to which the symbols in it are (e.g.
   the base address of the text segment).
   MAINLINE is true if we are reading the main symbol
   table (as opposed to a shared lib or dynamically loaded file).  */

static void
dbx_symfile_read (sf, addr, mainline)
     struct sym_fns *sf;
     CORE_ADDR addr;
     int mainline;	/* FIXME comments above */
{
  struct dbx_symfile_info *info = (struct dbx_symfile_info *) (sf->sym_private);
  bfd *sym_bfd = sf->objfile->obfd;
  int val;

  val = bfd_seek (sf->objfile->obfd, info->symtab_offset, L_SET);
  if (val < 0)
    perror_with_name (sf->objfile->name);

  /* If mainline, set global string table pointers, and reinitialize global
     partial symbol list.  */
  if (mainline) {
    symfile_string_table = info->stringtab;
    symfile_string_table_size = info->stringtab_size;
  }

  /* If we are reinitializing, or if we have never loaded syms yet, init */
  if (mainline || global_psymbols.size == 0 || static_psymbols.size == 0)
    init_psymbol_list (info->symcount);

  /* FIXME POKING INSIDE BFD DATA STRUCTURES */
  symbol_size = obj_symbol_entry_size (sym_bfd);

  pending_blocks = 0;
  make_cleanup (really_free_pendings, 0);

  init_misc_bunches ();
  make_cleanup (discard_misc_bunches, 0);

  /* Now that the symbol table data of the executable file are all in core,
     process them and define symbols accordingly.  */

  read_dbx_symtab (addr - bfd_section_vma (sym_bfd, info->text_sect), /*offset*/
		   sf->objfile, info->stringtab, info->stringtab_size,
		   info->symcount,
		   bfd_section_vma  (sym_bfd, info->text_sect),
		   bfd_section_size (sym_bfd, info->text_sect));

  /* Go over the misc symbol bunches and install them in vector.  */

  condense_misc_bunches (!mainline);

  /* Free up any memory we allocated for ourselves.  */

  if (!mainline) {
    free (info->stringtab);	/* Stringtab is only saved for mainline */
  }
  free (info);
  sf->sym_private = 0;		/* Zap pointer to our (now gone) info struct */

  if (!partial_symtab_list) {
    wrap_here ("");
    printf_filtered ("(no debugging symbols found)...");
    wrap_here ("");
  }
}

/* Initialize anything that needs initializing when a completely new
   symbol file is specified (not just adding some symbols from another
   file, e.g. a shared library).  */

static void
dbx_new_init ()
{
  buildsym_new_init ();

  /* Don't put these on the cleanup chain; they need to stick around
     until the next call to dbx_new_init.  *Then* we'll free them. */
  if (symfile_string_table)
    {
      free (symfile_string_table);
      symfile_string_table = 0;
      symfile_string_table_size = 0;
    }
  free_and_init_header_files ();
}


/* dbx_symfile_init ()
   is the dbx-specific initialization routine for reading symbols.
   It is passed a struct sym_fns which contains, among other things,
   the BFD for the file whose symbols are being read, and a slot for a pointer
   to "private data" which we fill with goodies.

   We read the string table into malloc'd space and stash a pointer to it.

   Since BFD doesn't know how to read debug symbols in a format-independent
   way (and may never do so...), we have to do it ourselves.  We will never
   be called unless this is an a.out (or very similar) file. 
   FIXME, there should be a cleaner peephole into the BFD environment here.  */

static void
dbx_symfile_init (sf)
  struct sym_fns *sf;
{
  int val;
  bfd *sym_bfd = sf->objfile->obfd;
  char *name = bfd_get_filename (sym_bfd);
  struct dbx_symfile_info *info;
  unsigned char size_temp[4];

  /* Allocate struct to keep track of the symfile */
  sf->sym_private = xmalloc (sizeof (*info));
  info = (struct dbx_symfile_info *)sf->sym_private;

  /* FIXME POKING INSIDE BFD DATA STRUCTURES */
#define	STRING_TABLE_OFFSET	(sym_bfd->origin + obj_str_filepos (sym_bfd))
#define	SYMBOL_TABLE_OFFSET	(sym_bfd->origin + obj_sym_filepos (sym_bfd))
  /* FIXME POKING INSIDE BFD DATA STRUCTURES */

  info->text_sect = bfd_get_section_by_name (sym_bfd, ".text");
  if (!info->text_sect)
    abort();
  info->symcount = bfd_get_symcount (sym_bfd);

  /* Read the string table size and check it for bogosity.  */
  val = bfd_seek (sym_bfd, STRING_TABLE_OFFSET, L_SET);
  if (val < 0)
      perror_with_name (name);

  val = bfd_read (size_temp, sizeof (long), 1, sym_bfd);
  if (val < 0)
      perror_with_name (name);
  info->stringtab_size = bfd_h_get_32 (sym_bfd, size_temp);
  
  if (info->stringtab_size >= 0)
    {
      info->stringtab = (char *) xmalloc (info->stringtab_size);
      /* Caller is responsible for freeing the string table.  No cleanup. */
    }
  else
    info->stringtab = NULL;
  if (info->stringtab == NULL && info->stringtab_size != 0)
    error ("ridiculous string table size: %d bytes", info->stringtab_size);

  /* Now read in the string table in one big gulp.  */

  val = bfd_seek (sym_bfd, STRING_TABLE_OFFSET, L_SET);
  if (val < 0)
    perror_with_name (name);
  val = bfd_read (info->stringtab, info->stringtab_size, 1, sym_bfd);
  if (val != info->stringtab_size)
    perror_with_name (name);

  /* Record the position of the symbol table for later use.  */

  info->symtab_offset = SYMBOL_TABLE_OFFSET;
}

/* Buffer for reading the symbol table entries.  */
static struct internal_nlist symbuf[4096];
static int symbuf_idx;
static int symbuf_end;

/* The address in memory of the string table of the object file we are
   reading (which might not be the "main" object file, but might be a
   shared library or some other dynamically loaded thing).  This is set
   by read_dbx_symtab when building psymtabs, and by read_ofile_symtab 
   when building symtabs, and is used only by next_symbol_text.  */
static char *stringtab_global;

/* Refill the symbol table input buffer
   and set the variables that control fetching entries from it.
   Reports an error if no data available.
   This function can read past the end of the symbol table
   (into the string table) but this does no harm.  */

static void
fill_symbuf (sym_bfd)
     bfd *sym_bfd;
{
  int nbytes = bfd_read (symbuf, sizeof (symbuf), 1, sym_bfd);
  if (nbytes < 0)
    perror_with_name (bfd_get_filename (sym_bfd));
  else if (nbytes == 0)
    error ("Premature end of file reading symbol table");
  symbuf_end = nbytes / symbol_size;
  symbuf_idx = 0;
}

#define SWAP_SYMBOL(symp, abfd) \
  { \
    (symp)->n_strx = bfd_h_get_32(abfd,			\
				(unsigned char *)&(symp)->n_strx);	\
    (symp)->n_desc = bfd_h_get_16 (abfd,			\
				(unsigned char *)&(symp)->n_desc);  	\
    (symp)->n_value = bfd_h_get_32 (abfd,			\
				(unsigned char *)&(symp)->n_value); 	\
  }

/* Invariant: The symbol pointed to by symbuf_idx is the first one
   that hasn't been swapped.  Swap the symbol at the same time
   that symbuf_idx is incremented.  */

/* dbx allows the text of a symbol name to be continued into the
   next symbol name!  When such a continuation is encountered
   (a \ at the end of the text of a name)
   call this function to get the continuation.  */

char *
next_symbol_text ()
{
  if (symbuf_idx == symbuf_end)
    fill_symbuf (symfile_bfd);
  symnum++;
  SWAP_SYMBOL(&symbuf[symbuf_idx], symfile_bfd);
  return symbuf[symbuf_idx++].n_strx + stringtab_global;
}

/* Initializes storage for all of the partial symbols that will be
   created by read_dbx_symtab and subsidiaries.  */

static void
init_psymbol_list (total_symbols)
     int total_symbols;
{
  /* Free any previously allocated psymbol lists.  */
  if (global_psymbols.list)
    free (global_psymbols.list);
  if (static_psymbols.list)
    free (static_psymbols.list);

  /* Current best guess is that there are approximately a twentieth
     of the total symbols (in a debugging file) are global or static
     oriented symbols */
  global_psymbols.size = total_symbols / 10;
  static_psymbols.size = total_symbols / 10;
  global_psymbols.next = global_psymbols.list = (struct partial_symbol *)
    xmalloc (global_psymbols.size * sizeof (struct partial_symbol));
  static_psymbols.next = static_psymbols.list = (struct partial_symbol *)
    xmalloc (static_psymbols.size * sizeof (struct partial_symbol));
}

/* Initialize the list of bincls to contain none and have some
   allocated.  */

static void
init_bincl_list (number)
     int number;
{
  bincls_allocated = number;
  next_bincl = bincl_list = (struct header_file_location *)
      xmalloc (bincls_allocated * sizeof(struct header_file_location));
}

/* Add a bincl to the list.  */

static void
add_bincl_to_list (pst, name, instance)
     struct partial_symtab *pst;
     char *name;
     int instance;
{
  if (next_bincl >= bincl_list + bincls_allocated)
    {
      int offset = next_bincl - bincl_list;
      bincls_allocated *= 2;
      bincl_list = (struct header_file_location *)
	xrealloc ((char *)bincl_list,
		  bincls_allocated * sizeof (struct header_file_location));
      next_bincl = bincl_list + offset;
    }
  next_bincl->pst = pst;
  next_bincl->instance = instance;
  next_bincl++->name = name;
}

/* Given a name, value pair, find the corresponding
   bincl in the list.  Return the partial symtab associated
   with that header_file_location.  */

static struct partial_symtab *
find_corresponding_bincl_psymtab (name, instance)
     char *name;
     int instance;
{
  struct header_file_location *bincl;

  for (bincl = bincl_list; bincl < next_bincl; bincl++)
    if (bincl->instance == instance
	&& !strcmp (name, bincl->name))
      return bincl->pst;

  return (struct partial_symtab *) 0;
}

/* Free the storage allocated for the bincl list.  */

static void
free_bincl_list ()
{
  free (bincl_list);
  bincls_allocated = 0;
}

static struct partial_symtab *start_psymtab ();
static void end_psymtab();

#ifdef DEBUG
/* This is normally a macro defined in read_dbx_symtab, but this
   is a lot easier to debug.  */

ADD_PSYMBOL_TO_PLIST(NAME, NAMELENGTH, NAMESPACE, CLASS, PLIST, VALUE)
     char *NAME;
     int NAMELENGTH;
     enum namespace NAMESPACE;
     enum address_class CLASS;
     struct psymbol_allocation_list *PLIST;
     unsigned long VALUE;
{
  register struct partial_symbol *psym;

#define LIST *PLIST
  do {		        						
    if ((LIST).next >=					
	(LIST).list + (LIST).size)			
      {									
	(LIST).list = (struct partial_symbol *)				
	  xrealloc ((LIST).list,					
		    ((LIST).size * 2					
		     * sizeof (struct partial_symbol)));		
	/* Next assumes we only went one over.  Should be good if	
	   program works correctly */					
	(LIST).next =							
	  (LIST).list + (LIST).size;				
	(LIST).size *= 2;				
      }									
    psym = (LIST).next++;						
#undef LIST

    SYMBOL_NAME (psym) = (char *) obstack_alloc (psymbol_obstack,	
						 (NAMELENGTH) + 1);	
    strncpy (SYMBOL_NAME (psym), (NAME), (NAMELENGTH));			
    SYMBOL_NAME (psym)[(NAMELENGTH)] = '\0';				
    SYMBOL_NAMESPACE (psym) = (NAMESPACE);				
    SYMBOL_CLASS (psym) = (CLASS);				
    SYMBOL_VALUE (psym) = (VALUE); 					
  } while (0);
}

/* Since one arg is a struct, we have to pass in a ptr and deref it (sigh) */
#define	ADD_PSYMBOL_TO_LIST(NAME, NAMELENGTH, NAMESPACE, CLASS,  LIST, VALUE) \
       ADD_PSYMBOL_TO_PLIST(NAME, NAMELENGTH, NAMESPACE, CLASS, &LIST, VALUE)

#endif /* DEBUG */

/* Given pointers to an a.out symbol table in core containing dbx
   style data, setup partial_symtab's describing each source file for
   which debugging information is available.  NLISTLEN is the number
   of symbols in the symbol table.  All symbol names are given as
   offsets relative to STRINGTAB.  STRINGTAB_SIZE is the size of
   STRINGTAB.  SYMFILE_NAME is the name of the file we are reading from
   and ADDR is its relocated address (if incremental) or 0 (if not).  */

static void
read_dbx_symtab (addr, objfile, stringtab, stringtab_size, nlistlen,
		 text_addr, text_size)
     CORE_ADDR addr;
     struct objfile *objfile;
     register char *stringtab;
     register long stringtab_size;
     register int nlistlen;
     CORE_ADDR text_addr;
     int text_size;
{
  register struct internal_nlist *bufp;
  register char *namestring;
  register struct partial_symbol *psym;
  int nsl;
  int past_first_source_file = 0;
  CORE_ADDR last_o_file_start = 0;
  struct cleanup *old_chain;
  char *p;
  bfd *abfd;

  /* End of the text segment of the executable file.  */
  CORE_ADDR end_of_text_addr;

  /* Current partial symtab */
  struct partial_symtab *pst;

  /* List of current psymtab's include files */
  char **psymtab_include_list;
  int includes_allocated;
  int includes_used;

  /* Index within current psymtab dependency list */
  struct partial_symtab **dependency_list;
  int dependencies_used, dependencies_allocated;

  stringtab_global = stringtab;
  
  pst = (struct partial_symtab *) 0;

  includes_allocated = 30;
  includes_used = 0;
  psymtab_include_list = (char **) alloca (includes_allocated *
					   sizeof (char *));

  dependencies_allocated = 30;
  dependencies_used = 0;
  dependency_list =
    (struct partial_symtab **) alloca (dependencies_allocated *
				       sizeof (struct partial_symtab *));

  old_chain = make_cleanup (free_objfile, objfile);

  /* Init bincl list */
  init_bincl_list (20);
  make_cleanup (free_bincl_list, 0);

  last_source_file = 0;

#ifdef END_OF_TEXT_DEFAULT
  end_of_text_addr = END_OF_TEXT_DEFAULT;
#else
  end_of_text_addr = text_addr + addr + text_size;	/* Relocate */
#endif

  symfile_bfd = objfile->obfd;	/* For next_text_symbol */
  abfd = objfile->obfd;
  symbuf_end = symbuf_idx = 0;

  for (symnum = 0; symnum < nlistlen; symnum++)
    {
      /* Get the symbol for this run and pull out some info */
      QUIT;	/* allow this to be interruptable */
      if (symbuf_idx == symbuf_end)
	fill_symbuf (abfd);
      bufp = &symbuf[symbuf_idx++];

      /*
       * Special case to speed up readin.
       */
      if (bufp->n_type == (unsigned char)N_SLINE) continue;

      SWAP_SYMBOL (bufp, abfd);

      /* Ok.  There is a lot of code duplicated in the rest of this
         switch statement (for efficiency reasons).  Since I don't
         like duplicating code, I will do my penance here, and
         describe the code which is duplicated:

	 *) The assignment to namestring.
	 *) The call to strchr.
	 *) The addition of a partial symbol the the two partial
	    symbol lists.  This last is a large section of code, so
	    I've imbedded it in the following macro.
	 */
      
/* Set namestring based on bufp.  If the string table index is invalid, 
   give a fake name, and print a single error message per symbol file read,
   rather than abort the symbol reading or flood the user with messages.  */
#define SET_NAMESTRING()\
  if (((unsigned)bufp->n_strx) >= stringtab_size) {	\
    complain (&string_table_offset_complaint, symnum);			\
    namestring = "foo";							\
  } else								\
    namestring = bufp->n_strx + stringtab

/* Add a symbol with an integer value to a psymtab. */
/* This is a macro unless we're debugging.  See above this function. */
#ifndef DEBUG
#  define ADD_PSYMBOL_TO_LIST(NAME, NAMELENGTH, NAMESPACE, CLASS, LIST, VALUE) \
 ADD_PSYMBOL_VT_TO_LIST(NAME, NAMELENGTH, NAMESPACE, CLASS, LIST, VALUE, \
                        SYMBOL_VALUE)
#endif /* DEBUG */

/* Add a symbol with a CORE_ADDR value to a psymtab. */
#define	ADD_PSYMBOL_ADDR_TO_LIST(NAME, NAMELENGTH, NAMESPACE, CLASS, LIST, VALUE) \
 ADD_PSYMBOL_VT_TO_LIST(NAME, NAMELENGTH, NAMESPACE, CLASS, LIST, VALUE, \
                        SYMBOL_VALUE_ADDRESS)

/* Add any kind of symbol to a psymtab. */
#define	ADD_PSYMBOL_VT_TO_LIST(NAME, NAMELENGTH, NAMESPACE, CLASS, LIST, VALUE, VT)\
  do {		        						\
    if ((LIST).next >=							\
	(LIST).list + (LIST).size)					\
      {									\
	(LIST).list = (struct partial_symbol *)				\
	  xrealloc ((LIST).list,					\
		    ((LIST).size * 2					\
		     * sizeof (struct partial_symbol)));		\
	/* Next assumes we only went one over.  Should be good if	\
	   program works correctly */					\
	(LIST).next =							\
	  (LIST).list + (LIST).size;					\
	(LIST).size *= 2;						\
      }									\
    psym = (LIST).next++;						\
									\
    SYMBOL_NAME (psym) = (char *) obstack_alloc (psymbol_obstack,	\
						 (NAMELENGTH) + 1);	\
    strncpy (SYMBOL_NAME (psym), (NAME), (NAMELENGTH));			\
    SYMBOL_NAME (psym)[(NAMELENGTH)] = '\0';				\
    SYMBOL_NAMESPACE (psym) = (NAMESPACE);				\
    SYMBOL_CLASS (psym) = (CLASS);					\
    VT (psym) = (VALUE); 						\
  } while (0);

/* End of macro definitions, now let's handle them symbols!  */

      switch (bufp->n_type)
	{
	  /*
	   * Standard, external, non-debugger, symbols
	   */

	case N_TEXT | N_EXT:
	case N_NBTEXT | N_EXT:
	case N_NBDATA | N_EXT:
	case N_NBBSS | N_EXT:
	case N_SETV | N_EXT:
	case N_ABS | N_EXT:
	case N_DATA | N_EXT:
	case N_BSS | N_EXT:

	  bufp->n_value += addr;		/* Relocate */

	  SET_NAMESTRING();

	bss_ext_symbol:
	  record_misc_function (namestring, bufp->n_value,
				bufp->n_type); /* Always */

	  continue;

	  /* Standard, local, non-debugger, symbols */

	case N_NBTEXT:

	  /* We need to be able to deal with both N_FN or N_TEXT,
	     because we have no way of knowing whether the sys-supplied ld
	     or GNU ld was used to make the executable.  Sequents throw
	     in another wrinkle -- they renumbered N_FN.  */
	case N_FN:
	case N_FN_SEQ:
	case N_TEXT:
	  bufp->n_value += addr;		/* Relocate */
	  SET_NAMESTRING();
	  if ((namestring[0] == '-' && namestring[1] == 'l')
	      || (namestring [(nsl = strlen (namestring)) - 1] == 'o'
		  && namestring [nsl - 2] == '.'))
	    {
	      if (entry_point < bufp->n_value
		  && entry_point >= last_o_file_start
		  && addr == 0)		/* FIXME nogood nomore */
		{
		  startup_file_start = last_o_file_start;
		  startup_file_end = bufp->n_value;
		}
	      if (past_first_source_file && pst
		  /* The gould NP1 uses low values for .o and -l symbols
		     which are not the address.  */
		  && bufp->n_value > pst->textlow)
		{
		  end_psymtab (pst, psymtab_include_list, includes_used,
			       symnum * symbol_size, bufp->n_value,
			       dependency_list, dependencies_used,
			       global_psymbols.next, static_psymbols.next);
		  pst = (struct partial_symtab *) 0;
		  includes_used = 0;
		  dependencies_used = 0;
		}
	      else
		past_first_source_file = 1;
	      last_o_file_start = bufp->n_value;
	    }
	  continue;

	case N_DATA:
	  bufp->n_value += addr;		/* Relocate */
	  SET_NAMESTRING ();
	  /* Check for __DYNAMIC, which is used by Sun shared libraries. 
	     Record it even if it's local, not global, so we can find it.
	     Same with virtual function tables, both global and static.  */
	  if ((namestring[8] == 'C' && (strcmp ("__DYNAMIC", namestring) == 0))
	      || VTBL_PREFIX_P ((namestring+HASH_OFFSET)))
	    {
	      /* Not really a function here, but... */
	      record_misc_function (namestring, bufp->n_value,
				    bufp->n_type); /* Always */
	  }
	  continue;

	case N_UNDF | N_EXT:
	  if (bufp->n_value != 0) {
	    /* This is a "Fortran COMMON" symbol.  See if the target
	       environment knows where it has been relocated to.  */

	    CORE_ADDR reladdr;

	    SET_NAMESTRING();
	    if (target_lookup_symbol (namestring, &reladdr)) {
	      continue;		/* Error in lookup; ignore symbol for now.  */
	    }
	    bufp->n_type ^= (N_BSS^N_UNDF);	/* Define it as a bss-symbol */
	    bufp->n_value = reladdr;
	    goto bss_ext_symbol;
	  }
	  continue;	/* Just undefined, not COMMON */

	    /* Lots of symbol types we can just ignore.  */

	case N_UNDF:
	case N_ABS:
	case N_BSS:
	case N_NBDATA:
	case N_NBBSS:
	  continue;

	  /* Keep going . . .*/

	  /*
	   * Special symbol types for GNU
	   */
	case N_INDR:
	case N_INDR | N_EXT:
	case N_SETA:
	case N_SETA | N_EXT:
	case N_SETT:
	case N_SETT | N_EXT:
	case N_SETD:
	case N_SETD | N_EXT:
	case N_SETB:
	case N_SETB | N_EXT:
	case N_SETV:
	  continue;

	  /*
	   * Debugger symbols
	   */

	case N_SO: {
	  unsigned long valu = bufp->n_value;
	  /* Symbol number of the first symbol of this file (i.e. the N_SO
	     if there is just one, or the first if we have a pair).  */
	  int first_symnum = symnum;
	  
	  /* End the current partial symtab and start a new one */

	  SET_NAMESTRING();

	  /* Peek at the next symbol.  If it is also an N_SO, the
	     first one just indicates the directory.  */
	  if (symbuf_idx == symbuf_end)
	    fill_symbuf (abfd);
	  bufp = &symbuf[symbuf_idx];
	  /* n_type is only a char, so swapping swapping is irrelevant.  */
	  if (bufp->n_type == (unsigned char)N_SO)
	    {
	      SWAP_SYMBOL (bufp, abfd);
	      SET_NAMESTRING ();
	      valu = bufp->n_value;
	      symbuf_idx++;
	      symnum++;
	    }
	  valu += addr;		/* Relocate */

	  if (pst && past_first_source_file)
	    {
	      end_psymtab (pst, psymtab_include_list, includes_used,
			   first_symnum * symbol_size, valu,
			   dependency_list, dependencies_used,
			   global_psymbols.next, static_psymbols.next);
	      pst = (struct partial_symtab *) 0;
	      includes_used = 0;
	      dependencies_used = 0;
	    }
	  else
	    past_first_source_file = 1;

	  pst = start_psymtab (objfile, addr,
			       namestring, valu,
			       first_symnum * symbol_size,
			       global_psymbols.next, static_psymbols.next);
	  continue;
	}

	case N_BINCL:
	  /* Add this bincl to the bincl_list for future EXCLs.  No
	     need to save the string; it'll be around until
	     read_dbx_symtab function returns */

	  SET_NAMESTRING();

	  add_bincl_to_list (pst, namestring, bufp->n_value);

	  /* Mark down an include file in the current psymtab */

	  psymtab_include_list[includes_used++] = namestring;
	  if (includes_used >= includes_allocated)
	    {
	      char **orig = psymtab_include_list;

	      psymtab_include_list = (char **)
		alloca ((includes_allocated *= 2) *
			sizeof (char *));
	      bcopy (orig, psymtab_include_list,
		     includes_used * sizeof (char *));
	    }

	  continue;

	case N_SOL:
	  /* Mark down an include file in the current psymtab */

	  SET_NAMESTRING();

	  /* In C++, one may expect the same filename to come round many
	     times, when code is coming alternately from the main file
	     and from inline functions in other files. So I check to see
	     if this is a file we've seen before -- either the main
	     source file, or a previously included file.

	     This seems to be a lot of time to be spending on N_SOL, but
	     things like "break c-exp.y:435" need to work (I
	     suppose the psymtab_include_list could be hashed or put
	     in a binary tree, if profiling shows this is a major hog).  */
	  if (pst && !strcmp (namestring, pst->filename))
	    continue;
	  {
	    register int i;
	    for (i = 0; i < includes_used; i++)
	      if (!strcmp (namestring, psymtab_include_list[i]))
		{
		  i = -1; 
		  break;
		}
	    if (i == -1)
	      continue;
	  }

	  psymtab_include_list[includes_used++] = namestring;
	  if (includes_used >= includes_allocated)
	    {
	      char **orig = psymtab_include_list;

	      psymtab_include_list = (char **)
		alloca ((includes_allocated *= 2) *
			sizeof (char *));
	      bcopy (orig, psymtab_include_list,
		     includes_used * sizeof (char *));
	    }
	  continue;

	case N_LSYM:		/* Typedef or automatic variable. */
	case N_STSYM:		/* Data seg var -- static  */
	case N_LCSYM:		/* BSS      "  */
	case N_NBSTS:           /* Gould nobase.  */
	case N_NBLCS:           /* symbols.  */

	  SET_NAMESTRING();

	  p = (char *) strchr (namestring, ':');

	  /* Skip if there is no :.  */
	  if (!p) continue;

	  switch (p[1])
	    {
	    case 'T':
	      ADD_PSYMBOL_TO_LIST (namestring, p - namestring,
				   STRUCT_NAMESPACE, LOC_TYPEDEF,
				   static_psymbols, bufp->n_value);
	      if (p[2] == 't')
		{
		  /* Also a typedef with the same name.  */
		  ADD_PSYMBOL_TO_LIST (namestring, p - namestring,
				       VAR_NAMESPACE, LOC_TYPEDEF,
				       static_psymbols, bufp->n_value);
		  p += 1;
		}
	      goto check_enum;
	    case 't':
	      ADD_PSYMBOL_TO_LIST (namestring, p - namestring,
				   VAR_NAMESPACE, LOC_TYPEDEF,
				   static_psymbols, bufp->n_value);
	    check_enum:
	      /* If this is an enumerated type, we need to
		 add all the enum constants to the partial symbol
		 table.  This does not cover enums without names, e.g.
		 "enum {a, b} c;" in C, but fortunately those are
		 rare.  There is no way for GDB to find those from the
		 enum type without spending too much time on it.  Thus
		 to solve this problem, the compiler needs to put out separate
		 constant symbols ('c' N_LSYMS) for enum constants in
		 enums without names, or put out a dummy type.  */

	      /* We are looking for something of the form
		 <name> ":" ("t" | "T") [<number> "="] "e"
		 {<constant> ":" <value> ","} ";".  */

	      /* Skip over the colon and the 't' or 'T'.  */
	      p += 2;
	      /* This type may be given a number.  Also, numbers can come
		 in pairs like (0,26).  Skip over it.  */
	      while ((*p >= '0' && *p <= '9')
		     || *p == '(' || *p == ',' || *p == ')'
		     || *p == '=')
		p++;

	      if (*p++ == 'e')
		{
		  /* We have found an enumerated type.  */
		  /* According to comments in read_enum_type
		     a comma could end it instead of a semicolon.
		     I don't know where that happens.
		     Accept either.  */
		  while (*p && *p != ';' && *p != ',')
		    {
		      char *q;

		      /* Check for and handle cretinous dbx symbol name
			 continuation!  */
		      if (*p == '\\')
			p = next_symbol_text ();

		      /* Point to the character after the name
			 of the enum constant.  */
		      for (q = p; *q && *q != ':'; q++)
			;
		      /* Note that the value doesn't matter for
			 enum constants in psymtabs, just in symtabs.  */
		      ADD_PSYMBOL_TO_LIST (p, q - p,
					   VAR_NAMESPACE, LOC_CONST,
					   static_psymbols, 0);
		      /* Point past the name.  */
		      p = q;
		      /* Skip over the value.  */
		      while (*p && *p != ',')
			p++;
		      /* Advance past the comma.  */
		      if (*p)
			p++;
		    }
		}

	      continue;
	    case 'c':
	      /* Constant, e.g. from "const" in Pascal.  */
	      ADD_PSYMBOL_TO_LIST (namestring, p - namestring,
				   VAR_NAMESPACE, LOC_CONST,
				   static_psymbols, bufp->n_value);
	      continue;
	    default:
	      /* Skip if the thing following the : is
	         not a letter (which indicates declaration of a local
	         variable, which we aren't interested in).  */
	      continue;
	    }

	case N_FUN:
	case N_GSYM:		/* Global (extern) variable; can be
				   data or bss (sigh).  */

	/* Following may probably be ignored; I'll leave them here
	   for now (until I do Pascal and Modula 2 extensions).  */

	case N_PC:		/* I may or may not need this; I
				   suspect not.  */
	case N_M2C:		/* I suspect that I can ignore this here. */
	case N_SCOPE:		/* Same.   */

	  SET_NAMESTRING();

	  p = (char *) strchr (namestring, ':');
	  if (!p)
	    continue;		/* Not a debugging symbol.   */



	  /* Main processing section for debugging symbols which
	     the initial read through the symbol tables needs to worry
	     about.  If we reach this point, the symbol which we are
	     considering is definitely one we are interested in.
	     p must also contain the (valid) index into the namestring
	     which indicates the debugging type symbol.  */

	  switch (p[1])
	    {
	    case 'c':
	      ADD_PSYMBOL_TO_LIST (namestring, p - namestring,
				   VAR_NAMESPACE, LOC_CONST,
				   static_psymbols, bufp->n_value);
	      continue;
	    case 'S':
	      bufp->n_value += addr;		/* Relocate */
	      ADD_PSYMBOL_ADDR_TO_LIST (namestring, p - namestring,
				   VAR_NAMESPACE, LOC_STATIC,
				   static_psymbols, bufp->n_value);
	      continue;
	    case 'G':
	      bufp->n_value += addr;		/* Relocate */
	      /* The addresses in these entries are reported to be
		 wrong.  See the code that reads 'G's for symtabs. */
	      ADD_PSYMBOL_ADDR_TO_LIST (namestring, p - namestring,
				   VAR_NAMESPACE, LOC_STATIC,
				   global_psymbols, bufp->n_value);
	      continue;

	    case 't':
	      ADD_PSYMBOL_TO_LIST (namestring, p - namestring,
				   VAR_NAMESPACE, LOC_TYPEDEF,
				   static_psymbols, bufp->n_value);
	      continue;

	    case 'f':
	      ADD_PSYMBOL_TO_LIST (namestring, p - namestring,
				   VAR_NAMESPACE, LOC_BLOCK,
				   static_psymbols, bufp->n_value);
	      continue;

	      /* Global functions were ignored here, but now they
	         are put into the global psymtab like one would expect.
		 They're also in the misc fn vector... 
		 FIXME, why did it used to ignore these?  That broke
		 "i fun" on these functions.  */
	    case 'F':
	      ADD_PSYMBOL_TO_LIST (namestring, p - namestring,
				   VAR_NAMESPACE, LOC_BLOCK,
				   global_psymbols, bufp->n_value);
	      continue;

	      /* Two things show up here (hopefully); static symbols of
		 local scope (static used inside braces) or extensions
		 of structure symbols.  We can ignore both.  */
	    case 'V':
	    case '(':
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
	      continue;

	    default:
	      /* Unexpected symbol.  Ignore it; perhaps it is an extension
		 that we don't know about.

		 Someone says sun cc puts out symbols like
		 /foo/baz/maclib::/usr/local/bin/maclib,
		 which would get here with a symbol type of ':'.  */
	      continue;
	    }

	case N_EXCL:

	  SET_NAMESTRING();

	  /* Find the corresponding bincl and mark that psymtab on the
	     psymtab dependency list */
	  {
	    struct partial_symtab *needed_pst =
	      find_corresponding_bincl_psymtab (namestring, bufp->n_value);

	    /* If this include file was defined earlier in this file,
	       leave it alone.  */
	    if (needed_pst == pst) continue;

	    if (needed_pst)
	      {
		int i;
		int found = 0;

		for (i = 0; i < dependencies_used; i++)
		  if (dependency_list[i] == needed_pst)
		    {
		      found = 1;
		      break;
		    }

		/* If it's already in the list, skip the rest.  */
		if (found) continue;

		dependency_list[dependencies_used++] = needed_pst;
		if (dependencies_used >= dependencies_allocated)
		  {
		    struct partial_symtab **orig = dependency_list;
		    dependency_list =
		      (struct partial_symtab **)
			alloca ((dependencies_allocated *= 2)
				* sizeof (struct partial_symtab *));
		    bcopy (orig, dependency_list,
			   (dependencies_used
			    * sizeof (struct partial_symtab *)));
#ifdef DEBUG_INFO
		    fprintf (stderr, "Had to reallocate dependency list.\n");
		    fprintf (stderr, "New dependencies allocated: %d\n",
			     dependencies_allocated);
#endif
		  }
	      }
	    else
	      error ("Invalid symbol data: \"repeated\" header file not previously seen, at symtab pos %d.",
		     symnum);
	  }
	  continue;

	case N_EINCL:
	case N_DSLINE:
	case N_BSLINE:
	case N_SSYM:		/* Claim: Structure or union element.
				   Hopefully, I can ignore this.  */
	case N_ENTRY:		/* Alternate entry point; can ignore. */
	case N_MAIN:		/* Can definitely ignore this.   */
	case N_CATCH:		/* These are GNU C++ extensions */
	case N_EHDECL:		/* that can safely be ignored here. */
	case N_LENG:
	case N_BCOMM:
	case N_ECOMM:
	case N_ECOML:
	case N_FNAME:
	case N_SLINE:
	case N_RSYM:
	case N_PSYM:
	case N_LBRAC:
	case N_RBRAC:
	case N_NSYMS:		/* Ultrix 4.0: symbol count */
	case N_DEFD:		/* GNU Modula-2 */
	  /* These symbols aren't interesting; don't worry about them */

	  continue;

	default:
	  /* If we haven't found it yet, ignore it.  It's probably some
	     new type we don't know about yet.  */
	  complain (&unknown_symtype_complaint, local_hex_string(bufp->n_type));
	  continue;
	}
    }

  /* If there's stuff to be cleaned up, clean it up.  */
  if (nlistlen > 0				/* We have some syms */
      && entry_point < bufp->n_value
      && entry_point >= last_o_file_start)
    {
      startup_file_start = last_o_file_start;
      startup_file_end = bufp->n_value;
    }

  if (pst)
    {
      end_psymtab (pst, psymtab_include_list, includes_used,
		   symnum * symbol_size, end_of_text_addr,
		   dependency_list, dependencies_used,
		   global_psymbols.next, static_psymbols.next);
      includes_used = 0;
      dependencies_used = 0;
      pst = (struct partial_symtab *) 0;
    }

  free_bincl_list ();
  discard_cleanups (old_chain);
}

/* Allocate and partially fill a partial symtab.  It will be
   completely filled at the end of the symbol list.

   SYMFILE_NAME is the name of the symbol-file we are reading from, and ADDR
   is the address relative to which its symbols are (incremental) or 0
   (normal). */


static struct partial_symtab *
start_psymtab (objfile, addr,
	       filename, textlow, ldsymoff, global_syms, static_syms)
     struct objfile *objfile;
     CORE_ADDR addr;
     char *filename;
     CORE_ADDR textlow;
     int ldsymoff;
     struct partial_symbol *global_syms;
     struct partial_symbol *static_syms;
{
  struct partial_symtab *result =
    (struct partial_symtab *) obstack_alloc (psymbol_obstack,
					     sizeof (struct partial_symtab));

  result->addr = addr;

  result->filename =
    (char *) obstack_alloc (psymbol_obstack,
			    strlen (filename) + 1);
  strcpy (result->filename, filename);

  result->textlow = textlow;
  result->read_symtab_private = (char *) obstack_alloc (psymbol_obstack,
					       sizeof (struct symloc));
  LDSYMOFF(result) = ldsymoff;

  result->readin = 0;
  result->symtab = 0;
  result->read_symtab = dbx_psymtab_to_symtab;

  result->globals_offset = global_syms - global_psymbols.list;
  result->statics_offset = static_syms - static_psymbols.list;

  result->n_global_syms = 0;
  result->n_static_syms = 0;

  /* Chain it to the list owned by the current object file.  */
  result->objfile = objfile;
  result->objfile_chain = objfile->psymtabs;
  objfile->psymtabs = result;

  return result;
}

static int
compare_psymbols (s1, s2)
     register struct partial_symbol *s1, *s2;
{
  register char
    *st1 = SYMBOL_NAME (s1),
    *st2 = SYMBOL_NAME (s2);

  if (st1[0] - st2[0])
    return st1[0] - st2[0];
  if (st1[1] - st2[1])
    return st1[1] - st2[1];
  return strcmp (st1 + 1, st2 + 1);
}


/* Close off the current usage of a partial_symbol table entry.  This
   involves setting the correct number of includes (with a realloc),
   setting the high text mark, setting the symbol length in the
   executable, and setting the length of the global and static lists
   of psymbols.

   The global symbols and static symbols are then seperately sorted.

   Then the partial symtab is put on the global list.
   *** List variables and peculiarities of same. ***
   */
static void
end_psymtab (pst, include_list, num_includes, capping_symbol_offset,
	     capping_text, dependency_list, number_dependencies,
	     capping_global, capping_static)
     struct partial_symtab *pst;
     char **include_list;
     int num_includes;
     int capping_symbol_offset;
     CORE_ADDR capping_text;
     struct partial_symtab **dependency_list;
     int number_dependencies;
     struct partial_symbol *capping_global, *capping_static;
{
  int i;

  LDSYMLEN(pst) = capping_symbol_offset - LDSYMOFF(pst);
  pst->texthigh = capping_text;

  pst->n_global_syms =
    capping_global - (global_psymbols.list + pst->globals_offset);
  pst->n_static_syms =
    capping_static - (static_psymbols.list + pst->statics_offset);

  pst->number_of_dependencies = number_dependencies;
  if (number_dependencies)
    {
      pst->dependencies = (struct partial_symtab **)
	obstack_alloc (psymbol_obstack,
		       number_dependencies * sizeof (struct partial_symtab *));
      bcopy (dependency_list, pst->dependencies,
	     number_dependencies * sizeof (struct partial_symtab *));
    }
  else
    pst->dependencies = 0;

  for (i = 0; i < num_includes; i++)
    {
      struct partial_symtab *subpst =
	(struct partial_symtab *)
	  obstack_alloc (psymbol_obstack,
			 sizeof (struct partial_symtab));

      subpst->filename =
	(char *) obstack_alloc (psymbol_obstack,
				strlen (include_list[i]) + 1);
      strcpy (subpst->filename, include_list[i]);

      /* Chain it to the list that this object file owns.  */
      subpst->objfile = pst->objfile;
      subpst->objfile_chain = pst->objfile->psymtabs;
      pst->objfile->psymtabs = subpst;

      subpst->addr = pst->addr;
      subpst->read_symtab_private = (char *) obstack_alloc (psymbol_obstack,
						   sizeof (struct symloc));
      LDSYMOFF(subpst) =
	LDSYMLEN(subpst) =
	  subpst->textlow =
	    subpst->texthigh = 0;

      /* We could save slight bits of space by only making one of these,
	 shared by the entire set of include files.  FIXME-someday.  */
      subpst->dependencies = (struct partial_symtab **)
	obstack_alloc (psymbol_obstack,
		       sizeof (struct partial_symtab *));
      subpst->dependencies[0] = pst;
      subpst->number_of_dependencies = 1;

      subpst->globals_offset =
	subpst->n_global_syms =
	  subpst->statics_offset =
	    subpst->n_static_syms = 0;

      subpst->readin = 0;
      subpst->symtab = 0;
      subpst->read_symtab = dbx_psymtab_to_symtab;

      subpst->next = partial_symtab_list;
      partial_symtab_list = subpst;
    }

  /* Sort the global list; don't sort the static list */
  qsort (global_psymbols.list + pst->globals_offset, pst->n_global_syms,
	 sizeof (struct partial_symbol), compare_psymbols);

  /* If there is already a psymtab or symtab for a file of this name, remove it.
     (If there is a symtab, more drastic things also happen.)
     This happens in VxWorks.  */
  free_named_symtabs (pst->filename);

  if (num_includes == 0
   && number_dependencies == 0
   && pst->n_global_syms == 0
   && pst->n_static_syms == 0) {
    /* Throw away this psymtab, it's empty.  We can't deallocate it, since
       it is on the obstack, but we can forget to chain it on the list.  */
    ;
  } else {
    /* Put the psymtab on the psymtab list */
    pst->next = partial_symtab_list;
    partial_symtab_list = pst;
  }
}

static void
psymtab_to_symtab_1 (pst, stringtab, stringtab_size, sym_offset)
     struct partial_symtab *pst;
     char *stringtab;
     int stringtab_size;
     int sym_offset;
{
  struct cleanup *old_chain;
  int i;
  
  if (!pst)
    return;

  if (pst->readin)
    {
      fprintf (stderr, "Psymtab for %s already read in.  Shouldn't happen.\n",
	       pst->filename);
      return;
    }

  /* Read in all partial symtabs on which this one is dependent */
  for (i = 0; i < pst->number_of_dependencies; i++)
    if (!pst->dependencies[i]->readin)
      {
	/* Inform about additional files that need to be read in.  */
	if (info_verbose)
	  {
	    fputs_filtered (" ", stdout);
	    wrap_here ("");
	    fputs_filtered ("and ", stdout);
	    wrap_here ("");
	    printf_filtered ("%s...", pst->dependencies[i]->filename);
	    wrap_here ("");		/* Flush output */
	    fflush (stdout);
	  }
	psymtab_to_symtab_1 (pst->dependencies[i],
			     stringtab, stringtab_size, sym_offset);
      }

  if (LDSYMLEN(pst))		/* Otherwise it's a dummy */
    {
      /* Init stuff necessary for reading in symbols */
      buildsym_init ();
      old_chain = make_cleanup (really_free_pendings, 0);

      /* Read in this files symbols */
      bfd_seek (pst->objfile->obfd, sym_offset, L_SET);
      pst->symtab =
	read_ofile_symtab (pst->objfile, stringtab, stringtab_size,
			   LDSYMOFF(pst),
			   LDSYMLEN(pst), pst->textlow,
			   pst->texthigh - pst->textlow, pst->addr);
      sort_symtab_syms (pst->symtab);

      do_cleanups (old_chain);
    }

  pst->readin = 1;
}

/*
 * Read in all of the symbols for a given psymtab for real.
 * Be verbose about it if the user wants that.
 */
static void
dbx_psymtab_to_symtab (pst)
     struct partial_symtab *pst;
{
  char *stringtab;
  int stsize, val;
  bfd *sym_bfd;
  long st_temp;

  if (!pst)
    return;

  if (pst->readin)
    {
      fprintf (stderr, "Psymtab for %s already read in.  Shouldn't happen.\n",
	       pst->filename);
      return;
    }

  if (LDSYMLEN(pst) || pst->number_of_dependencies)
    {
      /* Print the message now, before reading the string table,
	 to avoid disconcerting pauses.  */
      if (info_verbose)
	{
	  printf_filtered ("Reading in symbols for %s...", pst->filename);
	  fflush (stdout);
	}

      sym_bfd = pst->objfile->obfd;

      /* We keep the string table for the main symfile resident in memory, but
	 not the string table for any other symbol files.  */
      if (symfile_objfile != pst->objfile)
	{
	  /* Read in the string table */

	  /* FIXME, this uses internal BFD variables.  See above in
	     dbx_symbol_file_open where the macro is defined!  */
	  bfd_seek (sym_bfd, STRING_TABLE_OFFSET, L_SET);

	  val = bfd_read (&st_temp, sizeof st_temp, 1, sym_bfd);
	  if (val < 0)
	      perror_with_name (pst->objfile->name);
	  stsize = bfd_h_get_32 (sym_bfd, (unsigned char *)&st_temp);
#if 0
	  /* BFD doesn't provide a way to know the total file size, sigh */
          struct stat statbuf;
	  if (fstat (desc, &statbuf) < 0)
	    perror_with_name (pst->objfile->name);
	  
	  if (stsize >= 0 && stsize < statbuf.st_size)
#else
	  if (stsize >= 0)
#endif
	    {
#ifdef BROKEN_LARGE_ALLOCA
	      stringtab = (char *) xmalloc (stsize);
	      make_cleanup (free, stringtab);
#else
	      stringtab = (char *) alloca (stsize);
#endif
	    }
	  else
	    stringtab = NULL;
	  if (stringtab == NULL && stsize != 0)
	    error ("ridiculous string table size: %d bytes", stsize);

	  /* FIXME, this uses internal BFD variables.  See above in
	     dbx_symbol_file_open where the macro is defined!  */
	  val = bfd_seek (sym_bfd, STRING_TABLE_OFFSET, L_SET);
	  if (val < 0)
	    perror_with_name (pst->objfile->name);
	  val = bfd_read (stringtab, stsize, 1, sym_bfd);
	  if (val < 0)
	    perror_with_name (pst->objfile->name);
	}
      else
	{
	  stringtab = symfile_string_table;
	  stsize = symfile_string_table_size;
	}

      /* FIXME POKING INSIDE BFD DATA STRUCTURES */
      symbol_size = obj_symbol_entry_size (sym_bfd);

      /* FIXME, this uses internal BFD variables.  See above in
	 dbx_symbol_file_open where the macro is defined!  */
      psymtab_to_symtab_1 (pst, stringtab, stsize,
			   SYMBOL_TABLE_OFFSET);

      /* Match with global symbols.  This only needs to be done once,
         after all of the symtabs and dependencies have been read in.   */
      scan_file_globals ();

      /* Finish up the debug error message.  */
      if (info_verbose)
	printf_filtered ("done.\n");
    }
}

/*
 * Read in a defined section of a specific object file's symbols.
 *
 * DESC is the file descriptor for the file, positioned at the
 * beginning of the symtab
 * STRINGTAB is a pointer to the files string
 * table, already read in
 * SYM_OFFSET is the offset within the file of
 * the beginning of the symbols we want to read, NUM_SUMBOLS is the
 * number of symbols to read
 * TEXT_OFFSET is the beginning of the text segment we are reading symbols for
 * TEXT_SIZE is the size of the text segment read in.
 * OFFSET is a relocation offset which gets added to each symbol
 */

static struct symtab *
read_ofile_symtab (objfile, stringtab, stringtab_size, sym_offset,
		   sym_size, text_offset, text_size, offset)
     struct objfile *objfile;
     register char *stringtab;
     unsigned int stringtab_size;
     int sym_offset;
     int sym_size;
     CORE_ADDR text_offset;
     int text_size;
     int offset;
{
  register char *namestring;
  register struct internal_nlist *bufp;
  unsigned char type;
  unsigned max_symnum;
  register bfd *abfd;

  subfile_stack = 0;

  stringtab_global = stringtab;
  last_source_file = 0;

  abfd = objfile->obfd;
  symfile_bfd = objfile->obfd;	/* Implicit param to next_text_symbol */
  our_objfile = objfile;  /* For end_symtab calls in process_one_symbol */
  symbuf_end = symbuf_idx = 0;

  /* It is necessary to actually read one symbol *before* the start
     of this symtab's symbols, because the GCC_COMPILED_FLAG_SYMBOL
     occurs before the N_SO symbol.

     Detecting this in read_dbx_symtab
     would slow down initial readin, so we look for it here instead.  */
  if (sym_offset >= (int)symbol_size)
    {
      bfd_seek (symfile_bfd, sym_offset - symbol_size, L_INCR);
      fill_symbuf (abfd);
      bufp = &symbuf[symbuf_idx++];
      SWAP_SYMBOL (bufp, abfd);

      SET_NAMESTRING ();

      processing_gcc_compilation =
	(bufp->n_type == N_TEXT
	 && !strcmp (namestring, GCC_COMPILED_FLAG_SYMBOL));
      /* FIXME!!!  Check for gcc2_compiled... */
    }
  else
    {
      /* The N_SO starting this symtab is the first symbol, so we
	 better not check the symbol before it.  I'm not this can
	 happen, but it doesn't hurt to check for it.  */
      bfd_seek (symfile_bfd, sym_offset, L_INCR);
      processing_gcc_compilation = 0;
    }

  if (symbuf_idx == symbuf_end)
    fill_symbuf (abfd);
  bufp = &symbuf[symbuf_idx];
  if (bufp->n_type != (unsigned char)N_SO)
    error("First symbol in segment of executable not a source symbol");

  max_symnum = sym_size / symbol_size;

  for (symnum = 0;
       symnum < max_symnum;
       symnum++)
    {
      QUIT;			/* Allow this to be interruptable */
      if (symbuf_idx == symbuf_end)
	fill_symbuf(abfd);
      bufp = &symbuf[symbuf_idx++];
      SWAP_SYMBOL (bufp, abfd);

      type = bufp->n_type;
      if (type == (unsigned char)N_CATCH)
	{
	  /* N_CATCH is not fixed up by the linker, and unfortunately,
	     there's no other place to put it in the .stab map.  */
	  bufp->n_value += text_offset + offset;
	}
      else {
        type &= ~N_EXT;		/* Ignore external-bit */
        if (type == N_TEXT || type == N_DATA || type == N_BSS)
	  bufp->n_value += offset;
        type = bufp->n_type;
      }

      SET_NAMESTRING ();

      if (type & N_STAB) {
	/* Check for a pair of N_SO symbols, which give both a new
	   source file name (second) and its directory (first).  */
	if (type == (unsigned char)N_SO) {
	  /* Save the outer values */
	  short bufp_n_desc = bufp->n_desc;
	  unsigned long valu = bufp->n_value;

	  if (symbuf_idx == symbuf_end)
	    fill_symbuf (abfd);
	  bufp = &symbuf[symbuf_idx];
	  if (bufp->n_type == (unsigned char)N_SO) {
	    char *namestring1 = namestring;

	    SWAP_SYMBOL (bufp, abfd);
	    bufp->n_value += offset;		/* Relocate */
	    symbuf_idx++;
	    symnum++;
	    SET_NAMESTRING ();

	    /* No need to check PCC_SOL_BROKEN, on the assumption that
	       such broken PCC's don't put out N_SO pairs.  */
	    if (last_source_file)
	      (void)end_symtab (bufp->n_value, 0, 0, objfile);
	    start_symtab (namestring, namestring1, bufp->n_value);
	  } else {
	    /* N_SO without a following N_SO */
	    process_one_symbol(type, bufp_n_desc, valu, namestring);
	    /* our_objfile is an implicit parameter.  */
	  }
	} else {

	  /* Ordinary symbol

		HERE IS WHERE THE REAL WORK GETS DONE!
							  */
	  process_one_symbol (type, bufp->n_desc, bufp->n_value,
			      namestring);
	  /* our_objfile is an implicit parameter.  */

	}
      }
      /* We skip checking for a new .o or -l file; that should never
         happen in this routine. */
      else if (type == N_TEXT
	       && !strcmp (namestring, GCC_COMPILED_FLAG_SYMBOL))
	/* I don't think this code will ever be executed, because
	   the GCC_COMPILED_FLAG_SYMBOL usually is right before
	   the N_SO symbol which starts this source file.
	   However, there is no reason not to accept
	   the GCC_COMPILED_FLAG_SYMBOL anywhere.  */
	processing_gcc_compilation = 1;
      else if (type & N_EXT || type == (unsigned char)N_TEXT
	       || type == (unsigned char)N_NBTEXT
	       ) {
	  /* Global symbol: see if we came across a dbx defintion for
	     a corresponding symbol.  If so, store the value.  Remove
	     syms from the chain when their values are stored, but
	     search the whole chain, as there may be several syms from
	     different files with the same name. */
	  /* This is probably not true.  Since the files will be read
	     in one at a time, each reference to a global symbol will
	     be satisfied in each file as it appears. So we skip this
	     section. */
	  ;
        }
    }

  return end_symtab (text_offset + text_size, 0, 0, objfile);
}

int
hashname (name)
     char *name;
{
  register char *p = name;
  register int total = p[0];
  register int c;

  c = p[1];
  total += c << 2;
  if (c)
    {
      c = p[2];
      total += c << 4;
      if (c)
	total += p[3] << 6;
    }

  /* Ensure result is positive.  */
  if (total < 0) total += (1000 << 6);
  return total % HASHSIZE;
}


static void
process_one_symbol (type, desc, valu, name)
     int type, desc;
     CORE_ADDR valu;
     char *name;
{
#ifndef SUN_FIXED_LBRAC_BUG
  /* This records the last pc address we've seen.  We depend on their being
     an SLINE or FUN or SO before the first LBRAC, since the variable does
     not get reset in between reads of different symbol files.  */
  static CORE_ADDR last_pc_address;
#endif
  register struct context_stack *new;
  char *colon_pos;

  /* Something is wrong if we see real data before
     seeing a source file name.  */

  if (last_source_file == 0 && type != (unsigned char)N_SO)
    {
      /* Currently this ignores N_ENTRY on Gould machines, N_NSYM on machines
	 where that code is defined.  */
      if (IGNORE_SYMBOL (type))
	return;

      /* FIXME, this should not be an error, since it precludes extending
         the symbol table information in this way...  */
      error ("Invalid symbol data: does not start by identifying a source file.");
    }

  switch (type)
    {
    case N_FUN:
    case N_FNAME:
      /* Either of these types of symbols indicates the start of
	 a new function.  We must process its "name" normally for dbx,
	 but also record the start of a new lexical context, and possibly
	 also the end of the lexical context for the previous function.  */
      /* This is not always true.  This type of symbol may indicate a
         text segment variable.  */

#ifndef SUN_FIXED_LBRAC_BUG
      last_pc_address = valu;	/* Save for SunOS bug circumcision */
#endif

      colon_pos = strchr (name, ':');
      if (!colon_pos++
	  || (*colon_pos != 'f' && *colon_pos != 'F'))
	{
	  define_symbol (valu, name, desc, type);
	  break;
	}

      within_function = 1;
      if (context_stack_depth > 0)
	{
	  new = pop_context ();
	  /* Make a block for the local symbols within.  */
	  finish_block (new->name, &local_symbols, new->old_blocks,
			new->start_addr, valu);
	}
      /* Stack must be empty now.  */
      if (context_stack_depth != 0)
	complain (lbrac_unmatched_complaint, symnum);

      new = push_context (0, valu);
      new->name = define_symbol (valu, name, desc, type);
      break;

    case N_CATCH:
      /* Record the address at which this catch takes place.  */
      define_symbol (valu, name, desc, type);
      break;

    case N_EHDECL:
      /* Don't know what to do with these yet.  */
      error ("action uncertain for eh extensions");
      break;

    case N_LBRAC:
      /* This "symbol" just indicates the start of an inner lexical
	 context within a function.  */

#if !defined (BLOCK_ADDRESS_ABSOLUTE)
      /* On most machines, the block addresses are relative to the
	 N_SO, the linker did not relocate them (sigh).  */
      valu += last_source_start_addr;
#endif

#ifndef SUN_FIXED_LBRAC_BUG
      if (valu < last_pc_address) {
	/* Patch current LBRAC pc value to match last handy pc value */
 	complain (&lbrac_complaint, 0);
	valu = last_pc_address;
      }
#endif
      new = push_context (desc, valu);
      break;

    case N_RBRAC:
      /* This "symbol" just indicates the end of an inner lexical
	 context that was started with N_LBRAC.  */

#if !defined (BLOCK_ADDRESS_ABSOLUTE)
      /* On most machines, the block addresses are relative to the
	 N_SO, the linker did not relocate them (sigh).  */
      valu += last_source_start_addr;
#endif

      new = pop_context();
      if (desc != new->depth)
	complain (lbrac_mismatch_complaint, symnum);

      /* Some compilers put the variable decls inside of an
         LBRAC/RBRAC block.  This macro should be nonzero if this
	 is true.  DESC is N_DESC from the N_RBRAC symbol.
	 GCC_P is true if we've detected the GCC_COMPILED_SYMBOL.  */
#if !defined (VARIABLES_INSIDE_BLOCK)
#define VARIABLES_INSIDE_BLOCK(desc, gcc_p) 0
#endif

      /* Can only use new->locals as local symbols here if we're in
         gcc or on a machine that puts them before the lbrack.  */
      if (!VARIABLES_INSIDE_BLOCK(desc, processing_gcc_compilation))
	local_symbols = new->locals;

      /* If this is not the outermost LBRAC...RBRAC pair in the
	 function, its local symbols preceded it, and are the ones
	 just recovered from the context stack.  Defined the block for them.

	 If this is the outermost LBRAC...RBRAC pair, there is no
	 need to do anything; leave the symbols that preceded it
	 to be attached to the function's own block.  However, if
	 it is so, we need to indicate that we just moved outside
	 of the function.  */
      if (local_symbols
	  && (context_stack_depth
	      > !VARIABLES_INSIDE_BLOCK(desc, processing_gcc_compilation)))
	{
	  /* FIXME Muzzle a compiler bug that makes end < start.  */
	  if (new->start_addr > valu)
	    {
	      complain(&lbrac_rbrac_complaint, 0);
	      new->start_addr = valu;
	    }
	  /* Make a block for the local symbols within.  */
	  finish_block (0, &local_symbols, new->old_blocks,
			new->start_addr, valu);
	}
      else
	{
	  within_function = 0;
	}
      if (VARIABLES_INSIDE_BLOCK(desc, processing_gcc_compilation))
	/* Now pop locals of block just finished.  */
	local_symbols = new->locals;
      break;

    case N_FN:
    case N_FN_SEQ:
      /* This kind of symbol indicates the start of an object file.  */
      break;

    case N_SO:
      /* This type of symbol indicates the start of data
	 for one source file.
	 Finish the symbol table of the previous source file
	 (if any) and start accumulating a new symbol table.  */
#ifndef SUN_FIXED_LBRAC_BUG
      last_pc_address = valu;	/* Save for SunOS bug circumcision */
#endif
  
#ifdef PCC_SOL_BROKEN
      /* pcc bug, occasionally puts out SO for SOL.  */
      if (context_stack_depth > 0)
	{
	  start_subfile (name, NULL);
	  break;
	}
#endif
      if (last_source_file)
	(void)end_symtab (valu, 0, 0);
      start_symtab (name, NULL, valu);
      break;

    case N_SOL:
      /* This type of symbol indicates the start of data for
	 a sub-source-file, one whose contents were copied or
	 included in the compilation of the main source file
	 (whose name was given in the N_SO symbol.)  */
      start_subfile (name, NULL);
      break;

    case N_BINCL:
      push_subfile ();
      add_new_header_file (name, valu);
      start_subfile (name, NULL);
      break;

    case N_EINCL:
      start_subfile (pop_subfile (), NULL);
      break;

    case N_EXCL:
      add_old_header_file (name, valu);
      break;

    case N_SLINE:
      /* This type of "symbol" really just records
	 one line-number -- core-address correspondence.
	 Enter it in the line list for this symbol table.  */
#ifndef SUN_FIXED_LBRAC_BUG
      last_pc_address = valu;	/* Save for SunOS bug circumcision */
#endif
      record_line (current_subfile, desc, valu);
      break;

    case N_BCOMM:
      if (common_block)
	error ("Invalid symbol data: common within common at symtab pos %d",
	       symnum);
      common_block = local_symbols;
      common_block_i = local_symbols ? local_symbols->nsyms : 0;
      break;

    case N_ECOMM:
      /* Symbols declared since the BCOMM are to have the common block
	 start address added in when we know it.  common_block points to
	 the first symbol after the BCOMM in the local_symbols list;
	 copy the list and hang it off the symbol for the common block name
	 for later fixup.  */
      {
	int i;
	struct symbol *sym =
	  (struct symbol *) xmalloc (sizeof (struct symbol));
	bzero (sym, sizeof *sym);
	SYMBOL_NAME (sym) = savestring (name, strlen (name));
	SYMBOL_CLASS (sym) = LOC_BLOCK;
	SYMBOL_NAMESPACE (sym) = (enum namespace)((long)
	  copy_pending (local_symbols, common_block_i, common_block));
	i = hashname (SYMBOL_NAME (sym));
	SYMBOL_VALUE_CHAIN (sym) = global_sym_chain[i];
	global_sym_chain[i] = sym;
	common_block = 0;
	break;
      }

    case N_ECOML:
    case N_LENG:
    case N_DEFD:		/* GNU Modula-2 symbol */
      break;

    default:
      if (name)
	define_symbol (valu, name, desc, type);
    }
}

/* Copy a pending list, used to record the contents of a common
   block for later fixup.  */
static struct pending *
copy_pending (beg, begi, end)
    struct pending *beg, *end;
    int begi;
{
  struct pending *new = 0;
  struct pending *next;

  for (next = beg; next != 0 && (next != end || begi < end->nsyms);
       next = next->next, begi = 0)
    {
      register int j;
      for (j = begi; j < next->nsyms; j++)
	add_symbol_to_list (next->symbol[j], &new);
    }
  return new;
}

/* Register our willingness to decode symbols for SunOS and a.out and
   b.out files handled by BFD... */
static struct sym_fns sunos_sym_fns = {"sunOs", 6,
              dbx_new_init, dbx_symfile_init, dbx_symfile_read};

static struct sym_fns aout_sym_fns = {"a.out", 5,
              dbx_new_init, dbx_symfile_init, dbx_symfile_read};

static struct sym_fns bout_sym_fns = {"b.out", 5,
              dbx_new_init, dbx_symfile_init, dbx_symfile_read};

void
_initialize_dbxread ()
{
  add_symtab_fns(&sunos_sym_fns);
  add_symtab_fns(&aout_sym_fns);
  add_symtab_fns(&bout_sym_fns);
}
