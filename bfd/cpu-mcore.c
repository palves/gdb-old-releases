/* BFD library support routines for Motorolla's MCore architecture
   Copyright (C) 1993, 1999 Free Software Foundation, Inc.

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

const bfd_arch_info_type bfd_mcore_arch =
{
  32,		  		/* 32 bits in a word */
  32,		  		/* 32 bits in an address */
  8,		  		/* 8 bits in a byte */
  bfd_arch_mcore, 		/* Architecture */
  0,		  		/* Machine number - 0 for now */
  "MCore",	  		/* Architecture name */
  "MCore",	  		/* Printable name */
  3,		  		/* Section align power */
  true,		  		/* Is this the default architecture ? */
  bfd_default_compatible,	/* Architecture comparison function */
  bfd_default_scan,	   	/* String to architecture conversion */
  NULL			   	/* Next in list */
};
