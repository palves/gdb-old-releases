/* Generic BFD library interface and support routines.
   Copyright (C) 1990, 1991, 1992, 1993 Free Software Foundation, Inc.
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

/*
SECTION
	<<typedef bfd>>

	A BFD has type <<bfd>>; objects of this type are the
	cornerstone of any application using BFD. Using BFD
	consists of making references though the BFD and to data in the BFD.

	Here is the structure that defines the type <<bfd>>.  It
	contains the major data about the file and pointers
	to the rest of the data.

CODE_FRAGMENT
.
.struct _bfd 
.{
.    {* The filename the application opened the BFD with.  *}
.    CONST char *filename;                
.
.    {* A pointer to the target jump table.             *}
.    struct bfd_target *xvec;
.
.    {* To avoid dragging too many header files into every file that
.       includes `<<bfd.h>>', IOSTREAM has been declared as a "char
.       *", and MTIME as a "long".  Their correct types, to which they
.       are cast when used, are "FILE *" and "time_t".    The iostream
.       is the result of an fopen on the filename. *}
.    char *iostream;
.
.    {* Is the file descriptor being cached?  That is, can it be closed as
.       needed, and re-opened when accessed later?  *}
.
.    boolean cacheable;
.
.    {* Marks whether there was a default target specified when the
.       BFD was opened. This is used to select which matching algorithm
.       to use to choose the back end. *}
.
.    boolean target_defaulted;
.
.    {* The caching routines use these to maintain a
.       least-recently-used list of BFDs *}
.
.    struct _bfd *lru_prev, *lru_next;
.
.    {* When a file is closed by the caching routines, BFD retains
.       state information on the file here: *}
.
.    file_ptr where;              
.
.    {* and here: (``once'' means at least once) *}
.
.    boolean opened_once;
.
.    {* Set if we have a locally maintained mtime value, rather than
.       getting it from the file each time: *}
.
.    boolean mtime_set;
.
.    {* File modified time, if mtime_set is true: *}
.
.    long mtime;          
.
.    {* Reserved for an unimplemented file locking extension.*}
.
.    int ifd;
.
.    {* The format which belongs to the BFD. (object, core, etc.) *}
.
.    bfd_format format;
.
.    {* The direction the BFD was opened with*}
.
.    enum bfd_direction {no_direction = 0,
.                        read_direction = 1,
.                        write_direction = 2,
.                        both_direction = 3} direction;
.
.    {* Format_specific flags*}
.
.    flagword flags;              
.
.    {* Currently my_archive is tested before adding origin to
.       anything. I believe that this can become always an add of
.       origin, with origin set to 0 for non archive files.   *}
.
.    file_ptr origin;             
.
.    {* Remember when output has begun, to stop strange things
.       from happening. *}
.    boolean output_has_begun;
.
.    {* Pointer to linked list of sections*}
.    struct sec  *sections;
.
.    {* The number of sections *}
.    unsigned int section_count;
.
.    {* Stuff only useful for object files: 
.       The start address. *}
.    bfd_vma start_address;
.
.    {* Used for input and output*}
.    unsigned int symcount;
.
.    {* Symbol table for output BFD (with symcount entries) *}
.    struct symbol_cache_entry  **outsymbols;             
.
.    {* Pointer to structure which contains architecture information*}
.    struct bfd_arch_info *arch_info;
.
.    {* Stuff only useful for archives:*}
.    PTR arelt_data;              
.    struct _bfd *my_archive;     {* The containing archive BFD.  *}
.    struct _bfd *next;           {* The next BFD in the archive.  *}
.    struct _bfd *archive_head;   {* The first BFD in the archive.  *}
.    boolean has_armap;           
.
.    {* A chain of BFD structures involved in a link.  *}
.    struct _bfd *link_next;
.
.    {* A field used by _bfd_generic_link_add_archive_symbols.  This will
.       be used only for archive elements.  *}
.    int archive_pass;
.
.    {* Used by the back end to hold private data. *}
.
.    union 
.      {
.      struct aout_data_struct *aout_data;
.      struct artdata *aout_ar_data;
.      struct _oasys_data *oasys_obj_data;
.      struct _oasys_ar_data *oasys_ar_data;
.      struct coff_tdata *coff_obj_data;
.      struct ecoff_tdata *ecoff_obj_data;
.      struct ieee_data_struct *ieee_data;
.      struct ieee_ar_data_struct *ieee_ar_data;
.      struct srec_data_struct *srec_data;
.      struct tekhex_data_struct *tekhex_data;
.      struct elf_obj_tdata *elf_obj_data;
.      struct nlm_obj_tdata *nlm_obj_data;
.      struct bout_data_struct *bout_data;
.      struct sun_core_struct *sun_core_data;
.      struct trad_core_struct *trad_core_data;
.      struct som_data_struct *som_data;
.      struct hpux_core_struct *hpux_core_data;
.      struct hppabsd_core_struct *hppabsd_core_data;
.      struct sgi_core_struct *sgi_core_data;
.      struct lynx_core_struct *lynx_core_data;
.      struct osf_core_struct *osf_core_data;
.      PTR any;
.      } tdata;
.  
.    {* Used by the application to hold private data*}
.    PTR usrdata;
.
.    {* Where all the allocated stuff under this BFD goes *}
.    struct obstack memory;
.};
.
*/

#include "bfd.h"
#include "sysdep.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "coff/internal.h"
#include "coff/sym.h"
#include "libcoff.h"
#include "libecoff.h"
#undef obj_symbols
#include "libelf.h"

#undef strerror
extern char *strerror();

/** Error handling
    o - Most functions return nonzero on success (check doc for
        precise semantics); 0 or NULL on error.
    o - Internal errors are documented by the value of bfd_error.
        If that is system_call_error then check errno.
    o - The easiest way to report this to the user is to use bfd_perror.
*/

bfd_ec bfd_error = no_error;

CONST char *CONST bfd_errmsgs[] = {
                        "No error",
                        "System call error",
                        "Invalid bfd target",
                        "File in wrong format",
                        "Invalid operation",
                        "Memory exhausted",
                        "No symbols",
                        "No relocation info",
                        "No more archived files",
                        "Malformed archive",
                        "Symbol not found",
                        "File format not recognized",
                        "File format is ambiguous",
                        "Section has no contents",
                        "Nonrepresentable section on output",
			"Symbol needs debug section which does not exist",
			"Bad value",
			"File truncated",
                        "#<Invalid error code>"
                       };

CONST char *
bfd_errmsg (error_tag)
     bfd_ec error_tag;
{
#ifndef errno
  extern int errno;
#endif
  if (error_tag == system_call_error)
    return strerror (errno);

  if ((((int)error_tag <(int) no_error) ||
       ((int)error_tag > (int)invalid_error_code)))
    error_tag = invalid_error_code;/* sanity check */

  return bfd_errmsgs [(int)error_tag];
}

void
DEFUN(bfd_perror,(message),
      CONST char *message)
{
  if (bfd_error == system_call_error)
    perror((char *)message);            /* must be system error then... */
  else {
    if (message == NULL || *message == '\0')
      fprintf (stderr, "%s\n", bfd_errmsg (bfd_error));
    else
      fprintf (stderr, "%s: %s\n", message, bfd_errmsg (bfd_error));
  }
}

 
/** Symbols */


/*
FUNCTION
	bfd_get_reloc_upper_bound

SYNOPSIS
	unsigned int bfd_get_reloc_upper_bound(bfd *abfd, asection *sect);

DESCRIPTION
	Return the number of bytes required to store the
	relocation information associated with section @var{sect}
	attached to bfd @var{abfd}.

*/


unsigned int
DEFUN(bfd_get_reloc_upper_bound,(abfd, asect),
     bfd *abfd AND
     sec_ptr asect)
{
  if (abfd->format != bfd_object) {
    bfd_error = invalid_operation;
    return 0;
  }

  return BFD_SEND (abfd, _get_reloc_upper_bound, (abfd, asect));
}

/*
FUNCTION
	bfd_canonicalize_reloc

SYNOPSIS
	unsigned int bfd_canonicalize_reloc
        	(bfd *abfd,
		asection *sec,
		arelent **loc,
		asymbol	**syms);

DESCRIPTION
	Call the back end associated with the open BFD
	@var{abfd} and translate the external form of the relocation
	information attached to @var{sec} into the internal canonical
	form.  Place the table into memory at @var{loc}, which has
	been preallocated, usually by a call to
	<<bfd_get_reloc_upper_bound>>.

	The @var{syms} table is also needed for horrible internal magic
	reasons.


*/
unsigned int
DEFUN(bfd_canonicalize_reloc,(abfd, asect, location, symbols),
     bfd *abfd AND
     sec_ptr asect AND
     arelent **location AND
     asymbol **symbols)
{
  if (abfd->format != bfd_object) {
    bfd_error = invalid_operation;
    return 0;
  }
  return BFD_SEND (abfd, _bfd_canonicalize_reloc,
		   (abfd, asect, location, symbols));
}

/*
FUNCTION
	bfd_set_reloc

SYNOPSIS
	void bfd_set_reloc
	  (bfd *abfd, asection *sec, arelent **rel, unsigned int count)

DESCRIPTION
	Set the relocation pointer and count within
	section @var{sec} to the values @var{rel} and @var{count}.
	The argument @var{abfd} is ignored.

*/
/*ARGSUSED*/
void
bfd_set_reloc (ignore_abfd, asect, location, count)
     bfd *ignore_abfd;
     sec_ptr asect;
     arelent **location;
     unsigned int count;
{
  asect->orelocation  = location;
  asect->reloc_count = count;
}

/*
FUNCTION
	bfd_set_file_flags

SYNOPSIS
	boolean bfd_set_file_flags(bfd *abfd, flagword flags);

DESCRIPTION
	Set the flag word in the BFD @var{abfd} to the value @var{flags}.

	Possible errors are:
	o <<wrong_format>> - The target bfd was not of object format.
	o <<invalid_operation>> - The target bfd was open for reading.
	o <<invalid_operation>> -
	The flag word contained a bit which was not applicable to the
	type of file.  E.g., an attempt was made to set the <<D_PAGED>> bit
	on a BFD format which does not support demand paging.

*/

boolean
bfd_set_file_flags (abfd, flags)
     bfd *abfd;
     flagword flags;
{
  if (abfd->format != bfd_object) {
    bfd_error = wrong_format;
    return false;
  }

  if (bfd_read_p (abfd)) {
    bfd_error = invalid_operation;
    return false;
  }

  bfd_get_file_flags (abfd) = flags;
  if ((flags & bfd_applicable_file_flags (abfd)) != flags) {
    bfd_error = invalid_operation;
    return false;
  }

return true;
}

void
bfd_assert(file, line)
char *file;
int line;
{
  fprintf(stderr, "bfd assertion fail %s:%d\n",file,line);
}


/*
FUNCTION
	bfd_set_start_address

SYNOPSIS
 	boolean bfd_set_start_address(bfd *abfd, bfd_vma vma);

DESCRIPTION
	Make @var{vma} the entry point of output BFD @var{abfd}.

RETURNS
	Returns <<true>> on success, <<false>> otherwise.
*/

boolean
bfd_set_start_address(abfd, vma)
bfd *abfd;
bfd_vma vma;
{
  abfd->start_address = vma;
  return true;
}


/*
FUNCTION
	bfd_get_mtime

SYNOPSIS
	long bfd_get_mtime(bfd *abfd);

DESCRIPTION
	Return the file modification time (as read from the file system, or
	from the archive header for archive members).

*/

long
bfd_get_mtime (abfd)
     bfd *abfd;
{
  FILE *fp;
  struct stat buf;

  if (abfd->mtime_set)
    return abfd->mtime;

  fp = bfd_cache_lookup (abfd);
  if (0 != fstat (fileno (fp), &buf))
    return 0;

  abfd->mtime = buf.st_mtime;		/* Save value in case anyone wants it */
  return buf.st_mtime;
}

/*
FUNCTION
	bfd_get_size

SYNOPSIS
	long bfd_get_size(bfd *abfd);

DESCRIPTION
	Return the file size (as read from file system) for the file
	associated with BFD @var{abfd}.

	The initial motivation for, and use of, this routine is not
	so we can get the exact size of the object the BFD applies to, since
	that might not be generally possible (archive members for example).
	It would be ideal if someone could eventually modify
	it so that such results were guaranteed.

	Instead, we want to ask questions like "is this NNN byte sized
	object I'm about to try read from file offset YYY reasonable?"
	As as example of where we might do this, some object formats
	use string tables for which the first <<sizeof(long)>> bytes of the
	table contain the size of the table itself, including the size bytes.
	If an application tries to read what it thinks is one of these
	string tables, without some way to validate the size, and for
	some reason the size is wrong (byte swapping error, wrong location
	for the string table, etc.), the only clue is likely to be a read
	error when it tries to read the table, or a "virtual memory
	exhausted" error when it tries to allocate 15 bazillon bytes
	of space for the 15 bazillon byte table it is about to read.
	This function at least allows us to answer the quesion, "is the
	size reasonable?".
*/

long
bfd_get_size (abfd)
     bfd *abfd;
{
  FILE *fp;
  struct stat buf;

  fp = bfd_cache_lookup (abfd);
  if (0 != fstat (fileno (fp), &buf))
    return 0;

  return buf.st_size;
}

/*
FUNCTION
	bfd_get_gp_size

SYNOPSIS
	int bfd_get_gp_size(bfd *abfd);

DESCRIPTION
	Return the maximum size of objects to be optimized using the GP
	register under MIPS ECOFF.  This is typically set by the <<-G>>
	argument to the compiler, assembler or linker.
*/

int
bfd_get_gp_size (abfd)
     bfd *abfd;
{
  if (abfd->xvec->flavour == bfd_target_ecoff_flavour)
    return ecoff_data (abfd)->gp_size;
  return 0;
}

/*
FUNCTION
	bfd_set_gp_size

SYNOPSIS
	void bfd_set_gp_size(bfd *abfd, int i);

DESCRIPTION
	Set the maximum size of objects to be optimized using the GP
	register under ECOFF or MIPS ELF.  This is typically set by
	the <<-G>> argument to the compiler, assembler or linker.
*/

void
bfd_set_gp_size (abfd, i)
     bfd *abfd;
     int i;
{
  if (abfd->xvec->flavour == bfd_target_ecoff_flavour)
    ecoff_data (abfd)->gp_size = i;
  else if (abfd->xvec->flavour == bfd_target_elf_flavour)
    elf_gp_size (abfd) = i;
}

/*
FUNCTION
	bfd_scan_vma

SYNOPSIS
	bfd_vma bfd_scan_vma(CONST char *string, CONST char **end, int base);

DESCRIPTION
	Convert, like <<strtoul>>, a numerical expression
	@var{string} into a <<bfd_vma>> integer, and return that integer.
	(Though without as many bells and whistles as <<strtoul>>.)
	The expression is assumed to be unsigned (i.e., positive).
	If given a @var{base}, it is used as the base for conversion.
	A base of 0 causes the function to interpret the string
	in hex if a leading "0x" or "0X" is found, otherwise
	in octal if a leading zero is found, otherwise in decimal.

	Overflow is not detected.
*/

bfd_vma
DEFUN(bfd_scan_vma,(string, end, base),
      CONST char *string AND
      CONST char **end AND
      int base)
{
  bfd_vma value;
  int digit;

  /* Let the host do it if possible.  */
  if (sizeof(bfd_vma) <= sizeof(unsigned long))
    return (bfd_vma) strtoul (string, 0, base);

  /* A negative base makes no sense, and we only need to go as high as hex.  */
  if ((base < 0) || (base > 16))
    return (bfd_vma) 0;

  if (base == 0)
    {
      if (string[0] == '0')
	{
	  if ((string[1] == 'x') || (string[1] == 'X'))
	    base = 16;
	  /* XXX should we also allow "0b" or "0B" to set base to 2? */
	  else
	    base = 8;
	}
      else
	base = 10;
    }
  if ((base == 16) &&
      (string[0] == '0') && ((string[1] == 'x') || (string[1] == 'X')))
    string += 2;
  /* XXX should we also skip over "0b" or "0B" if base is 2? */
    
/* Speed could be improved with a table like hex_value[] in gas.  */
#define HEX_VALUE(c) \
  (isxdigit(c) ?				\
    (isdigit(c) ?				\
      (c - '0') :				\
      (10 + c - (islower(c) ? 'a' : 'A'))) :	\
    42)

  for (value = 0; (digit = HEX_VALUE(*string)) < base; string++)
    {
      value = value * base + digit;
    }

  if (end)
    *end = string;

  return value;
}

/*
FUNCTION
	stuff

DESCRIPTION
	Stuff which should be documented:

.#define bfd_sizeof_headers(abfd, reloc) \
.     BFD_SEND (abfd, _bfd_sizeof_headers, (abfd, reloc))
.
.#define bfd_find_nearest_line(abfd, sec, syms, off, file, func, line) \
.     BFD_SEND (abfd, _bfd_find_nearest_line,  (abfd, sec, syms, off, file, func, line))
.
.       {* Do these three do anything useful at all, for any back end?  *}
.#define bfd_debug_info_start(abfd) \
.        BFD_SEND (abfd, _bfd_debug_info_start, (abfd))
.
.#define bfd_debug_info_end(abfd) \
.        BFD_SEND (abfd, _bfd_debug_info_end, (abfd))
.
.#define bfd_debug_info_accumulate(abfd, section) \
.        BFD_SEND (abfd, _bfd_debug_info_accumulate, (abfd, section))
.
.
.#define bfd_stat_arch_elt(abfd, stat) \
.        BFD_SEND (abfd, _bfd_stat_arch_elt,(abfd, stat))
.
.#define bfd_set_arch_mach(abfd, arch, mach)\
.        BFD_SEND ( abfd, _bfd_set_arch_mach, (abfd, arch, mach))
.
.#define bfd_get_relocated_section_contents(abfd, link_info, link_order, data, relocateable, symbols) \
.	BFD_SEND (abfd, _bfd_get_relocated_section_contents, \
.                 (abfd, link_info, link_order, data, relocateable, symbols))
. 
.#define bfd_relax_section(abfd, section, link_info, symbols) \
.       BFD_SEND (abfd, _bfd_relax_section, \
.                 (abfd, section, link_info, symbols))
.
.#define bfd_link_hash_table_create(abfd) \
.	BFD_SEND (abfd, _bfd_link_hash_table_create, (abfd))
.
.#define bfd_link_add_symbols(abfd, info) \
.	BFD_SEND (abfd, _bfd_link_add_symbols, (abfd, info))
.
.#define bfd_final_link(abfd, info) \
.	BFD_SEND (abfd, _bfd_final_link, (abfd, info))
.

*/
