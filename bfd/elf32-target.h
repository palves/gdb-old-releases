/* Target definitions for 32-bit ELF
   Copyright 1993 Free Software Foundation, Inc.

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

/* This structure contains everything that BFD knows about a target.
   It includes things like its byte order, name, what routines to call
   to do various operations, etc.  Every BFD points to a target structure
   with its "xvec" member.

   There are two such structures here:  one for big-endian machines and
   one for little-endian machines.   */

/* Archives are generic or unimplemented.  */
#ifndef elf_slurp_armap
#define elf_slurp_armap			bfd_slurp_coff_armap
#endif
#define elf_slurp_extended_name_table	_bfd_slurp_extended_name_table
#define elf_truncate_arname		bfd_dont_truncate_arname
#define elf_openr_next_archived_file	bfd_generic_openr_next_archived_file
#define elf_generic_stat_arch_elt	bfd_generic_stat_arch_elt
#ifndef elf_write_armap
#define	elf_write_armap			coff_write_armap
#endif

/* Ordinary section reading and writing */
#define elf_new_section_hook		_bfd_dummy_new_section_hook
#define elf_get_section_contents	bfd_generic_get_section_contents
/* #define elf_set_section_contents	bfd_generic_set_section_contents */
#define	elf_close_and_cleanup		bfd_generic_close_and_cleanup

#define elf_bfd_debug_info_start	bfd_void
#define elf_bfd_debug_info_end		bfd_void
#define elf_bfd_debug_info_accumulate	(PROTO(void,(*),(bfd*, struct sec *))) bfd_void
#define elf_bfd_get_relocated_section_contents \
 bfd_generic_get_relocated_section_contents
#define elf_bfd_relax_section bfd_generic_relax_section
#define elf_bfd_seclet_link bfd_generic_seclet_link
#define elf_bfd_make_debug_symbol \
  ((asymbol *(*) PARAMS ((bfd *, void *, unsigned long))) bfd_nullvoidptr)

static CONST struct elf_backend_data elf_bed =
{
  elf_info_to_howto,
  ELF_ARCH,
};

#ifdef TARGET_BIG_SYM
bfd_target TARGET_BIG_SYM =
{
  /* name: identify kind of target */
  TARGET_BIG_NAME,

  /* flavour: general indication about file */
  bfd_target_elf_flavour,

  /* byteorder_big_p: data is big endian */
  true,

  /* header_byteorder_big_p: header is also big endian */
  true,

  /* object_flags: mask of all file flags */
  (HAS_RELOC | EXEC_P | HAS_LINENO | HAS_DEBUG | HAS_SYMS | HAS_LOCALS |
   DYNAMIC | WP_TEXT),
  
  /* section_flags: mask of all section flags */
  (SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_READONLY |
   SEC_CODE | SEC_DATA), 

   /* leading_symbol_char: is the first char of a user symbol
      predictable, and if so what is it */
   0,

  /* ar_pad_char: pad character for filenames within an archive header
     FIXME:  this really has nothing to do with ELF, this is a characteristic
     of the archiver and/or os and should be independently tunable */
  '/',

  /* ar_max_namelen: maximum number of characters in an archive header
     FIXME:  this really has nothing to do with ELF, this is a characteristic
     of the archiver and should be independently tunable.  This value is
     a WAG (wild a** guess) */
  15,

  /* align_power_min: minimum alignment restriction for any section
     FIXME:  this value may be target machine dependent */
  3,

  /* Routines to byte-swap various sized integers from the data sections */
  _do_getb64, _do_getb_signed_64, _do_putb64,
    _do_getb32, _do_getb_signed_32, _do_putb32,
    _do_getb16, _do_getb_signed_16, _do_putb16,

  /* Routines to byte-swap various sized integers from the file headers */
  _do_getb64, _do_getb_signed_64, _do_putb64,
    _do_getb32, _do_getb_signed_32, _do_putb32,
    _do_getb16, _do_getb_signed_16, _do_putb16,

  /* bfd_check_format: check the format of a file being read */
  { _bfd_dummy_target,		/* unknown format */
    elf_object_p,		/* assembler/linker output (object file) */
    bfd_generic_archive_p,	/* an archive */
    elf_core_file_p		/* a core file */
  },

  /* bfd_set_format: set the format of a file being written */
  { bfd_false,
    elf_mkobject,
    _bfd_generic_mkarchive,
    bfd_false
  },

  /* bfd_write_contents: write cached information into a file being written */
  { bfd_false,
    elf_write_object_contents,
    _bfd_write_archive_contents,
    bfd_false
  },

  /* Initialize a jump table with the standard macro.  All names start with
     "elf" */
  JUMP_TABLE(elf),

  /* backend_data: */
  (PTR) &elf_bed,
};
#endif

#ifdef TARGET_LITTLE_SYM
bfd_target TARGET_LITTLE_SYM =
{
  /* name: identify kind of target */
  TARGET_LITTLE_NAME,

  /* flavour: general indication about file */
  bfd_target_elf_flavour,

  /* byteorder_big_p: data is big endian */
  false,		/* Nope -- this one's little endian */

  /* header_byteorder_big_p: header is also big endian */
  false,		/* Nope -- this one's little endian */

  /* object_flags: mask of all file flags */
  (HAS_RELOC | EXEC_P | HAS_LINENO | HAS_DEBUG | HAS_SYMS | HAS_LOCALS |
   DYNAMIC | WP_TEXT),
  
  /* section_flags: mask of all section flags */
  (SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_READONLY |
   SEC_DATA), 

   /* leading_symbol_char: is the first char of a user symbol
      predictable, and if so what is it */
   0,

  /* ar_pad_char: pad character for filenames within an archive header
     FIXME:  this really has nothing to do with ELF, this is a characteristic
     of the archiver and/or os and should be independently tunable */
  '/',

  /* ar_max_namelen: maximum number of characters in an archive header
     FIXME:  this really has nothing to do with ELF, this is a characteristic
     of the archiver and should be independently tunable.  This value is
     a WAG (wild a** guess) */
  15,

  /* align_power_min: minimum alignment restriction for any section
     FIXME:  this value may be target machine dependent */
  3,

  /* Routines to byte-swap various sized integers from the data sections */
  _do_getl64, _do_getl_signed_64, _do_putl64,
    _do_getl32, _do_getl_signed_32, _do_putl32,
    _do_getl16, _do_getl_signed_16, _do_putl16,

  /* Routines to byte-swap various sized integers from the file headers */
  _do_getl64, _do_getl_signed_64, _do_putl64,
    _do_getl32, _do_getl_signed_32, _do_putl32,
    _do_getl16, _do_getl_signed_16, _do_putl16,

  /* bfd_check_format: check the format of a file being read */
  { _bfd_dummy_target,		/* unknown format */
    elf_object_p,		/* assembler/linker output (object file) */
    bfd_generic_archive_p,	/* an archive */
    elf_core_file_p		/* a core file */
  },

  /* bfd_set_format: set the format of a file being written */
  { bfd_false,
    elf_mkobject,
    _bfd_generic_mkarchive,
    bfd_false
  },

  /* bfd_write_contents: write cached information into a file being written */
  { bfd_false,
    elf_write_object_contents,
    _bfd_write_archive_contents,
    bfd_false
  },

  /* Initialize a jump table with the standard macro.  All names start with
     "elf" */
  JUMP_TABLE(elf),

  /* backend_data: */
  (PTR) &elf_bed,
};
#endif
