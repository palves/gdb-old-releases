/* HP PA-RISC SOM object file format:  definitions internal to BFD.
   Copyright (C) 1990-1991 Free Software Foundation, Inc.

   Contributed by the Center for Software Science at the
   University of Utah (pa-gdb-bugs@cs.utah.edu).

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

#ifndef _SOM_H
#define _SOM_H

#include "../bfd/sysdep.h"

#include <a.out.h>


#ifdef HOST_HPPABSD
/* BSD uses a completely different scheme for object file identification.
   so for now, define _PA_RISC_ID to accept any random value for a model
   number.  */
#undef _PA_RISC_ID
#define _PA_RISC_ID(__m_num) 1
#endif /* HOST_HPPABSD */

#define FILE_HDR_SIZE sizeof(struct header)
#define AUX_HDR_SIZE sizeof(struct som_exec_auxhdr)

typedef struct som_symbol
  {
    asymbol symbol;
    short desc;
    char other;
    unsigned char type;
    unsigned int som_type;

    /* Structured like the ELF tc_data union.  Allows more code sharing
       in GAS this way.  */
    union
      {
	unsigned int hppa_arg_reloc;
	PTR any;
      }
    tc_data;

    /* Index of this symbol in the symbol table.  Only used when
       building relocation streams for incomplete objects.  */
    int index;

    /* How many times this symbol is used in a relocation.  By sorting
       the symbols from most used to least used we can significantly
       reduce the size of the relocation stream for incomplete objects.  */
    int reloc_count;

    /* The unwind descriptor bits associated with R_ENTRY relocations
       for functions (not enough room in a BFD reloc to store all the
       information, so we tack it onto the symbol associated with the
       function.  */
    char *unwind;
  }
som_symbol_type;

struct somdata
  {
    struct header *file_hdr;
    struct copyright_aux_hdr *copyright_aux_hdr;
    struct user_string_aux_hdr *version_aux_hdr;
    som_symbol_type *symtab;
    char *stringtab;

    /* We remember these offsets so that after check_file_format, we have
       no dependencies on the particular format of the exec_hdr.  */

    file_ptr sym_filepos;
    file_ptr str_filepos;
    file_ptr reloc_filepos;

    unsigned stringtab_size;
  };

struct som_data_struct
  {
    struct somdata a;
  };

/* Used to keep extra SOM specific information for a given section.

   reloc_size holds the size of the relocation stream, note this
   is very different from the number of relocations as SOM relocations
   are variable length. 

   reloc_stream is the actual stream of relocation entries.

   The BFD section index may not exactly match a SOM subspace index,
   for this reason we keep track of the original SOM subspace index
   when a subspace is turned into a BFD section.  */

struct som_section_data_struct
  {
    unsigned int reloc_size;
    char *reloc_stream;
    unsigned int subspace_index;
    asection *containing_space;
    int is_space;
    struct space_dictionary_record space_dict;
    int is_subspace;
    struct subspace_dictionary_record subspace_dict;
  };

#define somdata(bfd)			((bfd)->tdata.som_data->a)
#define obj_som_file_hdr(bfd)		(somdata(bfd).file_hdr)
#define obj_som_copyright_hdr(bfd)	(somdata(bfd).copyright_aux_hdr)
#define obj_som_version_hdr(bfd)	(somdata(bfd).version_aux_hdr)
#define obj_som_symtab(bfd)		(somdata(bfd).symtab)
#define obj_som_stringtab(bfd)		(somdata(bfd).stringtab)
#define obj_som_sym_filepos(bfd)	(somdata(bfd).sym_filepos)
#define obj_som_str_filepos(bfd)	(somdata(bfd).str_filepos)
#define obj_som_stringtab_size(bfd)	(somdata(bfd).stringtab_size)
#define obj_som_reloc_filepos(bfd)	(somdata(bfd).reloc_filepos)
#define som_section_data(sec) \
  ((struct som_section_data_struct *)sec->used_by_bfd)
#define som_symbol_data(symbol) \
  (((som_symbol_type **) symbol->udata))


/* Defines groups of basic relocations.  FIXME:  These should
   be the only basic relocations created by GAS.  The rest
   should be internal to the BFD backend.

   The idea is both SOM and ELF define these basic relocation
   types so they map into a SOM or ELF specific reloation as
   appropriate.  This allows GAS to share much more code 
   between the two object formats.  */

#define R_HPPA_NONE			R_NO_RELOCATION
#define	R_HPPA				R_CODE_ONE_SYMBOL
#define	R_HPPA_ABS_CALL			R_ABS_CALL
#define	R_HPPA_PCREL_CALL		R_PCREL_CALL
#define	R_HPPA_GOTOFF			R_DP_RELATIVE
#define	R_HPPA_COMPLEX			R_COMP1
#define	R_HPPA_COMPLEX_PCREL_CALL	R_COMP2
#define	R_HPPA_COMPLEX_ABS_CALL		R_COMP3
#define R_HPPA_ENTRY			R_ENTRY
#define R_HPPA_EXIT			R_EXIT

/* Exported functions, mostly for use by GAS.  */
void bfd_som_set_section_attributes PARAMS ((asection *, char, char,
					     unsigned char, int));
void bfd_som_set_subsection_attributes PARAMS ((asection *, asection *,
						int, unsigned char, int));
void bfd_som_set_symbol_type PARAMS ((asymbol *, unsigned int));
void bfd_som_attach_unwind_info PARAMS ((asymbol *, char *));

#endif /* _SOM_H */
