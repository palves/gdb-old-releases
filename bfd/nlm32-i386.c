/* Support for 32-bit i386 NLM (NetWare Loadable Module)
   Copyright (C) 1993 Free Software Foundation, Inc.

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

#define ARCH_SIZE 32
#include "libnlm.h"

static boolean nlm_i386_read_reloc
  PARAMS ((bfd *, nlmNAME(symbol_type) *, asection **, arelent *));
static boolean nlm_i386_write_reloc
  PARAMS ((bfd *, asection *, arelent *));

/* Adjust the reloc location by an absolute value.  */

static reloc_howto_type nlm_i386_abs_howto =
  HOWTO (0,			/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "32",			/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false);		/* pcrel_offset */

/* Adjust the reloc location by a PC relative displacement.  */

static reloc_howto_type nlm_i386_pcrel_howto =
  HOWTO (1,			/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 0,			/* special_function */
	 "DISP32",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true);			/* pcrel_offset */

/* Read a NetWare i386 reloc.  */

static boolean
nlm_i386_read_reloc (abfd, sym, secp, rel)
     bfd *abfd;
     nlmNAME(symbol_type) *sym;
     asection **secp;
     arelent *rel;
{
  bfd_byte temp[4];
  bfd_vma val;
  const char *name;

  if (bfd_read (temp, sizeof (temp), 1, abfd) != sizeof (temp))
    {
      bfd_error = system_call_error;
      return false;
    }

  val = bfd_get_32 (abfd, temp);

  /* The value is an offset into either the code or data segment.
     This is the location which needs to be adjusted.

     If this is a relocation fixup rather than an imported symbol (the
     sym argument is NULL) then the high bit is 0 if the location
     needs to be adjusted by the address of the data segment, or 1 if
     the location needs to be adjusted by the address of the code
     segment.  If this is an imported symbol, then the high bit is 0
     if the location is 0 if the location should be adjusted by the
     offset to the symbol, or 1 if the location should adjusted by the
     absolute value of the symbol.

     The second most significant bit is 0 if the value is an offset
     into the data segment, or 1 if the value is an offset into the
     code segment.

     All this translates fairly easily into a BFD reloc.  */

  if (sym == NULL)
    {
      if ((val & NLM_HIBIT) == 0)
	name = NLM_INITIALIZED_DATA_NAME;
      else
	{
	  name = NLM_CODE_NAME;
	  val &=~ NLM_HIBIT;
	}
      rel->sym_ptr_ptr = bfd_get_section_by_name (abfd, name)->symbol_ptr_ptr;
      rel->howto = &nlm_i386_abs_howto;
    }
  else
    {
      /* In this case we do not need to set the sym_ptr_ptr field.  */
      rel->sym_ptr_ptr = NULL;
      if ((val & NLM_HIBIT) == 0)
	rel->howto = &nlm_i386_pcrel_howto;
      else
	{
	  rel->howto = &nlm_i386_abs_howto;
	  val &=~ NLM_HIBIT;
	}
    }

  if ((val & (NLM_HIBIT >> 1)) == 0)
    *secp = bfd_get_section_by_name (abfd, NLM_INITIALIZED_DATA_NAME);
  else
    {
      *secp = bfd_get_section_by_name (abfd, NLM_CODE_NAME);
      val &=~ (NLM_HIBIT >> 1);
    }

  rel->address = val;
  rel->addend = 0;

  return true;
}

/* Write a NetWare i386 reloc.  */

static boolean
nlm_i386_write_reloc (abfd, sec, rel)
     bfd *abfd;
     asection *sec;
     arelent *rel;
{
  bfd_vma addend, val;
  bfd_byte temp[4];

  /* NetWare only supports four byte relocs.  */
  if (rel->howto == NULL
      || rel->howto->rightshift != 0
      || rel->howto->size != 2
      || rel->howto->bitsize != 32
      || rel->howto->bitpos != 0
      || rel->howto->src_mask != 0xffffffff
      || rel->howto->dst_mask != 0xffffffff)
    {
      bfd_error = invalid_operation;
      return false;
    }

  /* The high bit of the value is 1 if this is a reloc of a data
     reference.  The second most significant bit is 1 if this is a
     reloc from the data section.  */
  val = bfd_get_section_vma (abfd, sec) + rel->address;
  val += addend;
  if (bfd_get_section_flags (abfd, sec) & SEC_CODE)
    val -= nlm_get_text_low (abfd);
  else
    {
      val -= nlm_get_data_low (abfd);
      val |= NLM_HIBIT >> 1;
    }

  if (bfd_get_section_flags (abfd, bfd_get_section (*rel->sym_ptr_ptr))
      & SEC_CODE)
    ;
  else
    val |= NLM_HIBIT;
  
  bfd_put_32 (abfd, val, temp);
  if (bfd_write (temp, sizeof (temp), 1, abfd) != sizeof (temp))
    {
      bfd_error = system_call_error;
      return false;
    }

  return true;
}

static const struct nlm_backend_data nlm32_i386_backend =
{
  bfd_arch_i386,
  nlm_i386_read_reloc,
  nlm_i386_write_reloc
};

#define TARGET_LITTLE_NAME		"nlm32-i386"
#define TARGET_LITTLE_SYM		nlmNAME(i386_vec)
#define TARGET_BACKEND_DATA		&nlm32_i386_backend

#include "nlm-target.h"
