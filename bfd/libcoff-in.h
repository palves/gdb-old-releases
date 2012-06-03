/* BFD COFF object file private structure.
   Copyright (C) 1990, 91, 92, 93, 94, 1995 Free Software Foundation, Inc.
   Written by Cygnus Support.

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

#include "bfdlink.h"

/* Object file tdata; access macros */

#define coff_data(bfd)		((bfd)->tdata.coff_obj_data)
#define exec_hdr(bfd)		(coff_data(bfd)->hdr)
#define obj_symbols(bfd)	(coff_data(bfd)->symbols)
#define	obj_sym_filepos(bfd)	(coff_data(bfd)->sym_filepos)

#define obj_relocbase(bfd)	(coff_data(bfd)->relocbase)
#define obj_raw_syments(bfd)	(coff_data(bfd)->raw_syments)
#define obj_raw_syment_count(bfd)	(coff_data(bfd)->raw_syment_count)
#define obj_convert(bfd)	(coff_data(bfd)->conversion_table)
#define obj_conv_table_size(bfd) (coff_data(bfd)->conv_table_size)

#define obj_coff_external_syms(bfd) (coff_data (bfd)->external_syms)
#define obj_coff_strings(bfd)	(coff_data (bfd)->strings)
#define obj_coff_sym_hashes(bfd) (coff_data (bfd)->sym_hashes)

/* `Tdata' information kept for COFF files.  */

typedef struct coff_tdata
{
  struct   coff_symbol_struct *symbols;	/* symtab for input bfd */
  unsigned int *conversion_table;
  int conv_table_size;
  file_ptr sym_filepos;

  struct coff_ptr_struct *raw_syments;
  unsigned int raw_syment_count;

  /* These are only valid once writing has begun */
  long int relocbase;

  /* These members communicate important constants about the symbol table
     to GDB's symbol-reading code.  These `constants' unfortunately vary
     from coff implementation to implementation...  */
  unsigned local_n_btmask;
  unsigned local_n_btshft;
  unsigned local_n_tmask;
  unsigned local_n_tshift;
  unsigned local_symesz;
  unsigned local_auxesz;
  unsigned local_linesz;

  /* Used by the COFF backend linker.  */
  PTR external_syms;
  char *strings;
  struct coff_link_hash_entry **sym_hashes;
} coff_data_type;

/* We take the address of the first element of a asymbol to ensure that the
 * macro is only ever applied to an asymbol.  */
#define coffsymbol(asymbol) ((coff_symbol_type *)(&((asymbol)->the_bfd)))

/* COFF linker hash table entries.  */

struct coff_link_hash_entry
{
  struct bfd_link_hash_entry root;

  /* Symbol index in output file.  Set to -1 initially.  Set to -2 if
     there is a reloc against this symbol.  */
  long indx;

  /* Symbol type.  */
  unsigned short type;

  /* Symbol class.  */
  unsigned char class;

  /* Number of auxiliary entries.  */
  char numaux;

  /* BFD to take auxiliary entries from.  */
  bfd *auxbfd;

  /* Pointer to array of auxiliary entries, if any.  */
  union internal_auxent *aux;
};

/* COFF linker hash table.  */

struct coff_link_hash_table
{
  struct bfd_link_hash_table root;
};

/* Look up an entry in a COFF linker hash table.  */

#define coff_link_hash_lookup(table, string, create, copy, follow)	\
  ((struct coff_link_hash_entry *)					\
   bfd_link_hash_lookup (&(table)->root, (string), (create),		\
			 (copy), (follow)))

/* Traverse a COFF linker hash table.  */

#define coff_link_hash_traverse(table, func, info)			\
  (bfd_link_hash_traverse						\
   (&(table)->root,							\
    (boolean (*) PARAMS ((struct bfd_link_hash_entry *, PTR))) (func),	\
    (info)))

/* Get the COFF linker hash table from a link_info structure.  */

#define coff_hash_table(p) ((struct coff_link_hash_table *) ((p)->hash))

/* Functions in coffgen.c.  */
extern const bfd_target *coff_object_p PARAMS ((bfd *));
extern struct sec *coff_section_from_bfd_index PARAMS ((bfd *, int));
extern long coff_get_symtab_upper_bound PARAMS ((bfd *));
extern long coff_get_symtab PARAMS ((bfd *, asymbol **));
extern int coff_count_linenumbers PARAMS ((bfd *));
extern struct coff_symbol_struct *coff_symbol_from PARAMS ((bfd *, asymbol *));
extern boolean coff_renumber_symbols PARAMS ((bfd *));
extern void coff_mangle_symbols PARAMS ((bfd *));
extern boolean coff_write_symbols PARAMS ((bfd *));
extern boolean coff_write_linenumbers PARAMS ((bfd *));
extern alent *coff_get_lineno PARAMS ((bfd *, asymbol *));
extern asymbol *coff_section_symbol PARAMS ((bfd *, char *));
extern struct coff_ptr_struct *coff_get_normalized_symtab PARAMS ((bfd *));
extern long coff_get_reloc_upper_bound PARAMS ((bfd *, sec_ptr));
extern asymbol *coff_make_empty_symbol PARAMS ((bfd *));
extern void coff_print_symbol PARAMS ((bfd *, PTR filep, asymbol *,
				       bfd_print_symbol_type how));
extern void coff_get_symbol_info PARAMS ((bfd *, asymbol *,
					  symbol_info *ret));
extern asymbol *coff_bfd_make_debug_symbol PARAMS ((bfd *, PTR,
						    unsigned long));
extern boolean coff_find_nearest_line PARAMS ((bfd *,
					       asection *,
					       asymbol **,
					       bfd_vma offset,
					       CONST char **filename_ptr,
					       CONST char **functionname_ptr,
					       unsigned int *line_ptr));
extern int coff_sizeof_headers PARAMS ((bfd *, boolean reloc));
extern boolean bfd_coff_reloc16_relax_section
  PARAMS ((bfd *, asection *, struct bfd_link_info *, boolean *));
extern bfd_byte *bfd_coff_reloc16_get_relocated_section_contents
  PARAMS ((bfd *, struct bfd_link_info *, struct bfd_link_order *,
	   bfd_byte *, boolean relocateable, asymbol **));
extern bfd_vma bfd_coff_reloc16_get_value PARAMS ((arelent *,
						   struct bfd_link_info *,
						   asection *));
extern void bfd_perform_slip PARAMS ((bfd *abfd, unsigned int slip,
				      asection *input_section,
				      bfd_vma val));

/* Functions in cofflink.c.  */

extern struct bfd_link_hash_table *_bfd_coff_link_hash_table_create
  PARAMS ((bfd *));
extern boolean _bfd_coff_link_add_symbols
  PARAMS ((bfd *, struct bfd_link_info *));
extern boolean _bfd_coff_final_link
  PARAMS ((bfd *, struct bfd_link_info *));
extern boolean _bfd_coff_generic_relocate_section
  PARAMS ((bfd *, struct bfd_link_info *, bfd *, asection *, bfd_byte *,
	   struct internal_reloc *, struct internal_syment *, asection **));

/* And more taken from the source .. */

