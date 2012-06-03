/* COFF specific linker code.
   Copyright 1994, 1995 Free Software Foundation, Inc.
   Written by Ian Lance Taylor, Cygnus Support.

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

/* This file contains the COFF backend linker code.  */

#include "bfd.h"
#include "sysdep.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "coff/internal.h"
#include "libcoff.h"

#define STRING_SIZE_SIZE (4)

/* Information we keep for each section in the output file when doing
   a relocateable link.  */

struct coff_link_section_info
{
  /* The relocs to be output.  */
  struct internal_reloc *relocs;
  /* For each reloc against a global symbol whose index was not known
     when the reloc was handled, the global hash table entry.  */
  struct coff_link_hash_entry **rel_hashes;
};

/* Information that we pass around while doing the final link step.  */

struct coff_final_link_info
{
  /* General link information.  */
  struct bfd_link_info *info;
  /* Output BFD.  */
  bfd *output_bfd;
  /* Used to indicate failure in traversal routine.  */
  boolean failed;
  /* Hash table for long symbol name.  */
  struct bfd_strtab_hash *strtab;
  /* When doing a relocateable link, an array of information kept for
     each output section, indexed by the target_index field.  */
  struct coff_link_section_info *section_info;
  /* Symbol index of last C_FILE symbol (-1 if none).  */
  long last_file_index;
  /* Contents of last C_FILE symbol.  */
  struct internal_syment last_file;
  /* Buffer large enough to hold swapped symbols of any input file.  */
  struct internal_syment *internal_syms;
  /* Buffer large enough to hold sections of symbols of any input file.  */
  asection **sec_ptrs;
  /* Buffer large enough to hold output indices of symbols of any
     input file.  */
  long *sym_indices;
  /* Buffer large enough to hold output symbols for any input file.  */
  bfd_byte *outsyms;
  /* Buffer large enough to hold external line numbers for any input
     section.  */
  bfd_byte *linenos;
  /* Buffer large enough to hold any input section.  */
  bfd_byte *contents;
  /* Buffer large enough to hold external relocs of any input section.  */
  bfd_byte *external_relocs;
  /* Buffer large enough to hold swapped relocs of any input section.  */
  struct internal_reloc *internal_relocs;
};

static struct bfd_hash_entry *coff_link_hash_newfunc
  PARAMS ((struct bfd_hash_entry *, struct bfd_hash_table *, const char *));
static boolean coff_link_add_object_symbols
  PARAMS ((bfd *, struct bfd_link_info *));
static boolean coff_link_check_archive_element
  PARAMS ((bfd *, struct bfd_link_info *, boolean *));
static boolean coff_link_get_symbols PARAMS ((bfd *));
static const char *coff_read_string_table PARAMS ((bfd *));
static boolean coff_link_free_symbols PARAMS ((bfd *));
static boolean coff_link_check_ar_symbols
  PARAMS ((bfd *, struct bfd_link_info *, boolean *));
static boolean coff_link_add_symbols PARAMS ((bfd *, struct bfd_link_info *));
static boolean coff_link_input_bfd
  PARAMS ((struct coff_final_link_info *, bfd *));
static boolean coff_write_global_sym
  PARAMS ((struct coff_link_hash_entry *, PTR));
static boolean coff_reloc_link_order
  PARAMS ((bfd *, struct coff_final_link_info *, asection *,
	   struct bfd_link_order *));

/* Create an entry in a COFF linker hash table.  */

static struct bfd_hash_entry *
coff_link_hash_newfunc (entry, table, string)
     struct bfd_hash_entry *entry;
     struct bfd_hash_table *table;
     const char *string;
{
  struct coff_link_hash_entry *ret = (struct coff_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == (struct coff_link_hash_entry *) NULL)
    ret = ((struct coff_link_hash_entry *)
	   bfd_hash_allocate (table, sizeof (struct coff_link_hash_entry)));
  if (ret == (struct coff_link_hash_entry *) NULL)
    {
      bfd_set_error (bfd_error_no_memory);
      return (struct bfd_hash_entry *) ret;
    }

  /* Call the allocation method of the superclass.  */
  ret = ((struct coff_link_hash_entry *)
	 _bfd_link_hash_newfunc ((struct bfd_hash_entry *) ret,
				 table, string));
  if (ret != (struct coff_link_hash_entry *) NULL)
    {
      /* Set local fields.  */
      ret->indx = -1;
      ret->type = T_NULL;
      ret->class = C_NULL;
      ret->numaux = 0;
      ret->auxbfd = NULL;
      ret->aux = NULL;
    }

  return (struct bfd_hash_entry *) ret;
}

/* Create a COFF linker hash table.  */

struct bfd_link_hash_table *
_bfd_coff_link_hash_table_create (abfd)
     bfd *abfd;
{
  struct coff_link_hash_table *ret;

  ret = ((struct coff_link_hash_table *)
	 malloc (sizeof (struct coff_link_hash_table)));
  if (ret == NULL)
    {
      bfd_set_error (bfd_error_no_memory);
      return NULL;
    }
  if (! _bfd_link_hash_table_init (&ret->root, abfd,
				   coff_link_hash_newfunc))
    {
      free (ret);
      return (struct bfd_link_hash_table *) NULL;
    }
  return &ret->root;
}

/* Given a COFF BFD, add symbols to the global hash table as
   appropriate.  */

boolean
_bfd_coff_link_add_symbols (abfd, info)
     bfd *abfd;
     struct bfd_link_info *info;
{
  switch (bfd_get_format (abfd))
    {
    case bfd_object:
      return coff_link_add_object_symbols (abfd, info);
    case bfd_archive:
      return (_bfd_generic_link_add_archive_symbols
	      (abfd, info, coff_link_check_archive_element));
    default:
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }
}

/* Add symbols from a COFF object file.  */

static boolean
coff_link_add_object_symbols (abfd, info)
     bfd *abfd;
     struct bfd_link_info *info;
{
  if (! coff_link_get_symbols (abfd))
    return false;
  if (! coff_link_add_symbols (abfd, info))
    return false;
  if (! info->keep_memory)
    {
      if (! coff_link_free_symbols (abfd))
	return false;
    }
  return true;
}

/* Check a single archive element to see if we need to include it in
   the link.  *PNEEDED is set according to whether this element is
   needed in the link or not.  This is called via
   _bfd_generic_link_add_archive_symbols.  */

static boolean
coff_link_check_archive_element (abfd, info, pneeded)
     bfd *abfd;
     struct bfd_link_info *info;
     boolean *pneeded;
{
  if (! coff_link_get_symbols (abfd))
    return false;

  if (! coff_link_check_ar_symbols (abfd, info, pneeded))
    return false;

  if (*pneeded)
    {
      if (! coff_link_add_symbols (abfd, info))
	return false;
    }

  if (! info->keep_memory || ! *pneeded)
    {
      if (! coff_link_free_symbols (abfd))
	return false;
    }

  return true;
}

/* Read in the external symbols.  */

static boolean
coff_link_get_symbols (abfd)
     bfd *abfd;
{
  bfd_size_type symesz;
  size_t size;
  PTR syms;

  if (obj_coff_external_syms (abfd) != NULL)
    return true;

  symesz = bfd_coff_symesz (abfd);

  size = obj_raw_syment_count (abfd) * symesz;

  syms = malloc (size);
  if (syms == NULL && size != 0)
    {
      bfd_set_error (bfd_error_no_memory);
      return false;
    }

  if (bfd_seek (abfd, obj_sym_filepos (abfd), SEEK_SET) != 0
      || bfd_read (syms, size, 1, abfd) != size)
    {
      if (syms != NULL)
	free (syms);
      return false;
    }

  obj_coff_external_syms (abfd) = syms;

  return true;
}

/* Read in the external strings.  The strings are not loaded until
   they are needed.  This is because we have no simple way of
   detecting a missing string table in an archive.  */

static const char *
coff_read_string_table (abfd)
     bfd *abfd;
{
  char extstrsize[STRING_SIZE_SIZE];
  size_t strsize;
  char *strings;

  if (obj_coff_strings (abfd) != NULL)
    return obj_coff_strings (abfd);

  if (bfd_seek (abfd,
		(obj_sym_filepos (abfd)
		 + obj_raw_syment_count (abfd) * bfd_coff_symesz (abfd)),
		SEEK_SET) != 0)
    return NULL;
    
  if (bfd_read (extstrsize, sizeof extstrsize, 1, abfd) != sizeof extstrsize)
    {
      if (bfd_get_error () != bfd_error_file_truncated)
	return NULL;

      /* There is no string table.  */
      strsize = STRING_SIZE_SIZE;
    }
  else
    {
#if STRING_SIZE_SIZE == 4
      strsize = bfd_h_get_32 (abfd, (bfd_byte *) extstrsize);
#else
 #error Change bfd_h_get_32
#endif
    }

  strings = malloc (strsize);
  if (strings == NULL)
    {
      bfd_set_error (bfd_error_no_memory);
      return NULL;
    }

  if (bfd_read (strings + STRING_SIZE_SIZE,
		strsize - STRING_SIZE_SIZE, 1, abfd)
      != strsize - STRING_SIZE_SIZE)
    {
      free (strings);
      return NULL;
    }

  obj_coff_strings (abfd) = strings;

  return strings;
}

/* Free up the external symbols and strings read from a COFF file.  */

static boolean
coff_link_free_symbols (abfd)
     bfd *abfd;
{
  if (obj_coff_external_syms (abfd) != NULL)
    {
      free (obj_coff_external_syms (abfd));
      obj_coff_external_syms (abfd) = NULL;
    }
  if (obj_coff_strings (abfd) != NULL)
    {
      free (obj_coff_strings (abfd));
      obj_coff_strings (abfd) = NULL;
    }
  return true;
}

/* Look through the symbols to see if this object file should be
   included in the link.  */

static boolean
coff_link_check_ar_symbols (abfd, info, pneeded)
     bfd *abfd;
     struct bfd_link_info *info;
     boolean *pneeded;
{
  boolean (*sym_is_global) PARAMS ((bfd *, struct internal_syment *));
  const char *strings;
  bfd_size_type symesz;
  bfd_byte *esym;
  bfd_byte *esym_end;

  *pneeded = false;

  sym_is_global = coff_backend_info (abfd)->_bfd_coff_sym_is_global;
  strings = NULL;

  symesz = bfd_coff_symesz (abfd);
  esym = (bfd_byte *) obj_coff_external_syms (abfd);
  esym_end = esym + obj_raw_syment_count (abfd) * symesz;
  while (esym < esym_end)
    {
      struct internal_syment sym;

      bfd_coff_swap_sym_in (abfd, (PTR) esym, (PTR) &sym);

      if ((sym.n_sclass == C_EXT
	   || (sym_is_global && (*sym_is_global) (abfd, &sym)))
	  && (sym.n_scnum != 0 || sym.n_value != 0))
	{
	  const char *name;
	  char buf[SYMNMLEN + 1];
	  struct bfd_link_hash_entry *h;

	  /* This symbol is externally visible, and is defined by this
             object file.  */

	  /* FIXME: It's not clear this will work correctly if sizeof
             (_n_zeroes) != 4.  */
	  if (sym._n._n_n._n_zeroes != 0
	      || sym._n._n_n._n_offset == 0)
	    {
	      memcpy (buf, sym._n._n_name, SYMNMLEN);
	      buf[SYMNMLEN] = '\0';
	      name = buf;
	    }
	  else
	    {
	      BFD_ASSERT (sym._n._n_n._n_offset >= STRING_SIZE_SIZE);
	      if (strings == NULL)
		{
		  strings = coff_read_string_table (abfd);
		  if (strings == NULL)
		    return false;
		}
	      name = strings + sym._n._n_n._n_offset;
	    }

	  h = bfd_link_hash_lookup (info->hash, name, false, false, true);

	  /* We are only interested in symbols that are currently
	     undefined.  If a symbol is currently known to be common,
	     COFF linkers do not bring in an object file which defines
	     it.  */
	  if (h != (struct bfd_link_hash_entry *) NULL
	      && h->type == bfd_link_hash_undefined)
	    {
	      if (! (*info->callbacks->add_archive_element) (info, abfd, name))
		return false;
	      *pneeded = true;
	      return true;
	    }
	}

      esym += (sym.n_numaux + 1) * symesz;
    }

  /* We do not need this object file.  */
  return true;
}

/* Add all the symbols from an object file to the hash table.  */

static boolean
coff_link_add_symbols (abfd, info)
     bfd *abfd;
     struct bfd_link_info *info;
{
  boolean (*sym_is_global) PARAMS ((bfd *, struct internal_syment *));
  const char *strings;
  boolean default_copy;
  bfd_size_type symcount;
  struct coff_link_hash_entry **sym_hash;
  bfd_size_type symesz;
  bfd_byte *esym;
  bfd_byte *esym_end;

  sym_is_global = coff_backend_info (abfd)->_bfd_coff_sym_is_global;
  strings = NULL;

  if (info->keep_memory)
    default_copy = false;
  else
    default_copy = true;

  symcount = obj_raw_syment_count (abfd);

  /* We keep a list of the linker hash table entries that correspond
     to particular symbols.  */
  sym_hash = ((struct coff_link_hash_entry **)
	      bfd_alloc (abfd,
			 ((size_t) symcount
			  * sizeof (struct coff_link_hash_entry *))));
  if (sym_hash == NULL && symcount != 0)
    {
      bfd_set_error (bfd_error_no_memory);
      return false;
    }
  obj_coff_sym_hashes (abfd) = sym_hash;
  memset (sym_hash, 0,
	  (size_t) symcount * sizeof (struct coff_link_hash_entry *));

  symesz = bfd_coff_symesz (abfd);
  BFD_ASSERT (symesz == bfd_coff_auxesz (abfd));
  esym = (bfd_byte *) obj_coff_external_syms (abfd);
  esym_end = esym + symcount * symesz;
  while (esym < esym_end)
    {
      struct internal_syment sym;
      boolean copy;

      bfd_coff_swap_sym_in (abfd, (PTR) esym, (PTR) &sym);

      if (sym.n_sclass == C_EXT
	  || (sym_is_global && (*sym_is_global) (abfd, &sym)))
	{
	  const char *name;
	  char buf[SYMNMLEN + 1];
	  flagword flags;
	  asection *section;
	  bfd_vma value;

	  /* This symbol is externally visible.  */

	  /* FIXME: It's not clear this will work correctly if sizeof
             (_n_zeroes) != 4.  */
	  copy = default_copy;
	  if (sym._n._n_n._n_zeroes == 0
	      && sym._n._n_n._n_offset != 0)
	    {
	      BFD_ASSERT (sym._n._n_n._n_offset >= STRING_SIZE_SIZE);
	      if (strings == NULL)
		{
		  strings = coff_read_string_table (abfd);
		  if (strings == NULL)
		    return false;
		}
	      name = strings + sym._n._n_n._n_offset;
	    }
	  else
	    {
	      memcpy (buf, sym._n._n_name, SYMNMLEN);
	      buf[SYMNMLEN] = '\0';
	      name = buf;
	      copy = true;
	    }

	  value = sym.n_value;

	  if (sym.n_scnum == 0)
	    {
	      if (value == 0)
		{
		  flags = 0;
		  section = bfd_und_section_ptr;
		}
	      else
		{
		  flags = BSF_GLOBAL;
		  section = bfd_com_section_ptr;
		}
	    }
	  else
	    {
	      flags = BSF_EXPORT | BSF_GLOBAL;
	      section = coff_section_from_bfd_index (abfd, sym.n_scnum);
	      value -= section->vma;
	    }

	  if (! (_bfd_generic_link_add_one_symbol
		 (info, abfd, name, flags, section, value,
		  (const char *) NULL, copy, false,
		  (struct bfd_link_hash_entry **) sym_hash)))
	    return false;

	  if (info->hash->creator->flavour == bfd_get_flavour (abfd))
	    {
	      if (((*sym_hash)->class == C_NULL
		   && (*sym_hash)->type == T_NULL)
		  || sym.n_scnum != 0
		  || (sym.n_value != 0
		      && (*sym_hash)->root.type != bfd_link_hash_defined))
		{
		  (*sym_hash)->class = sym.n_sclass;
		  (*sym_hash)->type = sym.n_type;
		  (*sym_hash)->numaux = sym.n_numaux;
		  (*sym_hash)->auxbfd = abfd;
		  if (sym.n_numaux != 0)
		    {
		      union internal_auxent *alloc;
		      unsigned int i;
		      bfd_byte *eaux;
		      union internal_auxent *iaux;

		      alloc = ((union internal_auxent *)
			       bfd_hash_allocate (&info->hash->table,
						  (sym.n_numaux
						   * sizeof (*alloc))));
		      if (alloc == NULL)
			{
			  bfd_set_error (bfd_error_no_memory);
			  return false;
			}
		      for (i = 0, eaux = esym + symesz, iaux = alloc;
			   i < sym.n_numaux;
			   i++, eaux += symesz, iaux++)
			bfd_coff_swap_aux_in (abfd, (PTR) eaux, sym.n_type,
					      sym.n_sclass, i, sym.n_numaux,
					      (PTR) iaux);
		      (*sym_hash)->aux = alloc;
		    }
		}
	    }
	}

      esym += (sym.n_numaux + 1) * symesz;
      sym_hash += sym.n_numaux + 1;
    }

  return true;
}

/* Do the final link step.  */

boolean
_bfd_coff_final_link (abfd, info)
     bfd *abfd;
     struct bfd_link_info *info;
{
  bfd_size_type symesz;
  struct coff_final_link_info finfo;
  asection *o;
  struct bfd_link_order *p;
  size_t max_contents_size;
  size_t max_sym_count;
  size_t max_lineno_count;
  size_t max_reloc_count;
  size_t max_output_reloc_count;
  file_ptr rel_filepos;
  unsigned int relsz;
  file_ptr line_filepos;
  unsigned int linesz;
  bfd *sub;
  bfd_byte *external_relocs = NULL;
  char strbuf[STRING_SIZE_SIZE];

  symesz = bfd_coff_symesz (abfd);

  finfo.info = info;
  finfo.output_bfd = abfd;
  finfo.strtab = NULL;
  finfo.section_info = NULL;
  finfo.last_file_index = -1;
  finfo.internal_syms = NULL;
  finfo.sec_ptrs = NULL;
  finfo.sym_indices = NULL;
  finfo.outsyms = NULL;
  finfo.linenos = NULL;
  finfo.contents = NULL;
  finfo.external_relocs = NULL;
  finfo.internal_relocs = NULL;

  finfo.strtab = _bfd_stringtab_init ();
  if (finfo.strtab == NULL)
    goto error_return;

  /* Compute the file positions for all the sections.  */
  if (! abfd->output_has_begun)
    bfd_coff_compute_section_file_positions (abfd);

  /* Count the line numbers and relocation entries required for the
     output file.  Set the file positions for the relocs.  */
  rel_filepos = obj_relocbase (abfd);
  relsz = bfd_coff_relsz (abfd);
  max_contents_size = 0;
  max_lineno_count = 0;
  max_reloc_count = 0;
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      o->reloc_count = 0;
      o->lineno_count = 0;
      for (p = o->link_order_head; p != NULL; p = p->next)
	{
	  if (p->type == bfd_indirect_link_order)
	    {
	      asection *sec;

	      sec = p->u.indirect.section;

	      if (info->strip == strip_none
		  || info->strip == strip_some)
		o->lineno_count += sec->lineno_count;

	      if (info->relocateable)
		o->reloc_count += sec->reloc_count;

	      if (sec->_raw_size > max_contents_size)
		max_contents_size = sec->_raw_size;
	      if (sec->lineno_count > max_lineno_count)
		max_lineno_count = sec->lineno_count;
	      if (sec->reloc_count > max_reloc_count)
		max_reloc_count = sec->reloc_count;
	    }
	  else if (info->relocateable
		   && (p->type == bfd_section_reloc_link_order
		       || p->type == bfd_symbol_reloc_link_order))
	    ++o->reloc_count;
	}
      if (o->reloc_count == 0)
	o->rel_filepos = 0;
      else
	{
	  o->flags |= SEC_RELOC;
	  o->rel_filepos = rel_filepos;
	  rel_filepos += o->reloc_count * relsz;
	}
    }

  /* If doing a relocateable link, allocate space for the pointers we
     need to keep.  */
  if (info->relocateable)
    {
      unsigned int i;

      /* We use section_count + 1, rather than section_count, because
         the target_index fields are 1 based.  */
      finfo.section_info = ((struct coff_link_section_info *)
			    malloc ((abfd->section_count + 1)
				    * sizeof (struct coff_link_section_info)));
      if (finfo.section_info == NULL)
	{
	  bfd_set_error (bfd_error_no_memory);
	  goto error_return;
	}
      for (i = 0; i <= abfd->section_count; i++)
	{
	  finfo.section_info[i].relocs = NULL;
	  finfo.section_info[i].rel_hashes = NULL;
	}
    }

  /* We now know the size of the relocs, so we can determine the file
     positions of the line numbers.  */
  line_filepos = rel_filepos;
  linesz = bfd_coff_linesz (abfd);
  max_output_reloc_count = 0;
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      if (o->lineno_count == 0)
	o->line_filepos = 0;
      else
	{
	  o->line_filepos = line_filepos;
	  line_filepos += o->lineno_count * linesz;
	}

      if (o->reloc_count != 0)
	{
	  /* We don't know the indices of global symbols until we have
             written out all the local symbols.  For each section in
             the output file, we keep an array of pointers to hash
             table entries.  Each entry in the array corresponds to a
             reloc.  When we find a reloc against a global symbol, we
             set the corresponding entry in this array so that we can
             fix up the symbol index after we have written out all the
             local symbols.

	     Because of this problem, we also keep the relocs in
	     memory until the end of the link.  This wastes memory,
	     but only when doing a relocateable link, which is not the
	     common case.  */
	  BFD_ASSERT (info->relocateable);
	  finfo.section_info[o->target_index].relocs =
	    ((struct internal_reloc *)
	     malloc (o->reloc_count * sizeof (struct internal_reloc)));
	  finfo.section_info[o->target_index].rel_hashes =
	    ((struct coff_link_hash_entry **)
	     malloc (o->reloc_count
		     * sizeof (struct coff_link_hash_entry *)));
	  if (finfo.section_info[o->target_index].relocs == NULL
	      || finfo.section_info[o->target_index].rel_hashes == NULL)
	    {
	      bfd_set_error (bfd_error_no_memory);
	      goto error_return;
	    }

	  if (o->reloc_count > max_output_reloc_count)
	    max_output_reloc_count = o->reloc_count;
	}

      /* Reset the reloc and lineno counts, so that we can use them to
	 count the number of entries we have output so far.  */
      o->reloc_count = 0;
      o->lineno_count = 0;
    }

  obj_sym_filepos (abfd) = line_filepos;

  /* Figure out the largest number of symbols in an input BFD.  Take
     the opportunity to clear the output_has_begun fields of all the
     input BFD's.  */
  max_sym_count = 0;
  for (sub = info->input_bfds; sub != NULL; sub = sub->link_next)
    {
      size_t sz;

      sub->output_has_begun = false;
      sz = obj_raw_syment_count (sub);
      if (sz > max_sym_count)
	max_sym_count = sz;
    }

  /* Allocate some buffers used while linking.  */
  finfo.internal_syms = ((struct internal_syment *)
			 malloc (max_sym_count
				 * sizeof (struct internal_syment)));
  finfo.sec_ptrs = (asection **) malloc (max_sym_count * sizeof (asection *));
  finfo.sym_indices = (long *) malloc (max_sym_count * sizeof (long));
  finfo.outsyms = (bfd_byte *) malloc ((max_sym_count + 1) * symesz);
  finfo.linenos = (bfd_byte *) malloc (max_lineno_count
				       * bfd_coff_linesz (abfd));
  finfo.contents = (bfd_byte *) malloc (max_contents_size);
  finfo.external_relocs = (bfd_byte *) malloc (max_reloc_count * relsz);
  if (! info->relocateable)
    finfo.internal_relocs = ((struct internal_reloc *)
			     malloc (max_reloc_count
				     * sizeof (struct internal_reloc)));
  if ((finfo.internal_syms == NULL && max_sym_count > 0)
      || (finfo.sec_ptrs == NULL && max_sym_count > 0)
      || (finfo.sym_indices == NULL && max_sym_count > 0)
      || finfo.outsyms == NULL
      || (finfo.linenos == NULL && max_lineno_count > 0)
      || (finfo.contents == NULL && max_contents_size > 0)
      || (finfo.external_relocs == NULL && max_reloc_count > 0)
      || (! info->relocateable
	  && finfo.internal_relocs == NULL
	  && max_reloc_count > 0))
    {
      bfd_set_error (bfd_error_no_memory);
      goto error_return;
    }

  /* We now know the position of everything in the file, except that
     we don't know the size of the symbol table and therefore we don't
     know where the string table starts.  We just build the string
     table in memory as we go along.  We process all the relocations
     for a single input file at once.  */
  obj_raw_syment_count (abfd) = 0;
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      for (p = o->link_order_head; p != NULL; p = p->next)
	{
	  if (p->type == bfd_indirect_link_order
	      && (bfd_get_flavour (p->u.indirect.section->owner)
		  == bfd_target_coff_flavour))
	    {
	      sub = p->u.indirect.section->owner;
	      if (! sub->output_has_begun)
		{
		  if (! coff_link_input_bfd (&finfo, sub))
		    goto error_return;
		  sub->output_has_begun = true;
		}
	    }
	  else if (p->type == bfd_section_reloc_link_order
		   || p->type == bfd_symbol_reloc_link_order)
	    {
	      if (! coff_reloc_link_order (abfd, &finfo, o, p))
		goto error_return;
	    }
	  else
	    {
	      if (! _bfd_default_link_order (abfd, info, o, p))
		goto error_return;
	    }
	}
    }

  /* Free up the buffers used by coff_link_input_bfd.  */
  if (finfo.internal_syms != NULL)
    {
      free (finfo.internal_syms);
      finfo.internal_syms = NULL;
    }
  if (finfo.sec_ptrs != NULL)
    {
      free (finfo.sec_ptrs);
      finfo.sec_ptrs = NULL;
    }
  if (finfo.sym_indices != NULL)
    {
      free (finfo.sym_indices);
      finfo.sym_indices = NULL;
    }
  if (finfo.linenos != NULL)
    {
      free (finfo.linenos);
      finfo.linenos = NULL;
    }
  if (finfo.contents != NULL)
    {
      free (finfo.contents);
      finfo.contents = NULL;
    }
  if (finfo.external_relocs != NULL)
    {
      free (finfo.external_relocs);
      finfo.external_relocs = NULL;
    }
  if (finfo.internal_relocs != NULL)
    {
      free (finfo.internal_relocs);
      finfo.internal_relocs = NULL;
    }

  /* The value of the last C_FILE symbol is supposed to be the symbol
     index of the first external symbol.  Write it out again if
     necessary.  */
  if (finfo.last_file_index != -1
      && finfo.last_file.n_value != obj_raw_syment_count (abfd))
    {
      finfo.last_file.n_value = obj_raw_syment_count (abfd);
      bfd_coff_swap_sym_out (abfd, (PTR) &finfo.last_file,
			     (PTR) finfo.outsyms);
      if (bfd_seek (abfd,
		    (obj_sym_filepos (abfd)
		     + finfo.last_file_index * symesz),
		    SEEK_SET) != 0
	  || bfd_write (finfo.outsyms, symesz, 1, abfd) != symesz)
	return false;
    }

  /* Write out the global symbols.  */
  finfo.failed = false;
  coff_link_hash_traverse (coff_hash_table (info), coff_write_global_sym,
			   (PTR) &finfo);
  if (finfo.failed)
    goto error_return;

  /* The outsyms buffer is used by coff_write_global_sym.  */
  if (finfo.outsyms != NULL)
    {
      free (finfo.outsyms);
      finfo.outsyms = NULL;
    }

  if (info->relocateable)
    {
      /* Now that we have written out all the global symbols, we know
	 the symbol indices to use for relocs against them, and we can
	 finally write out the relocs.  */
      external_relocs = (bfd_byte *) malloc (max_output_reloc_count * relsz);
      if (external_relocs == NULL)
	{
	  bfd_set_error (bfd_error_no_memory);
	  goto error_return;
	}

      for (o = abfd->sections; o != NULL; o = o->next)
	{
	  struct internal_reloc *irel;
	  struct internal_reloc *irelend;
	  struct coff_link_hash_entry **rel_hash;
	  bfd_byte *erel;

	  if (o->reloc_count == 0)
	    continue;

	  irel = finfo.section_info[o->target_index].relocs;
	  irelend = irel + o->reloc_count;
	  rel_hash = finfo.section_info[o->target_index].rel_hashes;
	  erel = external_relocs;
	  for (; irel < irelend; irel++, rel_hash++, erel += relsz)
	    {
	      if (*rel_hash != NULL)
		{
		  BFD_ASSERT ((*rel_hash)->indx >= 0);
		  irel->r_symndx = (*rel_hash)->indx;
		}
	      bfd_coff_swap_reloc_out (abfd, (PTR) irel, (PTR) erel);
	    }

	  if (bfd_seek (abfd, o->rel_filepos, SEEK_SET) != 0
	      || bfd_write ((PTR) external_relocs, relsz, o->reloc_count,
			    abfd) != relsz * o->reloc_count)
	    goto error_return;
	}

      free (external_relocs);
      external_relocs = NULL;
    }

  /* Free up the section information.  */
  if (finfo.section_info != NULL)
    {
      unsigned int i;

      for (i = 0; i < abfd->section_count; i++)
	{
	  if (finfo.section_info[i].relocs != NULL)
	    free (finfo.section_info[i].relocs);
	  if (finfo.section_info[i].rel_hashes != NULL)
	    free (finfo.section_info[i].rel_hashes);
	}
      free (finfo.section_info);
      finfo.section_info = NULL;
    }

  /* Write out the string table.  */
  if (bfd_seek (abfd,
		(obj_sym_filepos (abfd)
		 + obj_raw_syment_count (abfd) * symesz),
		SEEK_SET) != 0)
    return false;

#if STRING_SIZE_SIZE == 4
  bfd_h_put_32 (abfd,
		_bfd_stringtab_size (finfo.strtab) + STRING_SIZE_SIZE,
		(bfd_byte *) strbuf);
#else
 #error Change bfd_h_put_32
#endif

  if (bfd_write (strbuf, 1, STRING_SIZE_SIZE, abfd) != STRING_SIZE_SIZE)
    return false;

  if (! _bfd_stringtab_emit (abfd, finfo.strtab))
    return false;

  _bfd_stringtab_free (finfo.strtab);

  /* Setting bfd_get_symcount to 0 will cause write_object_contents to
     not try to write out the symbols.  */
  bfd_get_symcount (abfd) = 0;

  return true;

 error_return:
  if (finfo.strtab != NULL)
    _bfd_stringtab_free (finfo.strtab);
  if (finfo.section_info != NULL)
    {
      unsigned int i;

      for (i = 0; i < abfd->section_count; i++)
	{
	  if (finfo.section_info[i].relocs != NULL)
	    free (finfo.section_info[i].relocs);
	  if (finfo.section_info[i].rel_hashes != NULL)
	    free (finfo.section_info[i].rel_hashes);
	}
      free (finfo.section_info);
    }
  if (finfo.internal_syms != NULL)
    free (finfo.internal_syms);
  if (finfo.sec_ptrs != NULL)
    free (finfo.sec_ptrs);
  if (finfo.sym_indices != NULL)
    free (finfo.sym_indices);
  if (finfo.outsyms != NULL)
    free (finfo.outsyms);
  if (finfo.linenos != NULL)
    free (finfo.linenos);
  if (finfo.contents != NULL)
    free (finfo.contents);
  if (finfo.external_relocs != NULL)
    free (finfo.external_relocs);
  if (finfo.internal_relocs != NULL)
    free (finfo.internal_relocs);
  if (external_relocs != NULL)
    free (external_relocs);
  return false;
}

/* Link an input file into the linker output file.  This function
   handles all the sections and relocations of the input file at once.  */

static boolean
coff_link_input_bfd (finfo, input_bfd)
     struct coff_final_link_info *finfo;
     bfd *input_bfd;
{
  boolean (*sym_is_global) PARAMS ((bfd *, struct internal_syment *));
  bfd *output_bfd;
  const char *strings;
  bfd_size_type syment_base;
  unsigned int n_tmask;
  unsigned int n_btshft;
  boolean copy, hash;
  bfd_size_type isymesz;
  bfd_size_type osymesz;
  bfd_size_type linesz;
  bfd_byte *esym;
  bfd_byte *esym_end;
  struct internal_syment *isymp;
  asection **secpp;
  long *indexp;
  long output_index;
  bfd_byte *outsym;
  struct coff_link_hash_entry **sym_hash;
  bfd_size_type relsz;
  asection *o;

  /* Move all the symbols to the output file.  */

  output_bfd = finfo->output_bfd;
  sym_is_global = coff_backend_info (input_bfd)->_bfd_coff_sym_is_global;
  strings = NULL;
  syment_base = obj_raw_syment_count (output_bfd);
  isymesz = bfd_coff_symesz (input_bfd);
  osymesz = bfd_coff_symesz (output_bfd);
  linesz = bfd_coff_linesz (input_bfd);
  BFD_ASSERT (linesz == bfd_coff_linesz (output_bfd));

  n_tmask = coff_data (input_bfd)->local_n_tmask;
  n_btshft = coff_data (input_bfd)->local_n_btshft;

  /* Define macros so that ISFCN, et. al., macros work correctly.  */
#define N_TMASK n_tmask
#define N_BTSHFT n_btshft

  copy = false;
  if (! finfo->info->keep_memory)
    copy = true;
  hash = true;
  if ((output_bfd->flags & BFD_TRADITIONAL_FORMAT) != 0)
    hash = false;

  if (! coff_link_get_symbols (input_bfd))
    return false;

  esym = (bfd_byte *) obj_coff_external_syms (input_bfd);
  esym_end = esym + obj_raw_syment_count (input_bfd) * isymesz;
  isymp = finfo->internal_syms;
  secpp = finfo->sec_ptrs;
  indexp = finfo->sym_indices;
  output_index = syment_base;
  outsym = finfo->outsyms;
  while (esym < esym_end)
    {
      struct internal_syment isym;
      boolean skip;
      boolean global;
      int add;

      bfd_coff_swap_sym_in (input_bfd, (PTR) esym, (PTR) isymp);

      /* Make a copy of *isymp so that the relocate_section function
	 always sees the original values.  This is more reliable than
	 always recomputing the symbol value even if we are stripping
	 the symbol.  */
      isym = *isymp;

      if (isym.n_scnum != 0)
	*secpp = coff_section_from_bfd_index (input_bfd, isym.n_scnum);
      else
	{
	  if (isym.n_value == 0)
	    *secpp = bfd_und_section_ptr;
	  else
	    *secpp = bfd_com_section_ptr;
	}

      *indexp = -1;

      skip = false;
      global = false;
      add = 1 + isym.n_numaux;

      /* If we are stripping all symbols, we want to skip this one.  */
      if (finfo->info->strip == strip_all)
	skip = true;

      if (! skip)
	{
	  if (isym.n_sclass == C_EXT
	      || (sym_is_global && (*sym_is_global) (input_bfd, &isym)))
	    {
	      /* This is a global symbol.  Global symbols come at the
		 end of the symbol table, so skip them for now.
		 Function symbols, however, are an exception, and are
		 not moved to the end.  */
	      global = true;
	      if (! ISFCN (isym.n_type))
		skip = true;
	    }
	  else
	    {
	      /* This is a local symbol.  Skip it if we are discarding
                 local symbols.  */
	      if (finfo->info->discard == discard_all)
		skip = true;
	    }
	}

      /* If we stripping debugging symbols, and this is a debugging
         symbol, then skip it.  */
      if (! skip
	  && finfo->info->strip == strip_debugger
	  && isym.n_scnum == N_DEBUG)
	skip = true;

      /* If some symbols are stripped based on the name, work out the
	 name and decide whether to skip this symbol.  */
      if (! skip
	  && (finfo->info->strip == strip_some
	      || finfo->info->discard == discard_l))
	{
	  const char *name;
	  char buf[SYMNMLEN + 1];

	  if (isym._n._n_n._n_zeroes == 0
	      && isym._n._n_n._n_offset != 0)
	    {
	      if (strings == NULL)
		{
		  strings = coff_read_string_table (input_bfd);
		  if (strings == NULL)
		    return false;
		}
	      name = strings + isym._n._n_n._n_offset;
	    }
	  else
	    {
	      memcpy (buf, isym._n._n_name, SYMNMLEN);
	      buf[SYMNMLEN] = '\0';
	      name = buf;
	    }

	  if ((finfo->info->strip == strip_some
	       && (bfd_hash_lookup (finfo->info->keep_hash, name, false,
				    false) == NULL))
	      || (! global
		  && finfo->info->discard == discard_l
		  && strncmp (name, finfo->info->lprefix,
			      finfo->info->lprefix_len) == 0))
	    skip = true;
	}

      /* We now know whether we are to skip this symbol or not.  */
      if (! skip)
	{
	  /* Adjust the symbol in order to output it.  */

	  if (isym._n._n_n._n_zeroes == 0
	      && isym._n._n_n._n_offset != 0)
	    {
	      const char *name;
	      bfd_size_type indx;

	      /* This symbol has a long name.  Enter it in the string
		 table we are building.  Note that we do not check
		 bfd_coff_symname_in_debug.  That is only true for
		 XCOFF, and XCOFF requires different linking code
		 anyhow.  */
	      BFD_ASSERT (isym._n._n_n._n_offset >= STRING_SIZE_SIZE);
	      if (strings == NULL)
		{
		  strings = coff_read_string_table (input_bfd);
		  if (strings == NULL)
		    return false;
		}
	      name = strings + isym._n._n_n._n_offset;
	      indx = _bfd_stringtab_add (finfo->strtab, name, hash, copy);
	      if (indx == (bfd_size_type) -1)
		return false;
	      isym._n._n_n._n_offset = STRING_SIZE_SIZE + indx;
	    }

	  if (isym.n_scnum > 0)
	    {
	      isym.n_scnum = (*secpp)->output_section->target_index;
	      isym.n_value += ((*secpp)->output_section->vma
			       + (*secpp)->output_offset
			       - (*secpp)->vma);
	    }

	  /* The value of a C_FILE symbol is the symbol index of the
	     next C_FILE symbol.  The value of the last C_FILE symbol
	     is the symbol index to the first external symbol
	     (actually, coff_renumber_symbols does not get this
	     right--it just sets the value of the last C_FILE symbol
	     to zero--and nobody has ever complained about it).  We
	     try to get this right, below, just before we write the
	     symbols out, but in the general case we may have to write
	     the symbol out twice.  */
	  if (isym.n_sclass == C_FILE)
	    {
	      if (finfo->last_file_index != -1
		  && finfo->last_file.n_value != output_index)
		{
		  /* We must correct the value of the last C_FILE entry.  */
		  finfo->last_file.n_value = output_index;
		  if (finfo->last_file_index >= syment_base)
		    {
		      /* The last C_FILE symbol is in this input file.  */
		      bfd_coff_swap_sym_out (output_bfd,
					     (PTR) &finfo->last_file,
					     (PTR) (finfo->outsyms
						    + ((finfo->last_file_index
							- syment_base)
						       * osymesz)));
		    }
		  else
		    {
		      /* We have already written out the last C_FILE
			 symbol.  We need to write it out again.  We
			 borrow *outsym temporarily.  */
		      bfd_coff_swap_sym_out (output_bfd,
					     (PTR) &finfo->last_file,
					     (PTR) outsym);
		      if (bfd_seek (output_bfd,
				    (obj_sym_filepos (output_bfd)
				     + finfo->last_file_index * osymesz),
				    SEEK_SET) != 0
			  || (bfd_write (outsym, osymesz, 1, output_bfd)
			      != osymesz))
			return false;
		    }
		}

	      finfo->last_file_index = output_index;
	      finfo->last_file = isym;
	    }

	  /* Output the symbol.  */

	  bfd_coff_swap_sym_out (output_bfd, (PTR) &isym, (PTR) outsym);

	  *indexp = output_index;

	  if (global)
	    {
	      long indx;
	      struct coff_link_hash_entry *h;

	      indx = ((esym - (bfd_byte *) obj_coff_external_syms (input_bfd))
		      / isymesz);
	      h = obj_coff_sym_hashes (input_bfd)[indx];
	      BFD_ASSERT (h != NULL);
	      h->indx = output_index;
	    }

	  output_index += add;
	  outsym += add * osymesz;
	}

      esym += add * isymesz;
      isymp += add;
      ++secpp;
      ++indexp;
      for (--add; add > 0; --add)
	{
	  *secpp++ = NULL;
	  *indexp++ = -1;
	}
    }

  /* Fix up the aux entries.  This must be done in a separate pass,
     because we don't know the correct symbol indices until we have
     already decided which symbols we are going to keep.  */

  esym = (bfd_byte *) obj_coff_external_syms (input_bfd);
  esym_end = esym + obj_raw_syment_count (input_bfd) * isymesz;
  isymp = finfo->internal_syms;
  indexp = finfo->sym_indices;
  sym_hash = obj_coff_sym_hashes (input_bfd);
  outsym = finfo->outsyms;
  while (esym < esym_end)
    {
      int add;

      add = 1 + isymp->n_numaux;

      if (*indexp < 0
	  && (*sym_hash == NULL
	      || (*sym_hash)->auxbfd != input_bfd))
	esym += add * isymesz;
      else
	{
	  struct coff_link_hash_entry *h;
	  int i;

	  h = NULL;
	  if (*indexp < 0)
	    {
	      h = *sym_hash;
	      BFD_ASSERT (h->numaux == isymp->n_numaux);
	    }

	  esym += isymesz;

	  if (h == NULL)
	    outsym += osymesz;

	  /* Handle the aux entries.  This handling is based on
	     coff_pointerize_aux.  I don't know if it always correct.  */
	  for (i = 0; i < isymp->n_numaux && esym < esym_end; i++)
	    {
	      union internal_auxent aux;
	      union internal_auxent *auxp;

	      if (h != NULL)
		auxp = h->aux + i;
	      else
		{
		  bfd_coff_swap_aux_in (input_bfd, (PTR) esym, isymp->n_type,
					isymp->n_sclass, i, isymp->n_numaux,
					(PTR) &aux);
		  auxp = &aux;
		}

	      if (isymp->n_sclass == C_FILE)
		{
		  /* If this is a long filename, we must put it in the
		     string table.  */
		  if (auxp->x_file.x_n.x_zeroes == 0
		      && auxp->x_file.x_n.x_offset != 0)
		    {
		      const char *filename;
		      bfd_size_type indx;

		      BFD_ASSERT (auxp->x_file.x_n.x_offset
				  >= STRING_SIZE_SIZE);
		      if (strings == NULL)
			{
			  strings = coff_read_string_table (input_bfd);
			  if (strings == NULL)
			    return false;
			}
		      filename = strings + auxp->x_file.x_n.x_offset;
		      indx = _bfd_stringtab_add (finfo->strtab, filename,
						 hash, copy);
		      if (indx == (bfd_size_type) -1)
			return false;
		      auxp->x_file.x_n.x_offset = STRING_SIZE_SIZE + indx;
		    }
		}
	      else if (isymp->n_sclass != C_STAT || isymp->n_type != T_NULL)
		{
		  long indx;

		  if (ISFCN (isymp->n_type)
		      || ISTAG (isymp->n_sclass)
		      || isymp->n_sclass == C_BLOCK)
		    {
		      indx = auxp->x_sym.x_fcnary.x_fcn.x_endndx.l;
		      if (indx > 0
			  && indx < obj_raw_syment_count (input_bfd))
			{
			  /* We look forward through the symbol for
                             the index of the next symbol we are going
                             to include.  I don't know if this is
                             entirely right.  */
			  while (finfo->sym_indices[indx] < 0
				 && indx < obj_raw_syment_count (input_bfd))
			    ++indx;
			  if (indx >= obj_raw_syment_count (input_bfd))
			    indx = output_index;
			  else
			    indx = finfo->sym_indices[indx];
			  auxp->x_sym.x_fcnary.x_fcn.x_endndx.l = indx;
			}
		    }

		  indx = auxp->x_sym.x_tagndx.l;
		  if (indx > 0 && indx < obj_raw_syment_count (input_bfd))
		    {
		      indx = finfo->sym_indices[indx];
		      if (indx < 0)
			auxp->x_sym.x_tagndx.l = 0;
		      else
			auxp->x_sym.x_tagndx.l = indx;
		    }
		}

	      if (h == NULL)
		{
		  bfd_coff_swap_aux_out (output_bfd, (PTR) auxp, isymp->n_type,
					 isymp->n_sclass, i, isymp->n_numaux,
					 (PTR) outsym);
		  outsym += osymesz;
		}

	      esym += isymesz;
	    }
	}

      indexp += add;
      isymp += add;
      sym_hash += add;
    }

  /* Relocate the line numbers, unless we are stripping them.  */
  if (finfo->info->strip == strip_none
      || finfo->info->strip == strip_some)
    {
      for (o = input_bfd->sections; o != NULL; o = o->next)
	{
	  bfd_vma offset;
	  bfd_byte *eline;
	  bfd_byte *elineend;

	  if (o->lineno_count == 0)
	    continue;

	  if (bfd_seek (input_bfd, o->line_filepos, SEEK_SET) != 0
	      || bfd_read (finfo->linenos, linesz, o->lineno_count,
			   input_bfd) != linesz * o->lineno_count)
	    return false;

	  offset = o->output_section->vma + o->output_offset - o->vma;
	  eline = finfo->linenos;
	  elineend = eline + linesz * o->lineno_count;
	  for (; eline < elineend; eline += linesz)
	    {
	      struct internal_lineno iline;

	      bfd_coff_swap_lineno_in (input_bfd, (PTR) eline, (PTR) &iline);

	      if (iline.l_lnno != 0)
		iline.l_addr.l_paddr += offset;
	      else if (iline.l_addr.l_symndx >= 0
		       && (iline.l_addr.l_symndx
			   < obj_raw_syment_count (input_bfd)))
		{
		  long indx;

		  indx = finfo->sym_indices[iline.l_addr.l_symndx];

		  if (indx < 0)
		    {
		      /* These line numbers are attached to a symbol
			 which we are stripping.  We should really
			 just discard the line numbers, but that would
			 be a pain because we have already counted
			 them.  */
		      indx = 0;
		    }
		  else
		    {
		      struct internal_syment is;
		      union internal_auxent ia;

		      /* Fix up the lnnoptr field in the aux entry of
			 the symbol.  It turns out that we can't do
			 this when we modify the symbol aux entries,
			 because gas sometimes screws up the lnnoptr
			 field and makes it an offset from the start
			 of the line numbers rather than an absolute
			 file index.  */
		      bfd_coff_swap_sym_in (output_bfd,
					    (PTR) (finfo->outsyms
						   + ((indx - syment_base)
						      * osymesz)),
					    (PTR) &is);
		      if ((ISFCN (is.n_type)
			   || is.n_sclass == C_BLOCK)
			  && is.n_numaux >= 1)
			{
			  PTR auxptr;

			  auxptr = (PTR) (finfo->outsyms
					  + ((indx - syment_base + 1)
					     * osymesz));
			  bfd_coff_swap_aux_in (output_bfd, auxptr,
						is.n_type, is.n_sclass,
						0, is.n_numaux, (PTR) &ia);
			  ia.x_sym.x_fcnary.x_fcn.x_lnnoptr =
			    (o->output_section->line_filepos
			     + o->output_section->lineno_count * linesz
			     + eline - finfo->linenos);
			  bfd_coff_swap_aux_out (output_bfd, (PTR) &ia,
						 is.n_type, is.n_sclass, 0,
						 is.n_numaux, auxptr);
			}
		    }

		  iline.l_addr.l_symndx = indx;
		}

	      bfd_coff_swap_lineno_out (output_bfd, (PTR) &iline, (PTR) eline);
	    }

	  if (bfd_seek (output_bfd,
			(o->output_section->line_filepos
			 + o->output_section->lineno_count * linesz),
			SEEK_SET) != 0
	      || bfd_write (finfo->linenos, linesz, o->lineno_count,
			    output_bfd) != linesz * o->lineno_count)
	    return false;

	  o->output_section->lineno_count += o->lineno_count;
	}
    }

  /* If we swapped out a C_FILE symbol, guess that the next C_FILE
     symbol will be the first symbol in the next input file.  In the
     normal case, this will save us from writing out the C_FILE symbol
     again.  */
  if (finfo->last_file_index != -1
      && finfo->last_file_index >= syment_base)
    {
      finfo->last_file.n_value = output_index;
      bfd_coff_swap_sym_out (output_bfd, (PTR) &finfo->last_file,
			     (PTR) (finfo->outsyms
 				    + ((finfo->last_file_index - syment_base)
 				       * osymesz)));
    }

  /* Write the modified symbols to the output file.  */
  if (outsym > finfo->outsyms)
    {
      if (bfd_seek (output_bfd,
		    obj_sym_filepos (output_bfd) + syment_base * osymesz,
		    SEEK_SET) != 0
	  || bfd_write (finfo->outsyms, outsym - finfo->outsyms, 1,
			output_bfd) != outsym - finfo->outsyms)
	return false;

      BFD_ASSERT ((obj_raw_syment_count (output_bfd)
		   + (outsym - finfo->outsyms) / osymesz)
		  == output_index);

      obj_raw_syment_count (output_bfd) = output_index;
    }

  /* Relocate the contents of each section.  */
  relsz = bfd_coff_relsz (input_bfd);
  for (o = input_bfd->sections; o != NULL; o = o->next)
    {
      if ((o->flags & SEC_HAS_CONTENTS) == 0)
	continue;

      if (! bfd_get_section_contents (input_bfd, o, finfo->contents,
				      (file_ptr) 0, o->_raw_size))
	return false;

      if ((o->flags & SEC_RELOC) != 0)
	{
	  int target_index;
	  struct internal_reloc *internal_relocs;
	  bfd_byte *erel;
	  bfd_byte *erel_end;
	  struct internal_reloc *irel;

	  /* Read in the relocs.  */
	  if (bfd_seek (input_bfd, o->rel_filepos, SEEK_SET) != 0
	      || (bfd_read (finfo->external_relocs, relsz, o->reloc_count,
			    input_bfd) != relsz * o->reloc_count))
	    return false;

	  /* If we are doing a relocateable link, we keep the swapped
	     in relocs in memory, and don't write them out until the
	     end of the link.  */
	  target_index = o->output_section->target_index;
	  if (! finfo->info->relocateable)
	    internal_relocs = finfo->internal_relocs;
	  else
	    internal_relocs = (finfo->section_info[target_index].relocs
			       + o->output_section->reloc_count);

	  /* Swap in the relocs.  */
	  erel = finfo->external_relocs;
	  erel_end = erel + relsz * o->reloc_count;
	  irel = internal_relocs;
	  for (; erel < erel_end; erel += relsz, irel++)
	    bfd_coff_swap_reloc_in (input_bfd, (PTR) erel, (PTR) irel);

	  /* Call processor specific code to relocate the section
             contents.  */
	  if (! bfd_coff_relocate_section (output_bfd, finfo->info,
					   input_bfd, o,
					   finfo->contents,
					   internal_relocs,
					   finfo->internal_syms,
					   finfo->sec_ptrs))
	    return false;

	  if (finfo->info->relocateable)
	    {
	      bfd_vma offset;
	      struct internal_reloc *irelend;
	      struct coff_link_hash_entry **rel_hash;

	      offset = o->output_section->vma + o->output_offset - o->vma;

	      irel = internal_relocs;
	      irelend = irel + o->reloc_count;
	      rel_hash = (finfo->section_info[target_index].rel_hashes
			  + o->output_section->reloc_count);
	      for (; irel < irelend; irel++, rel_hash++)
		{
		  struct coff_link_hash_entry *h;

		  *rel_hash = NULL;

		  /* Adjust the reloc address and symbol index.  */

		  irel->r_vaddr += offset;

		  if (irel->r_symndx == -1)
		    continue;

		  h = obj_coff_sym_hashes (input_bfd)[irel->r_symndx];
		  if (h != NULL)
		    {
		      /* This is a global symbol.  */
		      if (h->indx >= 0)
			irel->r_symndx = h->indx;
		      else
			{
			  /* This symbol is being written at the end
			     of the file, and we do not yet know the
			     symbol index.  We save the pointer to the
			     hash table entry in the rel_hash list.
			     We set the indx field to -2 to indicate
			     that this symbol must not be stripped.  */
			  *rel_hash = h;
			  h->indx = -2;
			}
		    }
		  else
		    {
		      long indx;

		      indx = finfo->sym_indices[irel->r_symndx];
		      if (indx != -1)
			irel->r_symndx = indx;
		      else
			{
			  struct internal_syment *is;
			  const char *name;
			  char buf[SYMNMLEN + 1];

			  /* This reloc is against a symbol we are
                             stripping.  It would be possible to
                             handle this case, but I don't think it's
                             worth it.  */
			  is = finfo->internal_syms + irel->r_symndx;

			  if (is->_n._n_n._n_zeroes == 0
			      && is->_n._n_n._n_offset != 0)
			    {
			      if (strings == NULL)
				{
				  strings = coff_read_string_table (input_bfd);
				  if (strings == NULL)
				    return false;
				}
			      name = strings + is->_n._n_n._n_offset;
			    }
			  else
			    {
			      memcpy (buf, is->_n._n_name, SYMNMLEN);
			      buf[SYMNMLEN] = '\0';
			      name = buf;
			    }

			  if (! ((*finfo->info->callbacks->unattached_reloc)
				 (finfo->info, name, input_bfd, o,
				  irel->r_vaddr)))
			    return false;
			}
		    }
		}

	      o->output_section->reloc_count += o->reloc_count;
	    }
	}

      /* Write out the modified section contents.  */
      if (! bfd_set_section_contents (output_bfd, o->output_section,
				      finfo->contents, o->output_offset,
				      (o->_cooked_size != 0
				       ? o->_cooked_size
				       : o->_raw_size)))
	return false;
    }

  if (! finfo->info->keep_memory)
    {
      if (! coff_link_free_symbols (input_bfd))
	return false;
    }

  return true;
}

/* Write out a global symbol.  Called via coff_link_hash_traverse.  */

static boolean
coff_write_global_sym (h, data)
     struct coff_link_hash_entry *h;
     PTR data;
{
  struct coff_final_link_info *finfo = (struct coff_final_link_info *) data;
  bfd *output_bfd;
  struct internal_syment isym;
  bfd_size_type symesz;
  unsigned int i;

  output_bfd = finfo->output_bfd;

  if (h->indx >= 0)
    return true;

  if (h->indx != -2
      && (finfo->info->strip == strip_all
	  || (finfo->info->strip == strip_some
	      && (bfd_hash_lookup (finfo->info->keep_hash,
				   h->root.root.string, false, false)
		  == NULL))))
    return true;

  switch (h->root.type)
    {
    default:
    case bfd_link_hash_new:
      abort ();
      return false;

    case bfd_link_hash_undefined:
    case bfd_link_hash_weak:
      isym.n_scnum = N_UNDEF;
      isym.n_value = 0;
      break;

    case bfd_link_hash_defined:
      {
	asection *sec;

	sec = h->root.u.def.section->output_section;
	if (bfd_is_abs_section (sec))
	  isym.n_scnum = N_ABS;
	else
	  isym.n_scnum = sec->target_index;
	isym.n_value = (h->root.u.def.value
			+ sec->vma
			+ h->root.u.def.section->output_offset);
      }
      break;

    case bfd_link_hash_common:
      isym.n_scnum = N_UNDEF;
      isym.n_value = h->root.u.c.size;
      break;

    case bfd_link_hash_indirect:
    case bfd_link_hash_warning:
      /* Just ignore these.  They can't be handled anyhow.  */
      return true;
    }

  if (strlen (h->root.root.string) <= SYMNMLEN)
    strncpy (isym._n._n_name, h->root.root.string, SYMNMLEN);
  else
    {
      boolean hash;
      bfd_size_type indx;

      hash = true;
      if ((output_bfd->flags & BFD_TRADITIONAL_FORMAT) != 0)
	hash = false;
      indx = _bfd_stringtab_add (finfo->strtab, h->root.root.string, hash,
				 false);
      if (indx == (bfd_size_type) -1)
	{
	  finfo->failed = true;
	  return false;
	}
      isym._n._n_n._n_zeroes = 0;
      isym._n._n_n._n_offset = STRING_SIZE_SIZE + indx;
    }

  isym.n_sclass = h->class;
  isym.n_type = h->type;

  if (isym.n_sclass == C_NULL)
    isym.n_sclass = C_EXT;

  isym.n_numaux = h->numaux;
  
  bfd_coff_swap_sym_out (output_bfd, (PTR) &isym, (PTR) finfo->outsyms);

  symesz = bfd_coff_symesz (output_bfd);

  if (bfd_seek (output_bfd,
		(obj_sym_filepos (output_bfd)
		 + obj_raw_syment_count (output_bfd) * symesz),
		SEEK_SET) != 0
      || bfd_write (finfo->outsyms, symesz, 1, output_bfd) != symesz)
    {
      finfo->failed = true;
      return false;
    }

  h->indx = obj_raw_syment_count (output_bfd);

  ++obj_raw_syment_count (output_bfd);

  /* Write out any associated aux entries.  There normally will be
     none.  If there are any, I have no idea how to modify them.  */
  for (i = 0; i < isym.n_numaux; i++)
    {
      bfd_coff_swap_aux_out (output_bfd, (PTR) (h->aux + i), isym.n_type,
			     isym.n_sclass, i, isym.n_numaux,
			     (PTR) finfo->outsyms);
      if (bfd_write (finfo->outsyms, symesz, 1, output_bfd) != symesz)
	{
	  finfo->failed = true;
	  return false;
	}
      ++obj_raw_syment_count (output_bfd);
    }

  return true;
}

/* Handle a link order which is supposed to generate a reloc.  */

static boolean
coff_reloc_link_order (output_bfd, finfo, output_section, link_order)
     bfd *output_bfd;
     struct coff_final_link_info *finfo;
     asection *output_section;
     struct bfd_link_order *link_order;
{
  reloc_howto_type *howto;
  struct internal_reloc *irel;
  struct coff_link_hash_entry **rel_hash_ptr;

  howto = bfd_reloc_type_lookup (output_bfd, link_order->u.reloc.p->reloc);
  if (howto == NULL)
    {
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  if (link_order->u.reloc.p->addend != 0)
    {
      bfd_size_type size;
      bfd_byte *buf;
      bfd_reloc_status_type rstat;
      boolean ok;

      size = bfd_get_reloc_size (howto);
      buf = (bfd_byte *) bfd_zmalloc (size);
      if (buf == NULL)
	{
	  bfd_set_error (bfd_error_no_memory);
	  return false;
	}

      rstat = _bfd_relocate_contents (howto, output_bfd,
				      link_order->u.reloc.p->addend, buf);
      switch (rstat)
	{
	case bfd_reloc_ok:
	  break;
	default:
	case bfd_reloc_outofrange:
	  abort ();
	case bfd_reloc_overflow:
	  if (! ((*finfo->info->callbacks->reloc_overflow)
		 (finfo->info,
		  (link_order->type == bfd_section_reloc_link_order
		   ? bfd_section_name (output_bfd,
				       link_order->u.reloc.p->u.section)
		   : link_order->u.reloc.p->u.name),
		  howto->name, link_order->u.reloc.p->addend,
		  (bfd *) NULL, (asection *) NULL, (bfd_vma) 0)))
	    {
	      free (buf);
	      return false;
	    }
	  break;
	}
      ok = bfd_set_section_contents (output_bfd, output_section, (PTR) buf,
				     (file_ptr) link_order->offset, size);
      free (buf);
      if (! ok)
	return false;
    }

  /* Store the reloc information in the right place.  It will get
     swapped and written out at the end of the final_link routine.  */

  irel = (finfo->section_info[output_section->target_index].relocs
	  + output_section->reloc_count);
  rel_hash_ptr = (finfo->section_info[output_section->target_index].rel_hashes
		  + output_section->reloc_count);

  memset (irel, 0, sizeof (struct internal_reloc));
  *rel_hash_ptr = NULL;

  irel->r_vaddr = output_section->vma + link_order->offset;

  if (link_order->type == bfd_section_reloc_link_order)
    {
      /* We need to somehow locate a symbol in the right section.  The
         symbol must either have a value of zero, or we must adjust
         the addend by the value of the symbol.  FIXME: Write this
         when we need it.  The old linker couldn't handle this anyhow.  */
      abort ();
      *rel_hash_ptr = NULL;
      irel->r_symndx = 0;
    }
  else
    {
      struct coff_link_hash_entry *h;

      h = coff_link_hash_lookup (coff_hash_table (finfo->info),
				 link_order->u.reloc.p->u.name,
				 false, false, true);
      if (h != NULL)
	{
	  if (h->indx >= 0)
	    irel->r_symndx = h->indx;
	  else
	    {
	      /* Set the index to -2 to force this symbol to get
		 written out.  */
	      h->indx = -2;
	      *rel_hash_ptr = h;
	      irel->r_symndx = 0;
	    }
	}
      else
	{
	  if (! ((*finfo->info->callbacks->unattached_reloc)
		 (finfo->info, link_order->u.reloc.p->u.name, (bfd *) NULL,
		  (asection *) NULL, (bfd_vma) 0)))
	    return false;
	  irel->r_symndx = 0;
	}
    }

  /* FIXME: Is this always right?  */
  irel->r_type = howto->type;

  /* r_size is only used on the RS/6000, which needs its own linker
     routines anyhow.  r_extern is only used for ECOFF.  */

  /* FIXME: What is the right value for r_offset?  Is zero OK?  */

  ++output_section->reloc_count;

  return true;
}

/* A basic reloc handling routine which may be used by processors with
   simple relocs.  */

boolean
_bfd_coff_generic_relocate_section (output_bfd, info, input_bfd,
				    input_section, contents, relocs, syms,
				    sections)
     bfd *output_bfd;
     struct bfd_link_info *info;
     bfd *input_bfd;
     asection *input_section;
     bfd_byte *contents;
     struct internal_reloc *relocs;
     struct internal_syment *syms;
     asection **sections;
{
  struct internal_reloc *rel;
  struct internal_reloc *relend;

  rel = relocs;
  relend = rel + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      long symndx;
      struct coff_link_hash_entry *h;
      struct internal_syment *sym;
      bfd_vma addend;
      bfd_vma val;
      reloc_howto_type *howto;
      bfd_reloc_status_type rstat;

      symndx = rel->r_symndx;

      if (symndx == -1)
	{
	  h = NULL;
	  sym = NULL;
	}
      else
	{    
	  h = obj_coff_sym_hashes (input_bfd)[symndx];
	  sym = syms + symndx;
	}

      /* COFF treats common symbols in one of two ways.  Either the
         size of the symbol is included in the section contents, or it
         is not.  We assume that the size is not included, and force
         the rtype_to_howto function to adjust the addend as needed.  */
      if (sym != NULL && sym->n_scnum != 0)
	addend = - sym->n_value;
      else
	addend = 0;

      howto = bfd_coff_rtype_to_howto (input_bfd, input_section, rel, h,
				       sym, &addend);
      if (howto == NULL)
	return false;

      val = 0;

      if (h == NULL)
	{
	  asection *sec;

	  if (symndx == -1)
	    {
	      sec = bfd_abs_section_ptr;
	      val = 0;
	    }
	  else
	    {
	      sec = sections[symndx];
	      val = (sec->output_section->vma
		     + sec->output_offset
		     + sym->n_value
		     - sec->vma);
	    }
	}
      else
	{
	  if (h->root.type == bfd_link_hash_defined)
	    {
	      asection *sec;

	      sec = h->root.u.def.section;
	      val = (h->root.u.def.value
		     + sec->output_section->vma
		     + sec->output_offset);
	    }
	  else if (! info->relocateable)
	    {
	      if (! ((*info->callbacks->undefined_symbol)
		     (info, h->root.root.string, input_bfd, input_section,
		      rel->r_vaddr - input_section->vma)))
		return false;
	    }
	}

      rstat = _bfd_final_link_relocate (howto, input_bfd, input_section,
					contents,
					rel->r_vaddr - input_section->vma,
					val, addend);

      switch (rstat)
	{
	default:
	  abort ();
	case bfd_reloc_ok:
	  break;
	case bfd_reloc_overflow:
	  {
	    const char *name;
	    char buf[SYMNMLEN + 1];

	    if (symndx == -1)
	      name = "*ABS*";
	    else if (h != NULL)
	      name = h->root.root.string;
	    else if (sym->_n._n_n._n_zeroes == 0
		     && sym->_n._n_n._n_offset != 0)
	      name = obj_coff_strings (input_bfd) + sym->_n._n_n._n_offset;
	    else
	      {
		strncpy (buf, sym->_n._n_name, SYMNMLEN);
		buf[SYMNMLEN] = '\0';
		name = buf;
	      }

	    if (! ((*info->callbacks->reloc_overflow)
		   (info, name, howto->name, (bfd_vma) 0, input_bfd,
		    input_section, rel->r_vaddr - input_section->vma)))
	      return false;
	  }
	}
    }

  return true;
}
