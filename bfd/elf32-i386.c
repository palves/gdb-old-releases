/* Intel 80386/80486-specific support for 32-bit ELF
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

#include "bfd.h"
#include "sysdep.h"
#include "libbfd.h"
#include "libelf.h"

enum reloc_type
  {
    R_386_NONE = 0,
    R_386_32,		R_386_PC32,
    R_386_GOT32,	R_386_PLT32,
    R_386_COPY,
    R_386_GLOB_DAT,	R_386_JUMP_SLOT,
    R_386_RELATIVE,
    R_386_GOTOFF,	R_386_GOTPC,
    R_386_max,
  };

#if 0
static CONST char *CONST reloc_type_names[] =
{
  "R_386_NONE",
  "R_386_32",		"R_386_PC32",
  "R_386_GOT32",	"R_386_PLT32",
  "R_386_COPY",
  "R_386_GLOB_DAT",	"R_386_JUMP_SLOT",
  "R_386_RELATIVE",
  "R_386_GOTOFF",	"R_386_GOTPC",
};
#endif

static CONST struct reloc_howto_struct *
DEFUN (elf_bfd_reloc_type_lookup, (abfd, code),
       bfd *abfd AND
       bfd_reloc_code_real_type code)
{
  return 0;
}

static void
DEFUN (elf_info_to_howto, (abfd, cache_ptr, dst),
       bfd *abfd AND
       arelent *cache_ptr AND
       Elf_Internal_Rela *dst)
{
  abort ();
}

#define TARGET_LITTLE_SYM	elf32_i386_vec
#define TARGET_LITTLE_NAME	"elf32-i386"
#define ELF_ARCH		bfd_arch_i386

#include "elf32-target.h"
