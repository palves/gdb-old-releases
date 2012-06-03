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

typedef struct coff_ptr_struct
{

        /* Remembers the offset from the first symbol in the file for
          this symbol. Generated by coff_renumber_symbols. */
unsigned int offset;

        /* Should the value of this symbol be renumbered.  Used for
          XCOFF C_BSTAT symbols.  Set by coff_slurp_symbol_table.  */
unsigned int fix_value : 1;

        /* Should the tag field of this symbol be renumbered.
          Created by coff_pointerize_aux. */
unsigned int fix_tag : 1;

        /* Should the endidx field of this symbol be renumbered.
          Created by coff_pointerize_aux. */
unsigned int fix_end : 1;

        /* Should the x_csect.x_scnlen field be renumbered.
          Created by coff_slurp_symbol_table. */
unsigned int fix_scnlen : 1;

        /* The container for the symbol structure as read and translated
           from the file. */

union {
   union internal_auxent auxent;
   struct internal_syment syment;
 } u;
} combined_entry_type;


 /* Each canonical asymbol really looks like this: */

typedef struct coff_symbol_struct
{
    /* The actual symbol which the rest of BFD works with */
asymbol symbol;

    /* A pointer to the hidden information for this symbol */
combined_entry_type *native;

    /* A pointer to the linenumber information for this symbol */
struct lineno_cache_entry *lineno;

    /* Have the line numbers been relocated yet ? */
boolean done_lineno;
} coff_symbol_type;
typedef struct
{
  void (*_bfd_coff_swap_aux_in) PARAMS ((
       bfd            *abfd,
       PTR             ext,
       int             type,
       int             class,
       int             indaux,
       int             numaux,
       PTR             in));

  void (*_bfd_coff_swap_sym_in) PARAMS ((
       bfd            *abfd ,
       PTR             ext,
       PTR             in));

  void (*_bfd_coff_swap_lineno_in) PARAMS ((
       bfd            *abfd,
       PTR            ext,
       PTR             in));

 unsigned int (*_bfd_coff_swap_aux_out) PARAMS ((
       bfd   	*abfd,
       PTR	in,
       int    	type,
       int    	class,
       int     indaux,
       int     numaux,
       PTR    	ext));

 unsigned int (*_bfd_coff_swap_sym_out) PARAMS ((
      bfd      *abfd,
      PTR	in,
      PTR	ext));

 unsigned int (*_bfd_coff_swap_lineno_out) PARAMS ((
      	bfd   	*abfd,
      	PTR	in,
	PTR	ext));

 unsigned int (*_bfd_coff_swap_reloc_out) PARAMS ((
      	bfd     *abfd,
     	PTR	src,
	PTR	dst));

 unsigned int (*_bfd_coff_swap_filehdr_out) PARAMS ((
      	bfd  	*abfd,
	PTR 	in,
	PTR 	out));

 unsigned int (*_bfd_coff_swap_aouthdr_out) PARAMS ((
      	bfd 	*abfd,
	PTR 	in,
	PTR	out));

 unsigned int (*_bfd_coff_swap_scnhdr_out) PARAMS ((
      	bfd  	*abfd,
      	PTR	in,
	PTR	out));

 unsigned int _bfd_filhsz;
 unsigned int _bfd_aoutsz;
 unsigned int _bfd_scnhsz;
 unsigned int _bfd_symesz;
 unsigned int _bfd_auxesz;
 unsigned int _bfd_relsz;
 unsigned int _bfd_linesz;
 boolean _bfd_coff_long_filenames;
 void (*_bfd_coff_swap_filehdr_in) PARAMS ((
       bfd     *abfd,
       PTR     ext,
       PTR     in));
 void (*_bfd_coff_swap_aouthdr_in) PARAMS ((
       bfd     *abfd,
       PTR     ext,
       PTR     in));
 void (*_bfd_coff_swap_scnhdr_in) PARAMS ((
       bfd     *abfd,
       PTR     ext,
       PTR     in));
 void (*_bfd_coff_swap_reloc_in) PARAMS ((
       bfd     *abfd,
       PTR     ext,
       PTR     in));
 boolean (*_bfd_coff_bad_format_hook) PARAMS ((
       bfd     *abfd,
       PTR     internal_filehdr));
 boolean (*_bfd_coff_set_arch_mach_hook) PARAMS ((
       bfd     *abfd,
       PTR     internal_filehdr));
 PTR (*_bfd_coff_mkobject_hook) PARAMS ((
       bfd     *abfd,
       PTR     internal_filehdr,
       PTR     internal_aouthdr));
 flagword (*_bfd_styp_to_sec_flags_hook) PARAMS ((
       bfd     *abfd,
       PTR     internal_scnhdr,
       const char *name));
 asection *(*_bfd_make_section_hook) PARAMS ((
       bfd     *abfd,
       char    *name));
 void (*_bfd_set_alignment_hook) PARAMS ((
       bfd     *abfd,
       asection *sec,
       PTR     internal_scnhdr));
 boolean (*_bfd_coff_slurp_symbol_table) PARAMS ((
       bfd     *abfd));
 boolean (*_bfd_coff_symname_in_debug) PARAMS ((
       bfd     *abfd,
       struct internal_syment *sym));
 void (*_bfd_coff_reloc16_extra_cases) PARAMS ((
       bfd     *abfd,
       struct bfd_link_info *link_info,
       struct bfd_link_order *link_order,
       arelent *reloc,
       bfd_byte *data,
       unsigned int *src_ptr,
       unsigned int *dst_ptr));
 int (*_bfd_coff_reloc16_estimate) PARAMS ((
       bfd *abfd,
       asection *input_section,
       arelent *r,
       unsigned int shrink,
       struct bfd_link_info *link_info));
 boolean (*_bfd_coff_sym_is_global) PARAMS ((
       bfd *abfd,
       struct internal_syment *));
 void (*_bfd_coff_compute_section_file_positions) PARAMS ((
       bfd *abfd));
 boolean (*_bfd_coff_relocate_section) PARAMS ((
       bfd *output_bfd,
       struct bfd_link_info *info,
       bfd *input_bfd,
       asection *input_section,
       bfd_byte *contents,
       struct internal_reloc *relocs,
       struct internal_syment *syms,
       asection **sections));
 reloc_howto_type *(*_bfd_coff_rtype_to_howto) PARAMS ((
       bfd *abfd,
       asection *sec,
       struct internal_reloc *rel,
       struct coff_link_hash_entry *h,
       struct internal_syment *sym,
       bfd_vma *addendp));

} bfd_coff_backend_data;

#define coff_backend_info(abfd) ((bfd_coff_backend_data *) (abfd)->xvec->backend_data)

#define bfd_coff_swap_aux_in(a,e,t,c,ind,num,i) \
        ((coff_backend_info (a)->_bfd_coff_swap_aux_in) (a,e,t,c,ind,num,i))

#define bfd_coff_swap_sym_in(a,e,i) \
        ((coff_backend_info (a)->_bfd_coff_swap_sym_in) (a,e,i))

#define bfd_coff_swap_lineno_in(a,e,i) \
        ((coff_backend_info ( a)->_bfd_coff_swap_lineno_in) (a,e,i))

#define bfd_coff_swap_reloc_out(abfd, i, o) \
        ((coff_backend_info (abfd)->_bfd_coff_swap_reloc_out) (abfd, i, o))

#define bfd_coff_swap_lineno_out(abfd, i, o) \
        ((coff_backend_info (abfd)->_bfd_coff_swap_lineno_out) (abfd, i, o))

#define bfd_coff_swap_aux_out(a,i,t,c,ind,num,o) \
        ((coff_backend_info (a)->_bfd_coff_swap_aux_out) (a,i,t,c,ind,num,o))

#define bfd_coff_swap_sym_out(abfd, i,o) \
        ((coff_backend_info (abfd)->_bfd_coff_swap_sym_out) (abfd, i, o))

#define bfd_coff_swap_scnhdr_out(abfd, i,o) \
        ((coff_backend_info (abfd)->_bfd_coff_swap_scnhdr_out) (abfd, i, o))

#define bfd_coff_swap_filehdr_out(abfd, i,o) \
        ((coff_backend_info (abfd)->_bfd_coff_swap_filehdr_out) (abfd, i, o))

#define bfd_coff_swap_aouthdr_out(abfd, i,o) \
        ((coff_backend_info (abfd)->_bfd_coff_swap_aouthdr_out) (abfd, i, o))

#define bfd_coff_filhsz(abfd) (coff_backend_info (abfd)->_bfd_filhsz)
#define bfd_coff_aoutsz(abfd) (coff_backend_info (abfd)->_bfd_aoutsz)
#define bfd_coff_scnhsz(abfd) (coff_backend_info (abfd)->_bfd_scnhsz)
#define bfd_coff_symesz(abfd) (coff_backend_info (abfd)->_bfd_symesz)
#define bfd_coff_auxesz(abfd) (coff_backend_info (abfd)->_bfd_auxesz)
#define bfd_coff_relsz(abfd)  (coff_backend_info (abfd)->_bfd_relsz)
#define bfd_coff_linesz(abfd) (coff_backend_info (abfd)->_bfd_linesz)
#define bfd_coff_long_filenames(abfd) (coff_backend_info (abfd)->_bfd_coff_long_filenames)
#define bfd_coff_swap_filehdr_in(abfd, i,o) \
        ((coff_backend_info (abfd)->_bfd_coff_swap_filehdr_in) (abfd, i, o))

#define bfd_coff_swap_aouthdr_in(abfd, i,o) \
        ((coff_backend_info (abfd)->_bfd_coff_swap_aouthdr_in) (abfd, i, o))

#define bfd_coff_swap_scnhdr_in(abfd, i,o) \
        ((coff_backend_info (abfd)->_bfd_coff_swap_scnhdr_in) (abfd, i, o))

#define bfd_coff_swap_reloc_in(abfd, i, o) \
        ((coff_backend_info (abfd)->_bfd_coff_swap_reloc_in) (abfd, i, o))

#define bfd_coff_bad_format_hook(abfd, filehdr) \
        ((coff_backend_info (abfd)->_bfd_coff_bad_format_hook) (abfd, filehdr))

#define bfd_coff_set_arch_mach_hook(abfd, filehdr)\
        ((coff_backend_info (abfd)->_bfd_coff_set_arch_mach_hook) (abfd, filehdr))
#define bfd_coff_mkobject_hook(abfd, filehdr, aouthdr)\
        ((coff_backend_info (abfd)->_bfd_coff_mkobject_hook) (abfd, filehdr, aouthdr))

#define bfd_coff_styp_to_sec_flags_hook(abfd, scnhdr, name)\
        ((coff_backend_info (abfd)->_bfd_styp_to_sec_flags_hook) (abfd, scnhdr, name))

#define bfd_coff_make_section_hook(abfd, name)\
        ((coff_backend_info (abfd)->_bfd_make_section_hook) (abfd, name))

#define bfd_coff_set_alignment_hook(abfd, sec, scnhdr)\
        ((coff_backend_info (abfd)->_bfd_set_alignment_hook) (abfd, sec, scnhdr))

#define bfd_coff_slurp_symbol_table(abfd)\
        ((coff_backend_info (abfd)->_bfd_coff_slurp_symbol_table) (abfd))

#define bfd_coff_symname_in_debug(abfd, sym)\
        ((coff_backend_info (abfd)->_bfd_coff_symname_in_debug) (abfd, sym))

#define bfd_coff_reloc16_extra_cases(abfd, link_info, link_order, reloc, data, src_ptr, dst_ptr)\
        ((coff_backend_info (abfd)->_bfd_coff_reloc16_extra_cases)\
         (abfd, link_info, link_order, reloc, data, src_ptr, dst_ptr))

#define bfd_coff_reloc16_estimate(abfd, section, reloc, shrink, link_info)\
        ((coff_backend_info (abfd)->_bfd_coff_reloc16_estimate)\
         (abfd, section, reloc, shrink, link_info))

#define bfd_coff_sym_is_global(abfd, sym)\
        ((coff_backend_info (abfd)->_bfd_coff_sym_is_global)\
         (abfd, sym))

#define bfd_coff_compute_section_file_positions(abfd)\
        ((coff_backend_info (abfd)->_bfd_coff_compute_section_file_positions)\
         (abfd))

#define bfd_coff_relocate_section(obfd,info,ibfd,o,con,rel,isyms,secs)\
        ((coff_backend_info (ibfd)->_bfd_coff_relocate_section)\
         (obfd, info, ibfd, o, con, rel, isyms, secs))
#define bfd_coff_rtype_to_howto(abfd, sec, rel, h, sym, addendp)\
        ((coff_backend_info (abfd)->_bfd_coff_rtype_to_howto)\
         (abfd, sec, rel, h, sym, addendp))

