/* BFD backend for local host's a.out binaries
   Copyright (C) 1990-1991 Free Software Foundation, Inc.
   Written by Cygnus Support.  Probably John Gilmore's fault.

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

/* FIXME:  Re-write to use aout-target.h (or remove entirely...). */

#include "bfd.h"
#include "sysdep.h"
#include "libbfd.h"

#define	ARCH_SIZE	32

#include "libaout.h"           /* BFD a.out internal data structures */
#include "aout/aout64.h"

/* If the host has short headers on ZMAGIC files (like SunOS), then the
   text segment offset is 0 (as specified in aout64.h).  If it doesn't
   have short headers, the text segment offset is PAGE_SIZE.  */
#ifndef	HOST_SHORT_HEADER_HACK
#undef N_TXTOFF
#define N_TXTOFF(x)   ( (N_MAGIC((x)) == ZMAGIC) ? PAGE_SIZE : EXEC_BYTES_SIZE)
#endif

/* When porting to a new system, you must supply:

	HOST_PAGE_SIZE
	HOST_SEGMENT_SIZE	(optional -- defaults to page size)
	HOST_MACHINE_ARCH	(optional)
	HOST_MACHINE_MACHINE	(optional)
	HOST_TEXT_START_ADDR
	HOST_STACK_END_ADDR
	HOST_SHORT_HEADER_HACK	(optional)

   in the ./hosts/h-systemname.h file.  */

#define	PAGE_SIZE	HOST_PAGE_SIZE
#ifdef HOST_SEGMENT_SIZE
#define	SEGMENT_SIZE	HOST_SEGMENT_SIZE
#else
#define	SEGMENT_SIZE	HOST_PAGE_SIZE
#endif
#define	TEXT_START_ADDR	HOST_TEXT_START_ADDR
#define	STACK_END_ADDR	HOST_STACK_END_ADDR

static bfd_target *NAME(host_aout,callback) ();

/*SUPPRESS558*/
/*SUPPRESS529*/

bfd_target *
DEFUN(NAME(host_aout,object_p), (abfd),
     bfd *abfd)
{
  struct external_exec exec_bytes;
  struct internal_exec exec;

  if (bfd_read ((PTR) &exec_bytes, 1, EXEC_BYTES_SIZE, abfd)
      != EXEC_BYTES_SIZE) {
    bfd_error = wrong_format;
    return 0;
  }

  exec.a_info = bfd_h_get_32 (abfd, exec_bytes.e_info);

  if (N_BADMAG (exec)) return 0;

  NAME(aout,swap_exec_header_in)(abfd, &exec_bytes, &exec);
  return NAME(aout,some_aout_object_p) (abfd, &exec, NAME(host_aout,callback));
}

/* Set parameters about this a.out file that are machine-dependent.
   This routine is called from NAME(some_aout_object_p) just before it returns.
   This routine moves any data from the exec header,
   which is needed by the BFD code, out to places known to BFD.  This
   allows the rest of the BFD code to not know or care about the structure
   of exec_hdr (abfd).  */

static bfd_target *
DEFUN(NAME(host_aout,callback), (abfd),
     bfd *abfd)
{
  struct internal_exec *execp = exec_hdr (abfd);

  adata(abfd)->page_size = PAGE_SIZE;
  adata(abfd)->segment_size = SEGMENT_SIZE;
  adata(abfd)->exec_bytes_size = EXEC_BYTES_SIZE;

#ifdef HOST_MACHINE_ARCH
  bfd_default_set_arch_mach(abfd,
			    HOST_MACHINE_ARCH, 
#ifdef HOST_MACHINE_MACHINE
  			    HOST_MACHINE_MACHINE
#else  /* not HOST_MACHINE_MACHINE */
			    0
#endif /* not HOST_MACHINE_MACHINE */
			     );
#endif /* HOST_MACHINE_ARCH */

  WORK_OUT_FILE_POSITIONS (abfd, execp);

  return abfd->xvec;
}


boolean
DEFUN(NAME(host_aout,mkobject), (abfd),
     bfd *abfd)
{
  /* This struct is just for allocating two things with one zalloc, so
     they will be freed together, without violating alignment constraints. */
  struct aout_exec {
	struct aoutdata	aoutdata;
	struct internal_exec	exec;
  } *rawptr;

  bfd_error = system_call_error;

  /* Use an intermediate variable for clarity */
  rawptr = (struct aout_exec *)bfd_zalloc (abfd, sizeof (struct aout_exec));

  if (rawptr == NULL) {
    bfd_error = no_memory;
    return false;
  }

  set_tdata (abfd, &rawptr->aoutdata);
  exec_hdr (abfd) = &rawptr->exec;

  /* For simplicity's sake we just make all the sections right here. */

  obj_textsec (abfd) = (asection *)NULL;
  obj_datasec (abfd) = (asection *)NULL;
  obj_bsssec (abfd) = (asection *)NULL;
  bfd_make_section (abfd, ".text");
  bfd_make_section (abfd, ".data");
  bfd_make_section (abfd, ".bss");

  adata(abfd)->page_size = PAGE_SIZE;
  adata(abfd)->page_size = SEGMENT_SIZE;
  adata(abfd)->exec_bytes_size = EXEC_BYTES_SIZE;

  return true;
}

/* Write an object file in host a.out format.
   Section contents have already been written.  We write the
   file header, symbols, and relocation.  */

boolean
DEFUN(NAME(host_aout,write_object_contents), (abfd),
     bfd *abfd)
{
  size_t data_pad = 0;
  struct external_exec exec_bytes;
  struct internal_exec *execp = exec_hdr (abfd);

  execp->a_text = obj_textsec (abfd)->size;

  WRITE_HEADERS (abfd, execp);
  return true;
}

/* We use BFD generic archive files.  */
#define	aout_32_openr_next_archived_file	bfd_generic_openr_next_archived_file
#define	aout_32_generic_stat_arch_elt		bfd_generic_stat_arch_elt
#define	aout_32_slurp_armap			bfd_false
#define	aout_32_slurp_extended_name_table	bfd_true
#define	aout_32_write_armap			(PROTO (boolean, (*),	\
     (bfd *arch, unsigned int elength, struct orl *map, \
      unsigned int orl_count, int stridx))) bfd_false
#define	aout_32_truncate_arname			bfd_dont_truncate_arname

/* No core file defined here -- configure in trad-core.c separately.  */
#define	aout_32_core_file_failing_command _bfd_dummy_core_file_failing_command
#define	aout_32_core_file_failing_signal _bfd_dummy_core_file_failing_signal
#define	aout_32_core_file_matches_executable_p	\
				_bfd_dummy_core_file_matches_executable_p

#define aout_32_bfd_debug_info_start		bfd_void
#define aout_32_bfd_debug_info_end		bfd_void
#define aout_32_bfd_debug_info_accumulate	(PROTO(void,(*),(bfd*, struct sec *))) bfd_void

#define	aout_64_openr_next_archived_file	aout_32_openr_next_archived_file
#define	aout_64_generic_stat_arch_elt		aout_32_generic_stat_arch_elt
#define	aout_64_slurp_armap			aout_32_slurp_armap
#define	aout_64_slurp_extended_name_table	aout_32_slurp_extended_name_table
#define	aout_64_write_armap			aout_32_write_armap
#define	aout_64_truncate_arname			aout_32_truncate_arname

#define	aout_64_core_file_failing_command 	aout_32_core_file_failing_command
#define	aout_64_core_file_failing_signal	aout_32_core_file_failing_signal
#define	aout_64_core_file_matches_executable_p	aout_32_core_file_matches_executable_p

#define aout_64_bfd_debug_info_start		aout_32_bfd_debug_info_start
#define aout_64_bfd_debug_info_end		aout_32_bfd_debug_info_end
#define aout_64_bfd_debug_info_accumulate	aout_32_bfd_debug_info_accumulate


/* We implement these routines ourselves, rather than using the generic
   a.out versions.  */
#define	aout_write_object_contents	host_write_object_contents

bfd_target host_aout_big_vec =
  {
    "a.out-host-big",
    bfd_target_aout_flavour,
    true,			/* target byte order */
    true,			/* target headers byte order */
    (HAS_RELOC | EXEC_P |	/* object flags */
     HAS_LINENO | HAS_DEBUG |
     HAS_SYMS | HAS_LOCALS | DYNAMIC | WP_TEXT | D_PAGED),
    (SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_RELOC), /* section flags */
    ' ',						   /* ar_pad_char */
    16,							   /* ar_max_namelen */
    3,							   /* minimum alignment power */
    _do_getb64, _do_putb64, _do_getb32, _do_putb32, _do_getb16, _do_putb16, /* data */
    _do_getb64, _do_putb64, _do_getb32, _do_putb32, _do_getb16, _do_putb16, /* hdrs */
    
      {_bfd_dummy_target, NAME(host_aout,object_p),
       bfd_generic_archive_p, _bfd_dummy_target},
      {bfd_false, NAME(host_aout,mkobject),
       _bfd_generic_mkarchive, bfd_false},
      {bfd_false, NAME(host_aout,write_object_contents), /* bfd_write_contents */
       _bfd_write_archive_contents, bfd_false},
    
    JUMP_TABLE(JNAME(aout))
};

bfd_target host_aout_little_vec =
  {
    "a.out-host-little",
    bfd_target_aout_flavour,
    false,			/* target byte order */
    false,			/* target headers byte order */
    (HAS_RELOC | EXEC_P |	/* object flags */
     HAS_LINENO | HAS_DEBUG |
     HAS_SYMS | HAS_LOCALS | DYNAMIC | WP_TEXT | D_PAGED),
    (SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_RELOC), /* section flags */
    ' ',						   /* ar_pad_char */
    16,							   /* ar_max_namelen */
    3,							   /* minimum alignment power */
    _do_getl64, _do_putl64, _do_getl32, _do_putl32, _do_getl16, _do_putb16, /* data */
    _do_getl64, _do_putl64, _do_getl32, _do_putl32, _do_getl16, _do_putl16, /* hdrs */
    
      {_bfd_dummy_target, NAME(host_aout,object_p),
       bfd_generic_archive_p, _bfd_dummy_target},
      {bfd_false, NAME(host_aout,mkobject),
       _bfd_generic_mkarchive, bfd_false},
      {bfd_false, NAME(host_aout,write_object_contents), /* bfd_write_contents */
       _bfd_write_archive_contents, bfd_false},
    
    JUMP_TABLE(JNAME(aout))
};
