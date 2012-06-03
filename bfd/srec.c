/* BFD backend for s-record objects.
   Copyright (C) 1990-1991 Free Software Foundation, Inc.
   Written by Steve Chamberlain of Cygnus Support <steve@cygnus.com>.

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

/* S-records cannot hold anything but addresses and data, so that's all
   that we implement.
   
   The only interesting thing is that s-records may come out of order and
   there is no header, so an initial scan is required to discover the
   minimum and maximum addresses used to create the vma and size of the
   only section we create.  We arbitrarily call this section ".text".

   When bfd_get_section_contents is called the file is read again, and
   this time the data is placed into a bfd_alloc'd area.

   Any number of sections may be created for output, we just output them
   in the order provided to bfd_set_section_contents.  */

#include "bfd.h"
#include "sysdep.h"
#include "libbfd.h"

static char digs[] = "0123456789ABCDEF";

/* Macros for converting between hex and binary */

#define NIBBLE(x) (((x) >= '0' && (x) <= '9') ? ((x) - '0') : ((x) - 'A' + 10))
#define HEX(buffer) ((NIBBLE((buffer)->high) <<4) + NIBBLE((buffer)->low))
#define TOHEX(d,x) \
  ((d)->low = digs[(x) & 0xf], (d)->high = digs[((x)>>4)&0xf], (x))
#define	ISHEX(x)  (((x) >= '0' && (x) <= '9') || ((x) >= 'A' && (x) <= 'F'))

typedef struct {
  char high;
  char low;
} byte_as_two_char_type;

/* The maximum number of bytes on a line is FF */
#define MAXCHUNK 0xff 
/* The number of bytes we fit onto a line on output */
#define CHUNK 16 

/* The shape of an s-record .. */
typedef struct 
{
  char S;
  char type;
  byte_as_two_char_type size;
  union {
    struct {
      byte_as_two_char_type address[4];
      byte_as_two_char_type data[MAXCHUNK];
      /* If there isn't MAXCHUNK bytes of data then the checksum will 
	 appear earlier */
      byte_as_two_char_type checksum;
      char nl;
    } type_3;
    struct {
      byte_as_two_char_type address[4];
      byte_as_two_char_type data[MAXCHUNK];
      byte_as_two_char_type checksum;
      char nl;
    } type_6;

    struct {
      byte_as_two_char_type address[3];
      byte_as_two_char_type data[MAXCHUNK];
      byte_as_two_char_type checksum;
      char nl;
    } type_2;

    struct {
      byte_as_two_char_type address[2];
      byte_as_two_char_type data[MAXCHUNK];
      byte_as_two_char_type checksum;
      char nl;
    } type_1;
    byte_as_two_char_type data[MAXCHUNK];
  } u;
} srec_type;

#define enda(x) (x->vma + x->size)
/* 
   called once per input s-record, used to work out vma and size of data.
 */

static bfd_vma low,high;
static void
size_srec(abfd, section, address, raw, length)
bfd *abfd;
asection *section;
bfd_vma address;
byte_as_two_char_type *raw;
unsigned int length;
{
  if (address < low)
    low = address;
  if (address + length > high) 
    high = address + length;
}


/*
 called once per input s-record, copies data from input into bfd_alloc'd area
 */

static void
fillup(abfd, section, address, raw, length)
bfd *abfd;
asection *section;
bfd_vma address;
byte_as_two_char_type *raw;
unsigned int length;
{
  unsigned int i;
  bfd_byte *dst = (bfd_byte *)(section->used_by_bfd) +  address - section->vma;
  for (i = 0; i < length; i++) {
    *dst = HEX(raw);
    dst++;
    raw++;
  }
}

/* Pass over an s-record file, calling one of the above functions on each
   record.  */

static void
pass_over(abfd, func, section)
     bfd *abfd;
     void (*func)();
     asection *section;
{
  unsigned int bytes_on_line;
  boolean eof = false;
  bfd_vma address;
  /* To the front of the file */
  bfd_seek(abfd, (file_ptr)0, SEEK_SET);
  while (eof == false)
    {
      srec_type buffer;

      /* Find first 'S' */
      eof =  (boolean)(bfd_read(&buffer.S, 1, 1, abfd) != 1);
      while (buffer.S != 'S' && !eof) {
	eof = (boolean)(bfd_read(&buffer.S, 1, 1, abfd) != 1);
      }
      if (eof) break;

      bfd_read(&buffer.type, 1, 3, abfd);

      if (!ISHEX (buffer.size.high) || !ISHEX (buffer.size.low))
	break;
      bytes_on_line = HEX(&buffer.size);
      if (bytes_on_line > MAXCHUNK/2)
	break;
    
      bfd_read((PTR)buffer.u.data, 1 , bytes_on_line * 2, abfd);

      switch (buffer.type) {
      case '6':
	/* Prologue - ignore */
	break;

      case '3':
	address = (HEX(buffer.u.type_3.address+0) << 24)
		+ (HEX(buffer.u.type_3.address+1) << 16)
		+ (HEX(buffer.u.type_3.address+2) << 8) 
		+ (HEX(buffer.u.type_3.address+3));
        func(abfd,section, address, buffer.u.type_3.data, bytes_on_line -1);
	break;

      case '2':
	address = (HEX(buffer.u.type_2.address+0) << 16)
		+ (HEX(buffer.u.type_2.address+1) << 8)
		+ (HEX(buffer.u.type_2.address+2));
        func(abfd,section, address, buffer.u.type_2.data, bytes_on_line -1);
	break;

      case '1':
	address = (HEX(buffer.u.type_1.address+0) << 8) 
	        + (HEX(buffer.u.type_1.address+1));
        func(abfd, section, address, buffer.u.type_1.data, bytes_on_line -1);
	break;

      default:
	goto end_of_file;
      }
    }
  end_of_file: ;
}


bfd_target *
srec_object_p (abfd)
bfd *abfd;
{
  char b[4];
  asection *section;
  bfd_seek(abfd, (file_ptr)0, SEEK_SET);
  bfd_read(b, 1, 4, abfd);
  if (b[0] != 'S' || !ISHEX(b[1]) || !ISHEX(b[2]) || !ISHEX(b[3]))
    return (bfd_target*) NULL;
  
  /* We create one section called .text for all the contents, 
     and allocate enough room for the entire file.  */

  section =  bfd_make_section(abfd, ".text");
  section->size = 0;
  section->vma = 0xffffffff;
  low = 0xffffffff;
  high = 0;
  pass_over(abfd, size_srec, section);
  section->size = high - low;
  section->vma = low;
  return abfd->xvec;
}


static boolean
srec_get_section_contents (abfd, section, location, offset, count)
bfd *abfd;
sec_ptr section;
void  *location;
file_ptr offset;
unsigned      int count;
{
  if (section->used_by_bfd == (PTR)NULL) {
    section->used_by_bfd = (PTR)bfd_alloc (abfd, section->size);
    pass_over(abfd, fillup, section);
  }
  (void) memcpy((PTR)location, (PTR)((char *)(section->used_by_bfd) + offset), count);
  return true;
}
      


boolean
srec_set_arch_mach (abfd, arch, machine)
bfd *abfd;
enum bfd_architecture arch;
unsigned long machine;
{
  return bfd_default_set_arch_mach(abfd, arch, machine);
}



boolean
srec_set_section_contents (abfd, section, location, offset, bytes_to_do)
bfd *abfd;
sec_ptr section;
unsigned char *location;
file_ptr offset;
int bytes_to_do;
{
  bfd_vma address;
  int bytes_written;

  int type;
  unsigned int i;
  srec_type buffer;
  bytes_written = 0;
  if (section->vma <= 0xffff) 
    type = 1;
  else if (section->vma <= 0xffffff) 
    type = 2;
  else
    type = 3;

  buffer.S = 'S';
  buffer.type = '0' + type;

  while (bytes_written < bytes_to_do) {
    unsigned int size;
    unsigned int check_sum;
    byte_as_two_char_type *data; 
 unsigned   int bytes_this_chunk = bytes_to_do - bytes_written;

    if (bytes_this_chunk > CHUNK) {
      bytes_this_chunk = CHUNK;
    }

    address = section->vma + offset + bytes_written;

    switch (type) {
    case 3:
      check_sum = TOHEX(buffer.u.type_3.address, address >> 24);
      check_sum += TOHEX(buffer.u.type_3.address+1, address >> 16);
      check_sum += TOHEX(buffer.u.type_3.address+2, address >> 8);
      check_sum += TOHEX(buffer.u.type_3.address+3, address >> 0);
      size = bytes_this_chunk + 5;
      data = buffer.u.type_3.data;
      break;
    case 2:
      check_sum = TOHEX(buffer.u.type_3.address, address >> 16);
      check_sum += TOHEX(buffer.u.type_3.address+1, address >> 8);
      check_sum += TOHEX(buffer.u.type_3.address+2, address >> 0);
      size = bytes_this_chunk + 4;
      data = buffer.u.type_2.data;
      break;

    case 1:
      check_sum = TOHEX(buffer.u.type_3.address+0, address >> 8);
      check_sum += TOHEX(buffer.u.type_3.address+1, address >> 0);
      size = bytes_this_chunk + 3;
      data = buffer.u.type_1.data;
      break;
    }

    for (i = 0; i < bytes_this_chunk; i++) {
      check_sum += TOHEX(data, (location[i]));
      data++;
    }

    check_sum += TOHEX(&(buffer.size), size );
    (void) TOHEX(data, ~check_sum);
    data++;

    * ( (char *)(data)) = '\n';
    bfd_write((PTR)&buffer, 1, (char *)data - (char *)&buffer + 1 , abfd);

    bytes_written += bytes_this_chunk;
    location += bytes_this_chunk;
  }


  return true;
}

boolean
srec_write_object_contents (abfd)
     bfd *abfd;
{
  bfd_write("S9030000FC\n", 1,11,abfd);
  return true;
}

static int 
DEFUN(srec_sizeof_headers,(abfd, exec),
      bfd *abfd AND
      boolean exec)
{
return 0;
}

static asymbol *
DEFUN(srec_make_empty_symbol, (abfd),
      bfd*abfd)
{
  asymbol *new=  (asymbol *)bfd_zalloc (abfd, sizeof (asymbol));
  new->the_bfd = abfd;
  return new;
}
#define FOO PROTO
#define srec_new_section_hook (FOO(boolean, (*), (bfd *, asection *)))bfd_true
#define srec_get_symtab_upper_bound (PROTO(unsigned int, (*),(bfd *)))bfd_false
#define srec_get_symtab (FOO(unsigned int, (*), (bfd *, asymbol **)))bfd_0
#define srec_get_reloc_upper_bound (FOO(unsigned int, (*),(bfd*, asection *)))bfd_false
#define srec_canonicalize_reloc (FOO(unsigned int, (*),(bfd*,asection *, arelent **, asymbol **))) bfd_0

#define srec_print_symbol (FOO(void,(*),(bfd *, PTR, asymbol *, bfd_print_symbol_type))) bfd_void

#define srec_openr_next_archived_file (FOO(bfd *, (*), (bfd*,bfd*))) bfd_nullvoidptr
#define srec_find_nearest_line (FOO(boolean, (*),(bfd*,asection*,asymbol**,bfd_vma, CONST char**, CONST char**, unsigned int *))) bfd_false
#define srec_generic_stat_arch_elt  (FOO(int, (*), (bfd *,struct stat *))) bfd_0


#define srec_core_file_failing_command (char *(*)())(bfd_nullvoidptr)
#define srec_core_file_failing_signal (int (*)())bfd_0
#define srec_core_file_matches_executable_p (FOO(boolean, (*),(bfd*, bfd*)))bfd_false
#define srec_slurp_armap bfd_true
#define srec_slurp_extended_name_table bfd_true
#define srec_truncate_arname (void (*)())bfd_nullvoidptr
#define srec_write_armap  (FOO( boolean, (*),(bfd *, unsigned int, struct orl *, int, int))) bfd_nullvoidptr
#define srec_get_lineno (struct lineno_cache_entry *(*)())bfd_nullvoidptr
#define	srec_close_and_cleanup	bfd_generic_close_and_cleanup
#define srec_bfd_debug_info_start bfd_void
#define srec_bfd_debug_info_end bfd_void
#define srec_bfd_debug_info_accumulate  (FOO(void, (*), (bfd *,	 asection *))) bfd_void


bfd_target srec_vec =
{
  "srec",			/* name */
  bfd_target_srec_flavour,
  true,				/* target byte order */
  true,				/* target headers byte order */
  (HAS_RELOC | EXEC_P |		/* object flags */
   HAS_LINENO | HAS_DEBUG |
   HAS_SYMS | HAS_LOCALS | DYNAMIC | WP_TEXT | D_PAGED),
  (SEC_CODE|SEC_DATA|SEC_ROM|SEC_HAS_CONTENTS
   |SEC_ALLOC | SEC_LOAD | SEC_RELOC), /* section flags */
  ' ',				/* ar_pad_char */
  16,				/* ar_max_namelen */
    1,				/* minimum alignment */
  _do_getb64, _do_putb64,  _do_getb32, _do_putb32, _do_getb16, _do_putb16, /* data */
  _do_getb64, _do_putb64,  _do_getb32, _do_putb32, _do_getb16, _do_putb16, /* hdrs */

    {_bfd_dummy_target,
       srec_object_p,		/* bfd_check_format */
       (struct bfd_target *(*)()) bfd_nullvoidptr,
       (struct bfd_target *(*)())     bfd_nullvoidptr,
     },
    {
      bfd_false,
      bfd_true,			/* mkobject */
      _bfd_generic_mkarchive,
      bfd_false,
    },
    {				/* bfd_write_contents */
      bfd_false,
      srec_write_object_contents,
      _bfd_write_archive_contents,
      bfd_false,
    },
  JUMP_TABLE(srec)
  };
