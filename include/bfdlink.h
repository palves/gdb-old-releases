/* bfdlink.h -- header file for BFD link routines
   Copyright 1993 Free Software Foundation, Inc.
   Written by Steve Chamberlain and Ian Lance Taylor, Cygnus Support.

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

#ifndef BFDLINK_H
#define BFDLINK_H

/* Which symbols to strip during a link.  */
enum bfd_link_strip
{
  strip_none,		/* Don't strip any symbols.  */
  strip_debugger,	/* Strip debugging symbols.  */
  strip_some,		/* keep_hash is the list of symbols to keep.  */
  strip_all		/* Strip all symbols.  */
};

/* Which local symbols to discard during a link.  This is irrelevant
   if strip_all is used.  */
enum bfd_link_discard
{
  discard_none,		/* Don't discard any locals.  */
  discard_l,		/* Discard locals with a certain prefix.  */
  discard_all		/* Discard all locals.  */
};

/* These are the possible types of an entry in the BFD link hash
   table.  */

enum bfd_link_hash_type
{
  bfd_link_hash_new,		/* Symbol is new.  */
  bfd_link_hash_undefined,	/* Symbol seen before, but undefined.  */
  bfd_link_hash_weak,		/* Symbol is weak and undefined.  */
  bfd_link_hash_defined,	/* Symbol is defined.  */
  bfd_link_hash_common,		/* Symbol is common.  */
  bfd_link_hash_indirect,	/* Symbol is an indirect link.  */
  bfd_link_hash_warning		/* Like indirect, but warn if referenced.  */
};

/* The linking routines use a hash table which uses this structure for
   its elements.  */

struct bfd_link_hash_entry
{
  /* Base hash table entry structure.  */
  struct bfd_hash_entry root;
  /* Type of this entry.  */
  enum bfd_link_hash_type type;
  /* Whether this symbol has been written out.  */
  boolean written;
  /* Undefined and common entries are kept in a linked list through
     this field.  This field is not in the union because that would
     force us to remove entries from the list when we changed their
     type, which would force the list to be doubly linked, which would
     waste more memory.  When an undefined or common symbol is
     created, it should be added to this list, the head of which is in
     the link hash table itself.  As symbols are defined, they need
     not be removed from the list; anything which reads the list must
     doublecheck the symbol type.  Weak symbols are not kept on this
     list.  */
  struct bfd_link_hash_entry *next;
  /* A union of information depending upon the type.  */
  union
    {
      /* Nothing is kept for bfd_hash_new.  */
      /* bfd_link_hash_undefined, bfd_link_hash_weak.  */
      struct
	{
	  bfd *abfd;		/* BFD symbol was found in.  */
	} undef;
      /* bfd_link_hash_defined.  */
      struct
	{
	  bfd_vma value;	/* Symbol value.  */
	  asection *section;	/* Symbol section.  */
	} def;
      /* bfd_link_hash_indirect, bfd_link_hash_warning.  */
      struct
	{
	  struct bfd_link_hash_entry *link;	/* Real symbol.  */
	  const char *warning;	/* Warning (bfd_link_hash_warning only).  */
	} i;
      /* bfd_link_hash_common.  */
      struct
	{
	  bfd_vma size;		/* Common symbol size.  */
	  asection *section;	/* Symbol section.  */
	} c;
    } u;
};

/* This is the link hash table.  It is a derived class of
   bfd_hash_table.  */

struct bfd_link_hash_table
{
  /* The hash table itself.  */
  struct bfd_hash_table table;
  /* The back end which created this hash table.  This indicates the
     type of the entries in the hash table, which is sometimes
     important information when linking object files of different
     types together.  */
  bfd_target *creator;
  /* A linked list of undefined and common symbols, linked through the
     next field in the bfd_link_hash_entry structure.  */
  struct bfd_link_hash_entry *undefs;
  /* Entries are added to the tail of the undefs list.  */
  struct bfd_link_hash_entry *undefs_tail;
};

/* Look up an entry in a link hash table.  If FOLLOW is true, this
   follows bfd_link_hash_indirect and bfd_link_hash_warning links to
   the real symbol.  */
extern struct bfd_link_hash_entry *bfd_link_hash_lookup
  PARAMS ((struct bfd_link_hash_table *, const char *, boolean create,
	   boolean copy, boolean follow));

/* Traverse a link hash table.  */
extern void bfd_link_hash_traverse
  PARAMS ((struct bfd_link_hash_table *,
	   boolean (*) (struct bfd_link_hash_entry *, PTR),
	   PTR));

/* Add an entry to the undefs list.  */
extern void bfd_link_add_undef
  PARAMS ((struct bfd_link_hash_table *, struct bfd_link_hash_entry *));

/* This structure holds all the information needed to communicate
   between BFD and the linker when doing a link.  */

struct bfd_link_info
{
  /* Function callbacks.  */
  const struct bfd_link_callbacks *callbacks;
  /* true if BFD should generate a relocateable object file.  */
  boolean relocateable;
  /* Which symbols to strip.  */
  enum bfd_link_strip strip;
  /* Which local symbols to discard.  */
  enum bfd_link_discard discard;
  /* The local symbol prefix to discard if using discard_l.  */
  unsigned int lprefix_len;
  const char *lprefix;
  /* true if symbols should be retained in memory, false if they
     should be freed and reread.  */
  boolean keep_memory;
  /* The list of input BFD's involved in the link.  These are chained
     together via the link_next field.  */
  bfd *input_bfds;
  /* If a symbol should be created for each input BFD, this is section
     where those symbols should be placed.  It must be a section in
     the output BFD.  It may be NULL, in which case no such symbols
     will be created.  This is to support CREATE_OBJECT_SYMBOLS in the
     linker command language.  */
  asection *create_object_symbols_section;
  /* Hash table handled by BFD.  */
  struct bfd_link_hash_table *hash;
  /* Hash table of symbols to keep.  This is NULL unless strip is
     strip_some.  */
  struct bfd_hash_table *keep_hash;
  /* Hash table of symbols to report back via notice_callback.  If
     this is NULL no symbols are reported back.  */
  struct bfd_hash_table *notice_hash;
};

/* This structures holds a set of callback functions.  These are
   called by the BFD linker routines.  The first argument to each
   callback function is the bfd_link_info structure being used.  Each
   function returns a boolean value.  If the function returns false,
   then the BFD function which called it will return with a failure
   indication.  */

struct bfd_link_callbacks
{
  /* A function which is called when an object is added from an
     archive.  ABFD is the archive element being added.  NAME is the
     name of the symbol which caused the archive element to be pulled
     in.  */
  boolean (*add_archive_element) PARAMS ((struct bfd_link_info *,
					  bfd *abfd,
					  const char *name));
  /* A function which is called when a symbol is found with multiple
     definitions.  NAME is the symbol which is defined multiple times.
     OBFD is the old BFD, OSEC is the old section, OVAL is the old
     value, NBFD is the new BFD, NSEC is the new section, and NVAL is
     the new value.  OBFD may be NULL.  OSEC and NSEC may be
     bfd_com_section or bfd_ind_section.  */
  boolean (*multiple_definition) PARAMS ((struct bfd_link_info *,
					  const char *name,
					  bfd *obfd,
					  asection *osec,
					  bfd_vma oval,
					  bfd *nbfd,
					  asection *nsec,
					  bfd_vma nval));
  /* A function which is called when a common symbol is defined
     multiple times.  NAME is the symbol appearing multiple times.
     OBFD is the BFD of the existing symbol.  OTYPE is the type of the
     existing symbol, either bfd_link_hash_defined or
     bfd_link_hash_common.  If OTYPE is bfd_link_hash_common, OSIZE is
     the size of the existing symbol.  NBFD is the BFD of the new
     symbol.  NTYPE is the type of the new symbol, either
     bfd_link_hash_defined or bfd_link_hash_common.  If NTYPE is
     bfd_link_hash_common, NSIZE is the size of the new symbol.  */
  boolean (*multiple_common) PARAMS ((struct bfd_link_info *,
				      const char *name,
				      bfd *obfd,
				      enum bfd_link_hash_type otype,
				      bfd_vma osize,
				      bfd *nbfd,
				      enum bfd_link_hash_type ntype,
				      bfd_vma nsize));
  /* A function which is called to add a symbol to a set.  ENTRY is
     the link hash table entry for the set itself (e.g.,
     __CTOR_LIST__).  BITSIZE is the size in bits of an entry in the
     set (typically 32 or 64).  ABFD, SEC and VALUE identify the value
     to add to the set.  */
  boolean (*add_to_set) PARAMS ((struct bfd_link_info *,
				 struct bfd_link_hash_entry *entry,
				 unsigned int bitsize,
				 bfd *abfd, asection *sec, bfd_vma value));
  /* A function which is called when the name of a g++ constructor or
     destructor is found.  This is only called by some object file
     formats.  CONSTRUCTOR is true for a constructor, false for a
     destructor.  BITSIZE is the size in bits of a function pointer.
     NAME is the name of the symbol found.  ABFD, SECTION and VALUE
     are the value of the symbol.  */
  boolean (*constructor) PARAMS ((struct bfd_link_info *,
				  boolean constructor, unsigned int bitsize,
				  const char *name, bfd *abfd, asection *sec,
				  bfd_vma value));
  /* A function which is called when there is a reference to a warning
     symbol.  WARNING is the warning to be issued.  */
  boolean (*warning) PARAMS ((struct bfd_link_info *,
			      const char *warning));
  /* A function which is called when a relocation is attempted against
     an undefined symbol.  NAME is the symbol which is undefined.
     ABFD, SECTION and ADDRESS identify the location from which the
     reference is made.  */
  boolean (*undefined_symbol) PARAMS ((struct bfd_link_info *,
				       const char *name, bfd *abfd,
				       asection *section, bfd_vma address));
  /* A function which is called when a reloc overflow occurs.  ABFD,
     SECTION and ADDRESS identify the location at which the overflow
     occurs.  */
  boolean (*reloc_overflow) PARAMS ((struct bfd_link_info *,
				     bfd *abfd, asection *section,
				     bfd_vma address));
  /* A function which is called when a dangerous reloc is performed.
     The canonical example is an a29k IHCONST reloc which does not
     follow an IHIHALF reloc.  MESSAGE is an appropriate message.
     ABFD, SECTION and ADDRESS identify the location at which the
     problem occurred.  */
  boolean (*reloc_dangerous) PARAMS ((struct bfd_link_info *,
				      const char *message,
				      bfd *abfd, asection *section,
				      bfd_vma address));
  /* A function which is called when a reloc is found to be attached
     to a symbol which is not being written out.  NAME is the name of
     the symbol.  ABFD, SECTION and ADDRESS identify the location of
     the reloc.  */
  boolean (*unattached_reloc) PARAMS ((struct bfd_link_info *,
				       const char *name,
				       bfd *abfd, asection *section,
				       bfd_vma address));
  /* A function which is called when a symbol in notice_hash is
     defined or referenced.  NAME is the symbol.  ABFD, SECTION and
     ADDRESS are the value of the symbol.  If SECTION is
     bfd_und_section, this is a reference.  */
  boolean (*notice) PARAMS ((struct bfd_link_info *, const char *name,
			     bfd *abfd, asection *section, bfd_vma address));
};

/* The linker builds link_order structures which tell the code how to
   include input data in the output file.  */

/* These are the types of link_order structures.  */

enum bfd_link_order_type
{
  bfd_undefined_link_order,	/* Undefined.  */
  bfd_indirect_link_order,	/* Built from a section.  */
  bfd_fill_link_order		/* Fill with a 16 bit constant.  */
};

/* This is the link_order structure itself.  These form a chain
   attached to the section whose contents they are describing.  */

struct bfd_link_order 
{
  /* Next link_order in chain.  */
  struct bfd_link_order *next;
  /* Type of link_order.  */
  enum bfd_link_order_type type;
  /* Offset within output section.  */
  bfd_vma offset;  
  /* Size within output section.  */
  bfd_size_type size;
  /* Type specific information.  */
  union 
    {
      struct 
	{
	  /* Section to include.  If this is used, then
	     section->output_section must be the section the
	     link_order is attached to, section->output_offset must
	     equal the link_order offset field, and section->_raw_size
	     must equal the link_order size field.  Maybe these
	     restrictions should be relaxed someday.  */
	  asection *section;
	} indirect;
      struct
	{
	  /* Value to fill with.  */
	  unsigned int value;
	} fill;
    } u;
};

/* Allocate a new link_order for a section.  */
extern struct bfd_link_order *bfd_new_link_order PARAMS ((bfd *, asection *));

#endif