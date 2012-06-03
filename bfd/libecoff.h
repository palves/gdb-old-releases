/* BFD ECOFF object file private structure.
   Copyright (C) 1993 Free Software Foundation, Inc.
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

#include "bfdlink.h"

#ifndef ECOFF_H
#include "coff/ecoff.h"
#endif

/* This is the backend information kept for ECOFF files.  This
   structure is constant for a particular backend.  The first element
   is the COFF backend data structure, so that ECOFF targets can use
   the generic COFF code.  */

#define ecoff_backend(abfd) \
  ((struct ecoff_backend_data *) (abfd)->xvec->backend_data)

struct ecoff_backend_data
{
  /* COFF backend information.  This must be the first field.  */
  bfd_coff_backend_data coff;
  /* Supported architecture.  */
  enum bfd_architecture arch;
  /* Initial portion of armap string.  */
  const char *armap_start;
  /* The page boundary used to align sections in a demand-paged
     executable file.  E.g., 0x1000.  */
  bfd_vma round;
  /* True if the .rdata section is part of the text segment, as on the
     Alpha.  False if .rdata is part of the data segment, as on the
     MIPS.  */
  boolean rdata_in_text;
  /* Bitsize of constructor entries.  */
  unsigned int constructor_bitsize;
  /* Reloc to use for constructor entries.  */
  CONST struct reloc_howto_struct *constructor_reloc;
  /* How to swap debugging information.  */
  struct ecoff_debug_swap debug_swap;
  /* External reloc size.  */
  bfd_size_type external_reloc_size;
  /* Reloc swapping functions.  */
  void (*swap_reloc_in) PARAMS ((bfd *, PTR, struct internal_reloc *));
  void (*swap_reloc_out) PARAMS ((bfd *, const struct internal_reloc *, PTR));
  /* Backend reloc tweaking.  */
  void (*adjust_reloc_in) PARAMS ((bfd *, const struct internal_reloc *,
				   arelent *));
  void (*adjust_reloc_out) PARAMS ((bfd *, const arelent *,
				    struct internal_reloc *));
  /* Relocate section contents while linking.  */
  boolean (*relocate_section) PARAMS ((bfd *output_bfd, struct bfd_link_info *,
				       bfd *input_bfd, asection *input_section,
				       bfd_byte *contents,
				       PTR external_relocs));
};

/* This is the target specific information kept for ECOFF files.  */

#define ecoff_data(abfd) ((abfd)->tdata.ecoff_obj_data)

typedef struct ecoff_tdata
{
  /* The reloc file position, set by
     ecoff_compute_section_file_positions.  */
  file_ptr reloc_filepos;

  /* The symbol table file position, set by ecoff_mkobject_hook.  */
  file_ptr sym_filepos;

  /* The start and end of the text segment.  Only valid for an
     existing file, not for one we are creating.  */
  unsigned long text_start;
  unsigned long text_end;

  /* The cached gp value.  This is used when relocating.  */
  bfd_vma gp;

  /* The maximum size of objects to optimize using gp.  This is
     typically set by the -G option to the compiler, assembler or
     linker.  */
  int gp_size;

  /* The register masks.  When linking, all the masks found in the
     input files are combined into the masks of the output file.
     These are not all used for all targets, but that's OK, because
     the relevant ones are the only ones swapped in and out.  */
  unsigned long gprmask;
  unsigned long fprmask;
  unsigned long cprmask[4];

  /* The ECOFF symbolic debugging information.  */
  struct ecoff_debug_info debug_info;

  /* The unswapped ECOFF symbolic information.  */
  PTR raw_syments;

  /* The canonical BFD symbols.  */
  struct ecoff_symbol_struct *canonical_symbols;

  /* A mapping from external symbol numbers to entries in the linker
     hash table, used when linking.  */
  struct ecoff_link_hash_entry **sym_hashes;

  /* A mapping from reloc symbol indices to sections, used when
     linking.  */
  asection **symndx_to_section;

} ecoff_data_type;

/* Each canonical asymbol really looks like this.  */

typedef struct ecoff_symbol_struct
{
  /* The actual symbol which the rest of BFD works with */
  asymbol symbol;

  /* The fdr for this symbol.  */
  FDR *fdr;

  /* true if this is a local symbol rather than an external one.  */
  boolean local;

  /* A pointer to the unswapped hidden information for this symbol.
     This is either a struct sym_ext or a struct ext_ext, depending on
     the value of the local field above.  */
  PTR native;
} ecoff_symbol_type;

/* We take the address of the first element of a asymbol to ensure that the
   macro is only ever applied to an asymbol.  */
#define ecoffsymbol(asymbol) ((ecoff_symbol_type *) (&((asymbol)->the_bfd)))

/* This is a hack borrowed from coffcode.h; we need to save the index
   of an external symbol when we write it out so that can set the
   symbol index correctly when we write out the relocs.  */
#define ecoff_get_sym_index(symbol) ((unsigned long) (symbol)->udata)
#define ecoff_set_sym_index(symbol, idx) ((symbol)->udata = (PTR) (idx))

/* ECOFF linker hash table entries.  */

struct ecoff_link_hash_entry
{
  struct bfd_link_hash_entry root;
  /* Symbol index in output file.  */
  long indx;
  /* BFD that ext field value came from.  */
  bfd *abfd;
  /* ECOFF external symbol information.  */
  EXTR esym;
};

/* ECOFF linker hash table.  */

struct ecoff_link_hash_table
{
  struct bfd_link_hash_table root;
};

/* Make an ECOFF object.  */
extern boolean ecoff_mkobject PARAMS ((bfd *));

/* Read in the ECOFF symbolic debugging information.  */
extern boolean ecoff_slurp_symbolic_info PARAMS ((bfd *));

/* Generic ECOFF BFD backend vectors.  */
extern asymbol *ecoff_make_empty_symbol PARAMS ((bfd *abfd));
extern unsigned int ecoff_get_symtab_upper_bound PARAMS ((bfd *abfd));
extern unsigned int ecoff_get_symtab PARAMS ((bfd *abfd,
					      asymbol **alocation));
extern void ecoff_get_symbol_info PARAMS ((bfd *abfd,
					   asymbol *symbol,
					   symbol_info *ret));
extern void ecoff_print_symbol PARAMS ((bfd *abfd, PTR filep,
					asymbol *symbol,
					bfd_print_symbol_type how));
extern unsigned int ecoff_canonicalize_reloc PARAMS ((bfd *abfd,
						      asection *section,
						      arelent **relptr,
						      asymbol **symbols));
extern boolean ecoff_find_nearest_line PARAMS ((bfd *abfd,
						asection *section,
						asymbol **symbols,
						bfd_vma offset,
						CONST char **filename_ptr,
						CONST char **fnname_ptr,
						unsigned int *retline_ptr));
extern boolean ecoff_set_arch_mach PARAMS ((bfd *abfd,
					    enum bfd_architecture arch,
					    unsigned long machine));
extern int ecoff_sizeof_headers PARAMS ((bfd *abfd, boolean reloc));
extern boolean ecoff_set_section_contents PARAMS ((bfd *abfd,
						   asection *section,
						   PTR location,
						   file_ptr offset,
						   bfd_size_type count));
extern boolean ecoff_get_section_contents PARAMS ((bfd *abfd,
						   asection *section,
						   PTR location,
						   file_ptr offset,
						   bfd_size_type count));
extern boolean ecoff_write_object_contents PARAMS ((bfd *abfd));
extern boolean ecoff_slurp_armap PARAMS ((bfd *abfd));
extern boolean ecoff_write_armap PARAMS ((bfd *abfd, unsigned int elength,
					  struct orl *map,
					  unsigned int orl_count,
					  int stridx));
#define ecoff_slurp_extended_name_table	_bfd_slurp_extended_name_table
extern bfd_target *ecoff_archive_p PARAMS ((bfd *abfd));
#define ecoff_get_lineno \
  ((alent *(*) PARAMS ((bfd *, asymbol *))) bfd_nullvoidptr)
#define ecoff_truncate_arname		bfd_dont_truncate_arname
#define ecoff_openr_next_archived_file	bfd_generic_openr_next_archived_file
#define ecoff_generic_stat_arch_elt	bfd_generic_stat_arch_elt
#define ecoff_get_reloc_upper_bound	coff_get_reloc_upper_bound
#define	ecoff_close_and_cleanup		bfd_generic_close_and_cleanup
#define ecoff_bfd_debug_info_start	bfd_void
#define ecoff_bfd_debug_info_end	bfd_void
#define ecoff_bfd_debug_info_accumulate	\
  ((void (*) PARAMS ((bfd *, struct sec *))) bfd_void)
#define ecoff_bfd_relax_section		bfd_generic_relax_section
#define ecoff_bfd_make_debug_symbol \
  ((asymbol *(*) PARAMS ((bfd *, void *, unsigned long))) bfd_nullvoidptr)
extern struct bfd_link_hash_table *ecoff_bfd_link_hash_table_create
  PARAMS ((bfd *));
extern boolean ecoff_bfd_link_add_symbols
  PARAMS ((bfd *, struct bfd_link_info *));
extern boolean ecoff_bfd_final_link PARAMS ((bfd *, struct bfd_link_info *));

/* Hook functions for the generic COFF section reading code.  */
extern PTR ecoff_mkobject_hook PARAMS ((bfd *, PTR filehdr, PTR aouthdr));
extern asection *ecoff_make_section_hook PARAMS ((bfd *abfd, char *name));
extern boolean ecoff_new_section_hook PARAMS ((bfd *abfd,
					       asection *section));
#define ecoff_set_alignment_hook \
  ((void (*) PARAMS ((bfd *, asection *, PTR))) bfd_void)
extern boolean ecoff_set_arch_mach_hook PARAMS ((bfd *abfd, PTR filehdr));
extern long ecoff_sec_to_styp_flags PARAMS ((CONST char *name,
					     flagword flags));
extern flagword ecoff_styp_to_sec_flags PARAMS ((bfd *abfd, PTR hdr));
extern boolean ecoff_slurp_symbol_table PARAMS ((bfd *abfd));
