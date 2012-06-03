/* BFD back-end data structures for NLM (NetWare Loadable Modules) files.
   Copyright (C) 1993 Free Software Foundation, Inc.
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

#ifndef _LIBNLM_H_
#define _LIBNLM_H_ 1

#ifdef ARCH_SIZE
#  define NLM_ARCH_SIZE ARCH_SIZE
#endif
#include "nlm/common.h"
#include "nlm/internal.h"
#include "nlm/external.h"

/* A reloc for an imported NLM symbol.  Normal relocs are associated
   with sections, and include a symbol.  These relocs are associated
   with (undefined) symbols, and include a section.  */

struct nlm_relent
{
  /* Section of reloc.  */
  asection *section;
  /* Reloc info (sym_ptr_ptr field set only when canonicalized).  */
  arelent reloc;
};

/* Information we keep for an NLM symbol.  */

typedef struct
{
  /* BFD symbol.  */
  asymbol symbol;
  /* Number of reloc entries for imported symbol.  */
  bfd_size_type rcnt;
  /* Array of reloc information for imported symbol.  */
  struct nlm_relent *relocs;
} nlmNAME(symbol_type);

extern boolean nlm_mkobject PARAMS ((bfd *));
extern boolean nlm_set_arch_mach PARAMS ((bfd *, enum bfd_architecture,
					  unsigned long));

extern void nlmNAME(get_symbol_info)
     PARAMS ((bfd *, asymbol *, symbol_info *));
extern unsigned int nlmNAME(get_symtab_upper_bound)
     PARAMS ((bfd *));
extern unsigned int nlmNAME(get_symtab)
     PARAMS ((bfd *, asymbol **));
extern asymbol *nlmNAME(make_empty_symbol)
     PARAMS ((bfd *));
extern void nlmNAME(print_symbol)
     PARAMS ((bfd *, PTR, asymbol *, bfd_print_symbol_type));
extern unsigned int nlmNAME(get_reloc_upper_bound)
     PARAMS ((bfd *, asection *));
extern unsigned int nlmNAME(canonicalize_reloc)
     PARAMS ((bfd *, asection *, arelent **, asymbol **));
extern bfd_target *nlmNAME(object_p)
     PARAMS ((bfd *));
extern boolean nlmNAME(set_arch_mach)
     PARAMS ((bfd *, enum bfd_architecture, unsigned long));
extern boolean nlmNAME(set_section_contents)
     PARAMS ((bfd *, asection *, PTR, file_ptr, bfd_size_type));
extern boolean nlmNAME(write_object_contents)
     PARAMS ((bfd *));

/* Some private data is stashed away for future use using the tdata pointer
   in the bfd structure.  */

struct nlm_obj_tdata
{
  /* Actual data, but ref like ptr */
  Nlm_Internal_Fixed_Header	nlm_fixed_hdr[1];
  Nlm_Internal_Variable_Header	nlm_variable_hdr[1];
  Nlm_Internal_Version_Header	nlm_version_hdr[1];
  Nlm_Internal_Copyright_Header	nlm_copyright_hdr[1];
  Nlm_Internal_Extended_Header	nlm_extended_hdr[1];
  Nlm_Internal_Custom_Header	nlm_custom_hdr[1];
  /* BFD NLM symbols.  */
  nlmNAME(symbol_type)		*nlm_symbols;
  /* Lowest text and data VMA values.  */
  bfd_vma			nlm_text_low;
  bfd_vma			nlm_data_low;
  /* Caches for data read from object file.  */
  arelent *			nlm_reloc_fixups;
  asection **			nlm_reloc_fixup_secs;
};

#define nlm_tdata(bfd)			((bfd) -> tdata.nlm_obj_data)
#define nlm_fixed_header(bfd)		(nlm_tdata(bfd) -> nlm_fixed_hdr)
#define nlm_variable_header(bfd)	(nlm_tdata(bfd) -> nlm_variable_hdr)
#define nlm_version_header(bfd)		(nlm_tdata(bfd) -> nlm_version_hdr)
#define nlm_copyright_header(bfd)	(nlm_tdata(bfd) -> nlm_copyright_hdr)
#define nlm_extended_header(bfd)	(nlm_tdata(bfd) -> nlm_extended_hdr)
#define nlm_custom_header(bfd)		(nlm_tdata(bfd) -> nlm_custom_hdr)
#define nlm_get_symbols(bfd)		(nlm_tdata(bfd) -> nlm_symbols)
#define nlm_set_symbols(bfd, p)		(nlm_tdata(bfd) -> nlm_symbols = (p))
#define nlm_set_text_low(bfd, i)	(nlm_tdata(bfd) -> nlm_text_low = (i))
#define nlm_get_text_low(bfd)		(nlm_tdata(bfd) -> nlm_text_low)
#define nlm_set_data_low(bfd, i)	(nlm_tdata(bfd) -> nlm_data_low = (i))
#define nlm_get_data_low(bfd)		(nlm_tdata(bfd) -> nlm_data_low)
#define nlm_relocation_fixups(bfd)	(nlm_tdata(bfd) -> nlm_reloc_fixups)
#define nlm_relocation_fixup_secs(bfd)	(nlm_tdata(bfd)->nlm_reloc_fixup_secs)

/* We store some function pointer in the backend structure.  This lets
   different NLM targets share most of the same code, while providing
   slightly different code where necessary.  */

struct nlm_backend_data
{
  /* Architecture.  */
  enum bfd_architecture arch;
  /* Machine.  */
  long mach;
  /* Read a relocation fixup from abfd.  The reloc information is
     machine specific.  The second argument is the symbol if this is
     an import, or NULL if this is a reloc fixup.  This function
     should set the third argument to the section which the reloc
     belongs in, and the fourth argument to the reloc itself; it does
     not need to fill in the sym_ptr_ptr field for a reloc against an
     import symbol.  */
  boolean (*nlm_read_reloc) PARAMS ((bfd *, nlmNAME(symbol_type) *,
				     asection **, arelent *));
  /* Write a relocation fixup to abfd.  */
  boolean (*nlm_write_reloc) PARAMS ((bfd *, asection *, arelent *));
  /* To make objcopy to an i386 NLM work, the i386 backend needs a
     chance to work over the relocs.  This is a bit icky.  */
  boolean (*nlm_mangle_relocs) PARAMS ((bfd *, asection *, PTR data,
					bfd_vma offset,
					bfd_size_type count));
};

#define nlm_backend(bfd) \
  ((struct nlm_backend_data *)((bfd) -> xvec -> backend_data))
#define nlm_architecture(bfd) \
  (nlm_backend(bfd) ? nlm_backend(bfd) -> arch : bfd_arch_unknown)
#define nlm_machine(bfd) \
  (nlm_backend(bfd) ? nlm_backend(bfd) -> mach : 0)
#define nlm_read_reloc_func(bfd) \
  (nlm_backend(bfd) ? nlm_backend(bfd) -> nlm_read_reloc : 0)
#define nlm_write_reloc_func(bfd) \
  (nlm_backend(bfd) ? nlm_backend(bfd) -> nlm_write_reloc : 0)
#define nlm_mangle_relocs_func(bfd) \
  (nlm_backend(bfd) ? nlm_backend(bfd) -> nlm_mangle_relocs : 0)

/* The NLM code, data, and uninitialized sections have no names defined
   in the NLM, but bfd wants to give them names, so use the traditional
   UNIX names.  */

#define NLM_CODE_NAME			".text"
#define NLM_INITIALIZED_DATA_NAME	".data"
#define NLM_UNINITIALIZED_DATA_NAME	".bss"

#endif /* _LIBNLM_H_ */
