/* Define a target vector for a variant of a.out.
   Copyright (C) 1990-1991 Free Software Foundation, Inc.

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

#ifndef MY_object_p
/* Finish up the reading of an a.out file header */
static bfd_target *
DEFUN(MY(callback),(abfd),
      bfd *abfd)
{
  struct internal_exec *execp = exec_hdr (abfd);
  
  WORK_OUT_FILE_POSITIONS(abfd, execp) ;
  
  /* Determine the architecture and machine type of the object file.
   */
  bfd_default_set_arch_mach(abfd, DEFAULT_ARCH, 0);

  adata(abfd)->page_size = PAGE_SIZE;
#ifdef SEGMENT_SIZE
  adata(abfd)->segment_size = SEGMENT_SIZE;
#else
  adata(abfd)->segment_size = PAGE_SIZE;
#endif
  adata(abfd)->exec_bytes_size = EXEC_BYTES_SIZE;

  return abfd->xvec;
}

static bfd_target *
DEFUN(MY(object_p),(abfd),
     bfd *abfd)
{
  struct external_exec exec_bytes;
  struct internal_exec exec;
  bfd_target *target;

  if (bfd_read ((PTR) &exec_bytes, 1, EXEC_BYTES_SIZE, abfd)
      != EXEC_BYTES_SIZE) {
    bfd_error = wrong_format;
    return 0;
  }

  exec.a_info = bfd_h_get_32 (abfd, exec_bytes.e_info);

  if (N_BADMAG (exec)) return 0;

  NAME(aout,swap_exec_header_in)(abfd, &exec_bytes, &exec);
  target = aout_32_some_aout_object_p (abfd, &exec, MY(callback));

#ifdef ENTRY_CAN_BE_ZERO
  /* The NEWSOS3 entry-point is/was 0, which (amongst other lossage)
   * means that it isn't obvious if EXEC_P should be set.
   * All of the following must be true for an executable:
   * There must be no relocations, the bfd can be neither an
   * archive nor an archive element, and the file must be executable. */

  if (exec.a_trsize + exec.a_drsize == 0
      && bfd_get_format(abfd) == bfd_object && abfd->my_archive == NULL)
    {
      struct stat buf;
#ifndef S_IXUSR
#define S_IXUSR 0100	/* Execute by owner.  */
#endif
      if (stat(abfd->filename, &buf) == 0 && (buf.st_mode & S_IXUSR))
	abfd->flags |= EXEC_P;
    }
#endif /* ENTRY_CAN_BE_ZERO */

  return target;
}
#define MY_object_p MY(object_p)
#endif


#ifndef MY_mkobject
static boolean
DEFUN(MY(mkobject),(abfd),
      bfd *abfd)
{
  if (NAME(aout,mkobject)(abfd) == false)
    return false;
  adata(abfd)->page_size = PAGE_SIZE;
#ifdef SEGMENT_SIZE
  adata(abfd)->segment_size = SEGMENT_SIZE;
#else
  adata(abfd)->segment_size = PAGE_SIZE;
#endif
  adata(abfd)->exec_bytes_size = EXEC_BYTES_SIZE;
  return true;
}
#define MY_mkobject MY(mkobject)
#endif

/* Write an object file.
   Section contents have already been written.  We write the
   file header, symbols, and relocation.  */

#ifndef MY_write_object_contents
static boolean
DEFUN(MY(write_object_contents),(abfd),
      bfd *abfd)
{
  bfd_size_type data_pad = 0;
  struct external_exec exec_bytes;
  struct internal_exec *execp = exec_hdr (abfd);

  WRITE_HEADERS(abfd, execp);
  return true;
}
#define MY_write_object_contents MY(write_object_contents)
#endif

/* We assume BFD generic archive files.  */
#ifndef	MY_openr_next_archived_file
#define	MY_openr_next_archived_file	bfd_generic_openr_next_archived_file
#endif
#ifndef	MY_generic_stat_arch_elt
#define	MY_generic_stat_arch_elt	bfd_generic_stat_arch_elt
#endif
#ifndef	MY_slurp_armap
#define	MY_slurp_armap			bfd_slurp_bsd_armap
#endif
#ifndef	MY_slurp_extended_name_table
#define	MY_slurp_extended_name_table	bfd_true
#endif
#ifndef	MY_write_armap
#define	MY_write_armap		bsd_write_armap
#endif
#ifndef	MY_truncate_arname
#define	MY_truncate_arname		bfd_bsd_truncate_arname
#endif

/* We don't support core files yet.  FIXME.  */
#ifndef	MY_core_file_failing_command
#define	MY_core_file_failing_command _bfd_dummy_core_file_failing_command
#endif
#ifndef	MY_core_file_failing_signal
#define	MY_core_file_failing_signal	_bfd_dummy_core_file_failing_signal
#endif
#ifndef	MY_core_file_matches_executable_p
#define	MY_core_file_matches_executable_p	\
				_bfd_dummy_core_file_matches_executable_p
#endif
#ifndef	MY_core_file_p
#define	MY_core_file_p		_bfd_dummy_target
#endif

#ifndef MY_bfd_debug_info_start
#define MY_bfd_debug_info_start		bfd_void
#endif
#ifndef MY_bfd_debug_info_end
#define MY_bfd_debug_info_end		bfd_void
#endif
#ifndef MY_bfd_debug_info_accumulate
#define MY_bfd_debug_info_accumulate	(PROTO(void,(*),(bfd*, struct sec *))) bfd_void
#endif

#ifndef MY_core_file_failing_command
#define MY_core_file_failing_command NAME(aout,core_file_failing_command)
#endif
#ifndef MY_core_file_failing_signal
#define MY_core_file_failing_signal NAME(aout,core_file_failing_signal)
#endif
#ifndef MY_core_file_matches_executable_p
#define MY_core_file_matches_executable_p NAME(aout,core_file_matches_executable_p)
#endif
#ifndef MY_slurp_armap
#define MY_slurp_armap NAME(aout,slurp_armap)
#endif
#ifndef MY_slurp_extended_name_table
#define MY_slurp_extended_name_table NAME(aout,slurp_extended_name_table)
#endif
#ifndef MY_truncate_arname
#define MY_truncate_arname NAME(aout,truncate_arname)
#endif
#ifndef MY_write_armap
#define MY_write_armap NAME(aout,write_armap)
#endif
#ifndef MY_close_and_cleanup
#define MY_close_and_cleanup NAME(aout,close_and_cleanup)
#endif
#ifndef MY_set_section_contents
#define MY_set_section_contents NAME(aout,set_section_contents)
#endif
#ifndef MY_get_section_contents
#define MY_get_section_contents NAME(aout,get_section_contents)
#endif
#ifndef MY_new_section_hook
#define MY_new_section_hook NAME(aout,new_section_hook)
#endif
#ifndef MY_get_symtab_upper_bound
#define MY_get_symtab_upper_bound NAME(aout,get_symtab_upper_bound)
#endif
#ifndef MY_get_symtab
#define MY_get_symtab NAME(aout,get_symtab)
#endif
#ifndef MY_get_reloc_upper_bound
#define MY_get_reloc_upper_bound NAME(aout,get_reloc_upper_bound)
#endif
#ifndef MY_canonicalize_reloc
#define MY_canonicalize_reloc NAME(aout,canonicalize_reloc)
#endif
#ifndef MY_make_empty_symbol
#define MY_make_empty_symbol NAME(aout,make_empty_symbol)
#endif
#ifndef MY_print_symbol
#define MY_print_symbol NAME(aout,print_symbol)
#endif
#ifndef MY_get_lineno
#define MY_get_lineno NAME(aout,get_lineno)
#endif
#ifndef MY_set_arch_mach
#define MY_set_arch_mach NAME(aout,set_arch_mach)
#endif
#ifndef MY_openr_next_archived_file
#define MY_openr_next_archived_file NAME(aout,openr_next_archived_file)
#endif
#ifndef MY_find_nearest_line
#define MY_find_nearest_line NAME(aout,find_nearest_line)
#endif
#ifndef MY_generic_stat_arch_elt
#define MY_generic_stat_arch_elt NAME(aout,generic_stat_arch_elt)
#endif
#ifndef MY_sizeof_headers
#define MY_sizeof_headers NAME(aout,sizeof_headers)
#endif
#ifndef MY_bfd_debug_info_start
#define MY_bfd_debug_info_start NAME(aout,bfd_debug_info_start)
#endif
#ifndef MY_bfd_debug_info_end
#define MY_bfd_debug_info_end NAME(aout,bfd_debug_info_end)
#endif
#ifndef MY_bfd_debug_info_accumulat
#define MY_bfd_debug_info_accumulat NAME(aout,bfd_debug_info_accumulat)
#endif

bfd_target MY(vec) =
{
  TARGETNAME,		/* name */
  bfd_target_aout_flavour,
  true,				/* target byte order */
  true,				/* target headers byte order */
  (HAS_RELOC | EXEC_P |		/* object flags */
   HAS_LINENO | HAS_DEBUG |
   HAS_SYMS | HAS_LOCALS | DYNAMIC | WP_TEXT | D_PAGED),
  (SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_RELOC), /* section flags */
  ' ',				/* ar_pad_char */
  16,				/* ar_max_namelen */
  1,				/* minimum alignment */
#ifdef TARGET_IS_BIG_ENDIAN_P
  _do_getb64, _do_putb64,	_do_getb32, _do_putb32, _do_getb16, _do_putb16, /* data */
  _do_getb64, _do_putb64,	_do_getb32, _do_putb32, _do_getb16, _do_putb16, /* hdrs */
#else
  _do_getl64, _do_putl64,	_do_getl32, _do_putl32, _do_getl16, _do_putl16, /* data */
  _do_getl64, _do_putl64,	_do_getl32, _do_putl32, _do_getl16, _do_putl16, /* hdrs */
#endif
    {_bfd_dummy_target, MY_object_p, /* bfd_check_format */
       bfd_generic_archive_p, MY_core_file_p},
    {bfd_false, MY_mkobject,	/* bfd_set_format */
       _bfd_generic_mkarchive, bfd_false},
    {bfd_false, MY_write_object_contents, /* bfd_write_contents */
       _bfd_write_archive_contents, bfd_false},

  MY_core_file_failing_command,
  MY_core_file_failing_signal,
  MY_core_file_matches_executable_p,
  MY_slurp_armap,
  MY_slurp_extended_name_table,
  MY_truncate_arname,
  MY_write_armap,
  MY_close_and_cleanup,
  MY_set_section_contents,
  MY_get_section_contents,
  MY_new_section_hook,
  MY_get_symtab_upper_bound,
  MY_get_symtab,
  MY_get_reloc_upper_bound,
  MY_canonicalize_reloc,
  MY_make_empty_symbol,
  MY_print_symbol,
  MY_get_lineno,
  MY_set_arch_mach,
  MY_openr_next_archived_file,
  MY_find_nearest_line,
  MY_generic_stat_arch_elt,
  MY_sizeof_headers,
  MY_bfd_debug_info_start,
  MY_bfd_debug_info_end,
  MY_bfd_debug_info_accumulate
};
