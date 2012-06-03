/* bfd back-end for HP PA-RISC SOM objects.
   Copyright (C) 1990, 1991, 1992, 1993 Free Software Foundation, Inc.

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

#include "bfd.h"
#include "sysdep.h"

/* @@FIXME This is not a reasonable set of conditions to permit
   cross-compilation, obviously.  It also isn't enough to support hppa-elf
   targets either.  Can we eliminate the HPUX or BSD dependencies, or
   at least get the conditionals more localized?  */
#if defined (HOST_HPPAHPUX) || defined (HOST_HPPABSD)

#include "libbfd.h"
#include "libhppa.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <signal.h>
#include <machine/reg.h>
#include <sys/user.h>		/* After a.out.h  */
#include <sys/file.h>
#include <errno.h>

/* Magic not defined in standard HP-UX header files until 8.0 */

#ifndef CPU_PA_RISC1_0
#define CPU_PA_RISC1_0 0x20B
#endif /* CPU_PA_RISC1_0 */

#ifndef CPU_PA_RISC1_1
#define CPU_PA_RISC1_1 0x210
#endif /* CPU_PA_RISC1_1 */

#ifndef _PA_RISC1_0_ID
#define _PA_RISC1_0_ID CPU_PA_RISC1_0
#endif /* _PA_RISC1_0_ID */

#ifndef _PA_RISC1_1_ID
#define _PA_RISC1_1_ID CPU_PA_RISC1_1
#endif /* _PA_RISC1_1_ID */

#ifndef _PA_RISC_MAXID
#define _PA_RISC_MAXID	0x2FF
#endif /* _PA_RISC_MAXID */

#ifndef _PA_RISC_ID
#define _PA_RISC_ID(__m_num)		\
    (((__m_num) == _PA_RISC1_0_ID) ||	\
     ((__m_num) >= _PA_RISC1_1_ID && (__m_num) <= _PA_RISC_MAXID))
#endif /* _PA_RISC_ID */

struct container
  {
    struct header f;
    struct som_exec_auxhdr e;
  };

static bfd_target *
hppa_object_setup (abfd, file_hdrp, aux_hdrp)
     bfd *abfd;
     struct header *file_hdrp;
     struct som_exec_auxhdr *aux_hdrp;
{
  struct container *rawptr;
  struct header *f;
  struct hppa_data_struct *rawptr1;
  asection *text, *data, *bss;

  rawptr = (struct container *) bfd_zalloc (abfd, sizeof (struct container));
  if (rawptr == NULL)
    {
      bfd_error = no_memory;
      return 0;
    }

  rawptr1 = (struct hppa_data_struct *) bfd_zalloc (abfd, sizeof (struct hppa_data_struct));
  if (rawptr1 == NULL)
    {
      bfd_error = no_memory;
      return 0;
    }

  abfd->tdata.hppa_data = rawptr1;
  obj_file_hdr (abfd) = &rawptr->f;
  obj_aux_hdr (abfd) = &rawptr->e;
  *obj_file_hdr (abfd) = *file_hdrp;
  *obj_aux_hdr (abfd) = *aux_hdrp;

  /* Set the file flags */
  abfd->flags = NO_FLAGS;
  if (file_hdrp->entry_offset)
    abfd->flags |= HAS_RELOC;
  if (file_hdrp->symbol_total)
    abfd->flags |= HAS_LINENO | HAS_DEBUG | HAS_SYMS | HAS_LOCALS;

  bfd_get_start_address (abfd) = aux_hdrp->exec_entry;

  obj_pa_symbols (abfd) = (hppa_symbol_type *) NULL;
  bfd_get_symcount (abfd) = file_hdrp->symbol_total;

  bfd_default_set_arch_mach (abfd, bfd_arch_hppa, 0);

  /* create the sections.  This is raunchy, but bfd_close wants to reclaim
     them */

  text = bfd_make_section (abfd, ".text");
  data = bfd_make_section (abfd, ".data");
  bss = bfd_make_section (abfd, ".bss");

  text->_raw_size = aux_hdrp->exec_tsize;
  data->_raw_size = aux_hdrp->exec_dsize;
  bss->_raw_size = aux_hdrp->exec_bsize;

  text->flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS);
  data->flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS);
  bss->flags = SEC_ALLOC;

  /* The virtual memory addresses of the sections */
  text->vma = aux_hdrp->exec_tmem;
  data->vma = aux_hdrp->exec_dmem;
  bss->vma = aux_hdrp->exec_bfill;

  /* The file offsets of the sections */
  text->filepos = aux_hdrp->exec_tfile;
  data->filepos = aux_hdrp->exec_dfile;

  /* The file offsets of the relocation info */
  text->rel_filepos = 0;
  data->rel_filepos = 0;

  /* The file offsets of the string table and symbol table.  */
  obj_sym_filepos (abfd) = file_hdrp->symbol_location;
  bfd_get_symcount (abfd) = file_hdrp->symbol_total;
  obj_str_filepos (abfd) = file_hdrp->symbol_strings_location;
  obj_stringtab_size (abfd) = file_hdrp->symbol_strings_size;

  return abfd->xvec;
}

/* Create a new BFD section for NAME.  If NAME already exists, then create a
   new unique name, with NAME as the prefix.  This exists because SOM .o files
   created by the native compiler can have a $CODE$ section for each
   subroutine.
 */

static asection *
make_unique_section (abfd, name, num)
     bfd *abfd;
     CONST char *name;
     int num;
{
  asection *sect;
  char *newname;
  char altname[100];

  sect = bfd_make_section (abfd, name);
  while (!sect)
    {
      sprintf (altname, "%s-%d", name, num++);
      sect = bfd_make_section (abfd, altname);
    }

  newname = bfd_alloc (abfd, strlen (sect->name) + 1);
  strcpy (newname, sect->name);

  sect->name = newname;
  return sect;
}

/* Convert all of the space and subspace info into BFD sections.  Each space
   contains a number of subspaces, which in turn describe the mapping between
   regions of the exec file, and the address space that the program runs in.
   BFD sections which correspond to spaces will overlap the sections for the
   associated subspaces.  */

static int
setup_sections (abfd, file_hdr)
     bfd *abfd;
     struct header *file_hdr;
{
  char *space_strings;
  int space_index;

  /* First, read in space names */

  space_strings = alloca (file_hdr->space_strings_size);
  if (!space_strings)
    return 0;

  if (bfd_seek (abfd, file_hdr->space_strings_location, SEEK_SET) < 0)
    return 0;
  if (bfd_read (space_strings, 1, file_hdr->space_strings_size, abfd)
      != file_hdr->space_strings_size)
    return 0;

  /* Loop over all of the space dictionaries, building up sections */

  for (space_index = 0; space_index < file_hdr->space_total; space_index++)
    {
      struct space_dictionary_record space;
      struct subspace_dictionary_record subspace;
      int subspace_index, tmp;
      asection *space_asect;

      /* Read the space dictionary element */
      if (bfd_seek (abfd, file_hdr->space_location
		    + space_index * sizeof space, SEEK_SET) < 0)
	return 0;
      if (bfd_read (&space, 1, sizeof space, abfd) != sizeof space)
	return 0;

      /* Setup the space name string */
      space.name.n_name = space.name.n_strx + space_strings;

      /* Make a section out of it */
      space_asect = make_unique_section (abfd, space.name.n_name, space_index);
      if (!space_asect)
	return 0;

      /* Now, read in the first subspace for this space */
      if (bfd_seek (abfd, file_hdr->subspace_location
		    + space.subspace_index * sizeof subspace,
		    SEEK_SET) < 0)
	return 0;
      if (bfd_read (&subspace, 1, sizeof subspace, abfd) != sizeof subspace)
	return 0;
      /* Seek back to the start of the subspaces for loop below */
      if (bfd_seek (abfd, file_hdr->subspace_location
		    + space.subspace_index * sizeof subspace,
		    SEEK_SET) < 0)
	return 0;

      /* Setup the section flags as appropriate (this is somewhat bogus, as
	 there isn't a clear mapping between what's in the space record, and
	 what BFD can describe here). */
      if (space.is_loadable)
	space_asect->flags |= SEC_ALLOC;
      if (space.is_defined)
	space_asect->flags |= SEC_LOAD;

      /* Setup the start address and file loc from the first subspace record */
      space_asect->vma = subspace.subspace_start;
      space_asect->filepos = subspace.file_loc_init_value;
      space_asect->alignment_power = subspace.alignment;

      /* Loop over the rest of the subspaces, building up more sections */
      for (subspace_index = 0; subspace_index < space.subspace_quantity;
	   subspace_index++)
	{
	  asection *subspace_asect;

	  /* Read in the next subspace */
	  if (bfd_read (&subspace, 1, sizeof subspace, abfd)
	      != sizeof subspace)
	    return 0;

	  /* Setup the subspace name string */
	  subspace.name.n_name = subspace.name.n_strx + space_strings;

	  /* Make a section out of this subspace */
	  subspace_asect = make_unique_section (abfd, subspace.name.n_name,
				     space.subspace_index + subspace_index);

	  if (!subspace_asect)
	    return 0;

	  if (subspace.is_loadable)
	    subspace_asect->flags |= SEC_ALLOC | SEC_LOAD;
	  if (subspace.code_only)
	    subspace_asect->flags |= SEC_CODE;

	  subspace_asect->vma = subspace.subspace_start;
	  subspace_asect->_cooked_size = subspace.subspace_length;
	  subspace_asect->_raw_size = subspace.initialization_length;
	  subspace_asect->alignment_power = subspace.alignment;
	  subspace_asect->filepos = subspace.file_loc_init_value;

	}
      /* Setup the sizes for the space section based upon the info in the
	 last subspace of the space. */
      space_asect->_cooked_size = (subspace.subspace_start - space_asect->vma)
	+ subspace.subspace_length;
      space_asect->_raw_size = (subspace.file_loc_init_value
				- space_asect->filepos)
	+ subspace.initialization_length;
    }
}

static bfd_target *
hppa_object_p (abfd)
     bfd *abfd;
{
  struct header file_hdr;
  struct som_exec_auxhdr aux_hdr;

  if (bfd_read ((PTR) & file_hdr, 1, FILE_HDR_SIZE, abfd) != FILE_HDR_SIZE)
    return 0;

  if (!_PA_RISC_ID (file_hdr.system_id))
    {
      bfd_error = wrong_format;
      return 0;
    }

  switch (file_hdr.a_magic)
    {
    case RELOC_MAGIC:		/* I'm not really sure about all of these types... */
    case EXEC_MAGIC:
    case SHARE_MAGIC:
    case DEMAND_MAGIC:
#ifdef DL_MAGIC
    case DL_MAGIC:
#endif
#ifdef SHL_MAGIC
    case SHL_MAGIC:
#endif
      break;
    default:
      bfd_error = wrong_format;
      return 0;
    }

  if (file_hdr.version_id != VERSION_ID
      && file_hdr.version_id != NEW_VERSION_ID)
    {
      bfd_error = wrong_format;
      return 0;
    }

  if (bfd_read ((PTR) & aux_hdr, 1, AUX_HDR_SIZE, abfd) != AUX_HDR_SIZE)
    bfd_error = wrong_format;

  if (!setup_sections (abfd, &file_hdr))
    return 0;

  return hppa_object_setup (abfd, &file_hdr, &aux_hdr);
}

static boolean
hppa_mkobject (abfd)
     bfd *abfd;
{
  fprintf (stderr, "hppa_mkobject unimplemented\n");
  fflush (stderr);
  abort ();
  return (false);
}

boolean
hppa_write_object_contents(abfd)
     bfd *abfd;
{
  fprintf (stderr, "hppa_write_object_contents unimplemented\n");
  fflush (stderr);
  abort ();
  return (false);
}

static unsigned int
hppa_get_symtab_upper_bound (abfd)
     bfd *abfd;
{
  fprintf (stderr, "hppa_get_symtab_upper_bound unimplemented\n");
  fflush (stderr);
  abort ();
  return (0);
}

static unsigned int
hppa_get_reloc_upper_bound (abfd, asect)
     bfd *abfd;
     sec_ptr asect;
{
  fprintf (stderr, "hppa_get_reloc_upper_bound unimplemented\n");
  fflush (stderr);
  abort ();
  return (0);
}

static unsigned int
hppa_canonicalize_reloc (abfd, section, relptr, symbols)
     bfd *abfd;
     sec_ptr section;
     arelent **relptr;
     asymbol **symbols;
{
  fprintf (stderr, "hppa_canonicalize_reloc unimplemented\n");
  fflush (stderr);
  abort ();
}

extern bfd_target hppa_vec;

static unsigned int
hppa_get_symtab (abfd, location)
     bfd *abfd;
     asymbol **location;
{
  fprintf (stderr, "hppa_get_symtab unimplemented\n");
  fflush (stderr);
  abort ();
  return (0);
}

static asymbol *
hppa_make_empty_symbol (abfd)
     bfd *abfd;
{
  hppa_symbol_type *new =
  (hppa_symbol_type *) bfd_zalloc (abfd, sizeof (hppa_symbol_type));
  new->symbol.the_bfd = abfd;

  return &new->symbol;
}

static void
hppa_print_symbol (ignore_abfd, afile, symbol, how)
     bfd *ignore_abfd;
     PTR afile;
     asymbol *symbol;
     bfd_print_symbol_type how;
{
  fprintf (stderr, "hppa_print_symbol unimplemented\n");
  fflush (stderr);
  abort ();
}

static boolean
hppa_new_section_hook (abfd, newsect)
     bfd *abfd;
     asection *newsect;
{
  newsect->alignment_power = 3;

  /* We allow more than three sections internally */
  return true;
}

static boolean
hppa_set_section_contents (abfd, section, location, offset, count)
     bfd *abfd;
     sec_ptr section;
     PTR location;
     file_ptr offset;
     bfd_size_type count;
{
  fprintf (stderr, "hppa_set_section_contents unimplimented\n");
  fflush (stderr);
  abort ();
  return false;
}

static boolean
hppa_set_arch_mach (abfd, arch, machine)
     bfd *abfd;
     enum bfd_architecture arch;
     unsigned long machine;
{
  fprintf (stderr, "hppa_set_arch_mach unimplemented\n");
  fflush (stderr);
  /* Allow any architecture to be supported by the hppa backend */
  return bfd_default_set_arch_mach (abfd, arch, machine);
}

static boolean
hppa_find_nearest_line (abfd, section, symbols, offset, filename_ptr,
			functionname_ptr, line_ptr)
     bfd *abfd;
     asection *section;
     asymbol **symbols;
     bfd_vma offset;
     CONST char **filename_ptr;
     CONST char **functionname_ptr;
     unsigned int *line_ptr;
{
  fprintf (stderr, "hppa_find_nearest_line unimplemented\n");
  fflush (stderr);
  abort ();
  return (false);
}

static int
hppa_sizeof_headers (abfd, reloc)
     bfd *abfd;
     boolean reloc;
{
  fprintf (stderr, "hppa_sizeof_headers unimplemented\n");
  fflush (stderr);
  abort ();
  return (0);
}

/* Miscellaneous Support Functions -- Control Structures and Functions
   for the PA.  */

unsigned int 
assemble_3 (x)
     unsigned int x;
{
  return (((x & 1) << 2) | ((x & 6) >> 1)) & 7;
}

void 
dis_assemble_3 (x, r)
     unsigned int x;
     unsigned int *r;
{
  *r = (((x & 4) >> 2) | ((x & 3) << 1)) & 7;
}

unsigned int 
assemble_12 (x, y)
     unsigned int x, y;
{
  return (((y & 1) << 11) | ((x & 1) << 10) | ((x & 0x7fe) >> 1)) & 0xfff;
}

void 
dis_assemble_12 (as12, x, y)
     unsigned int as12;
     unsigned int *x, *y;
{
  *y = (as12 & 0x800) >> 11;
  *x = ((as12 & 0x3ff) << 1) | ((as12 & 0x400) >> 10);
}

unsigned long 
assemble_17 (x, y, z)
     unsigned int x, y, z;
{
  unsigned long temp;

  temp = ((z & 1) << 16) |
    ((x & 0x1f) << 11) |
    ((y & 1) << 10) |
    ((y & 0x7fe) >> 1);
  return temp & 0x1ffff;
}

void 
dis_assemble_17 (as17, x, y, z)
     unsigned int as17;
     unsigned int *x, *y, *z;
{

  *z = (as17 & 0x10000) >> 16;
  *x = (as17 & 0x0f800) >> 11;
  *y = (((as17 & 0x00400) >> 10) | ((as17 & 0x3ff) << 1)) & 0x7ff;
}

unsigned long 
assemble_21 (x)
     unsigned int x;
{
  unsigned long temp;

  temp = ((x & 1) << 20) |
    ((x & 0xffe) << 8) |
    ((x & 0xc000) >> 7) |
    ((x & 0x1f0000) >> 14) |
    ((x & 0x003000) >> 12);
  return temp & 0x1fffff;
}

void 
dis_assemble_21 (as21, x)
     unsigned int as21, *x;
{
  unsigned long temp;


  temp = (as21 & 0x100000) >> 20;
  temp |= (as21 & 0x0ffe00) >> 8;
  temp |= (as21 & 0x000180) << 7;
  temp |= (as21 & 0x00007c) << 14;
  temp |= (as21 & 0x000003) << 12;
  *x = temp;
}

#if 0
unsigned long 
sign_ext (x, len)
     unsigned int x, len;
{
  unsigned int sign;
  unsigned int result;
  unsigned int len_ones;
  int i;

  i = 0;
  len_ones = 0;
  while (i < len)
    {
      len_ones = (len_ones << 1) | 1;
      i++;
    }

  sign = (x >> (len - 1)) & 1;

  if (sign)
    result = (~0 ^ len_ones) | (len_ones & x);
  else
    result = len_ones & x;

  return result;
}

#endif
static unsigned long 
sign_ext (x, len)
     unsigned int x, len;
{
  return (x << (32 - len)) >> (32 - len);
}

static unsigned int 
ones (n)
     int n;
{
  unsigned int len_ones;
  int i;

  i = 0;
  len_ones = 0;
  while (i < n)
    {
      len_ones = (len_ones << 1) | 1;
      i++;
    }

  return len_ones;
}

void 
sign_unext (x, len, result)
     unsigned int x, len;
     unsigned int *result;
{
  unsigned int len_ones;

  len_ones = ones (len);

  *result = x & len_ones;
}

unsigned long 
low_sign_ext (x, len)
     unsigned int x, len;
{
  unsigned int temp1, temp2;
  unsigned int len_ones;

  len_ones = ones (len);

  temp1 = (x & 1) << (len - 1);
  temp2 = ((x & 0xfffffffe) & len_ones) >> 1;
  return sign_ext ((temp1 | temp2), len);
}

void 
low_sign_unext (x, len, result)
     unsigned int x, len;
     unsigned int *result;
{
  unsigned int temp;
  unsigned int sign;
  unsigned int rest;
  unsigned int one_bit_at_len;
  unsigned int len_ones;

  len_ones = ones (len);
  one_bit_at_len = 1 << (len - 1);

  sign_unext (x, len, &temp);
  sign = temp & one_bit_at_len;
  sign >>= (len - 1);

  rest = temp & (len_ones ^ one_bit_at_len);
  rest <<= 1;

  *result = rest | sign;
}

/* These work when 'y' is a power of two only. */

static long
round_down (x, y)
     long x, y;
{
  return x & ~(y - 1);
}

static long
round (x, y)
     long x, y;
{
  return (x + y / 2) & ~(y - 1);
}

static long
round_up (x, y)
     long x, y;
{
  return x - (x | ~(y - 1));
}

/*	L(Symbol, Addend):	*/
/*		round_down (Symbol + Addend, 2048)	*/

static long
L (Symbol, Addend)
{
  return (round_down (Symbol + Addend, 2048)) >> 11;
}

/*	R(Symbol, Addend):	*/
/*		Symbol + Addend - round_down (Symbol + Addend, 2048)	*/

static long
R (Symbol, Addend)
{
  return Symbol + Addend - round_down (Symbol + Addend, 2048);
}

/*	LS(Symbol, Addend):	*/
/*		round (Symbol + Addend, 2048)	*/

static long
LS (Symbol, Addend)
{
  return round (Symbol + Addend, 2048);
}

/*	RS(Symbol, Addend):	*/
/*		Symbol + Addend - round (Symbol + Addend, 2048)	*/

static long
RS (Symbol, Addend)
{
  return Symbol + Addend - round (Symbol + Addend, 2048);
}

/*	LD(Symbol, Addend):	*/
/*		round_up (Symbol + Addend, 2048)	*/

static long
LD (Symbol, Addend)
{
  return (round_up (Symbol + Addend, 2048)) >> 11;
}

/*	RD(Symbol, Addend):	*/
/*		Symbol + Addend - round_up (Symbol + Addend, 2048)	*/

static long
RD (Symbol, Addend)
{
  return Symbol + Addend - round_up (Symbol + Addend, 2048);
}

/*	LR(Symbol, Addend):	*/
/*		round_down (Symbol, 2048) + round (Addend, 8192)	*/

static long
LR (Symbol, Addend)
{
  return (round_down (Symbol, 2048) + round (Addend, 8192)) >> 11;
}

/*	RR(Symbol, Addend):	*/
/*		Symbol - round_down (Symbol, 2048) +	*/
/*			Addend - round (Addend, 8192)	*/

static long
RR (Symbol, Addend)
{
  return Symbol
  - round_down (Symbol, 2048)
  + Addend - round (Addend, 8192);
}

unsigned long
hppa_field_adjust (value, constant_value, r_field)
     unsigned long value;
     unsigned long constant_value;
     unsigned short r_field;
{
  unsigned long init_value = value;
  value += constant_value;
  switch (r_field)
    {
    case e_fsel:		/* F  : no change                      */
      break;

    case e_lssel:		/* LS : if (bit 21) then add 0x800
		                    arithmetic shift right 11 bits */
      if (value & 0x00000400)
	value += 0x800;
      value = (value & 0xfffff800) >> 11;
      BFD_ASSERT (value == LS (init_value, constant_value));
      break;

    case e_rssel:		/* RS : Sign extend from bit 21        */
      if (value & 0x00000400)
	value |= 0xfffff800;
      else
	value &= 0x7ff;
      BFD_ASSERT (value == RS (init_value, constant_value));
      break;

    case e_lsel:		/* L  : Arithmetic shift right 11 bits */
      value = (value & 0xfffff800) >> 11;
      BFD_ASSERT (value == L (init_value, constant_value));
      break;

    case e_rsel:		/* R  : Set bits 0-20 to zero          */
      value = value & 0x7ff;
      BFD_ASSERT (value == R (init_value, constant_value));
      break;

    case e_ldsel:		/* LD : Add 0x800, arithmetic shift
		                    right 11 bits                  */
      value += 0x800;
      value = (value & 0xfffff800) >> 11;
      BFD_ASSERT (value == LD (init_value, constant_value));
      break;

    case e_rdsel:		/* RD : Set bits 0-20 to one           */
      value |= 0xfffff800;
      BFD_ASSERT (value == RD (init_value, constant_value));
      break;

    case e_lrsel:		/* LR : L with "rounded" constant      */
      value = value + ((constant_value + 0x1000) & 0xffffe000);
      value = (value & 0xfffff800) >> 11;
      BFD_ASSERT (value == LR (init_value, constant_value));
      break;

    case e_rrsel:		/* RR : R with "rounded" constant      */
      value = value + ((constant_value + 0x1000) & 0xffffe000);
      value = (value & 0x7ff) + constant_value - ((constant_value + 0x1000) & 0xffffe000);
      BFD_ASSERT (value == RR (init_value, constant_value));
      break;

    default:
      fprintf (stderr, "Unrecognized field_selector 0x%02x\n", r_field);
      break;
    }
  return value;

}

/* Return information about SOM symbol SYMBOL in RET.  */

static void
hppa_get_symbol_info (ignore_abfd, symbol, ret)
     bfd *ignore_abfd;		/* Ignored.  */
     asymbol *symbol;
     symbol_info *ret;
{
  bfd_symbol_info (symbol, ret);
}

/* End of miscellaneous support functions. */

#define hppa_bfd_debug_info_start        bfd_void
#define hppa_bfd_debug_info_end          bfd_void
#define hppa_bfd_debug_info_accumulate   (PROTO(void,(*),(bfd*, struct sec *))) bfd_void

#define hppa_openr_next_archived_file    bfd_generic_openr_next_archived_file
#define hppa_generic_stat_arch_elt       bfd_generic_stat_arch_elt
#define hppa_slurp_armap                  bfd_false
#define hppa_slurp_extended_name_table    _bfd_slurp_extended_name_table
#define hppa_truncate_arname              (void (*)())bfd_nullvoidptr
#define hppa_write_armap                  0

#define hppa_get_lineno                   (struct lineno_cache_entry *(*)())bfd_nullvoidptr
#define	hppa_close_and_cleanup	           bfd_generic_close_and_cleanup
#define hppa_get_section_contents          bfd_generic_get_section_contents

#define hppa_bfd_get_relocated_section_contents \
 bfd_generic_get_relocated_section_contents
#define hppa_bfd_relax_section bfd_generic_relax_section
#define hppa_bfd_seclet_link bfd_generic_seclet_link
#define hppa_bfd_reloc_type_lookup \
  ((CONST struct reloc_howto_struct *(*) PARAMS ((bfd *, bfd_reloc_code_real_type))) bfd_nullvoidptr)
#define hppa_bfd_make_debug_symbol \
  ((asymbol *(*) PARAMS ((bfd *, void *, unsigned long))) bfd_nullvoidptr)

/* Core file support is in the hpux-core backend.  */
#define hppa_core_file_failing_command	_bfd_dummy_core_file_failing_command
#define hppa_core_file_failing_signal	_bfd_dummy_core_file_failing_signal
#define hppa_core_file_matches_executable_p	_bfd_dummy_core_file_matches_executable_p

bfd_target hppa_vec =
{
  "hppa",			/* name */
  bfd_target_hppa_flavour,
  true,				/* target byte order */
  true,				/* target headers byte order */
  (HAS_RELOC | EXEC_P |		/* object flags */
   HAS_LINENO | HAS_DEBUG |
   HAS_SYMS | HAS_LOCALS | DYNAMIC | WP_TEXT | D_PAGED),
  (SEC_CODE | SEC_DATA | SEC_ROM | SEC_HAS_CONTENTS
   | SEC_ALLOC | SEC_LOAD | SEC_RELOC),	/* section flags */

/* leading_symbol_char: is the first char of a user symbol
      predictable, and if so what is it */
  0,
  ' ',				/* ar_pad_char */
  16,				/* ar_max_namelen */
  3,				/* minimum alignment */
  _do_getb64, _do_getb_signed_64, _do_putb64,
  _do_getb32, _do_getb_signed_32, _do_putb32,
  _do_getb16, _do_getb_signed_16, _do_putb16,	/* data */
  _do_getb64, _do_getb_signed_64, _do_putb64,
  _do_getb32, _do_getb_signed_32, _do_putb32,
  _do_getb16, _do_getb_signed_16, _do_putb16,	/* hdrs */
  {_bfd_dummy_target,
   hppa_object_p,		/* bfd_check_format */
   bfd_generic_archive_p,
   bfd_false
  },
  {
    bfd_false,
    hppa_mkobject,
    _bfd_generic_mkarchive,
    bfd_false
  },
  {
    bfd_false,
    hppa_write_object_contents,
    _bfd_write_archive_contents,
    bfd_false,
  },
#undef hppa
  JUMP_TABLE (hppa),
  (PTR) 0
};

#endif /* HOST_HPPAHPUX || HOST_HPPABSD */
