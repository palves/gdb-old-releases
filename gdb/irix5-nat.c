/* Native support for the SGI Iris running IRIX version 5, for GDB.
   Copyright 1988, 1989, 1990, 1991, 1992, 1993 Free Software Foundation, Inc.
   Contributed by Alessandro Forin(af@cs.cmu.edu) at CMU
   and by Per Bothner(bothner@cs.wisc.edu) at U.Wisconsin.
   Implemented for Irix 4.x by Garrett A. Wollman.
   Modified for Irix 5.x by Ian Lance Taylor.

This file is part of GDB.

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

#include "defs.h"
#include "inferior.h"
#include "gdbcore.h"
#include "target.h"

#include <sys/time.h>
#include <sys/procfs.h>
#include <setjmp.h>		/* For JB_XXX.  */

/* Size of elements in jmpbuf */

#define JB_ELEMENT_SIZE 4

/*
 * See the comment in m68k-tdep.c regarding the utility of these functions.
 *
 * These definitions are from the MIPS SVR4 ABI, so they may work for
 * any MIPS SVR4 target.
 */

void 
supply_gregset (gregsetp)
     gregset_t *gregsetp;
{
  register int regi;
  register greg_t *regp = &(*gregsetp)[0];

  for(regi = 0; regi <= CTX_RA; regi++)
    supply_register (regi, (char *)(regp + regi));

  supply_register (PC_REGNUM, (char *)(regp + CTX_EPC));
  supply_register (HI_REGNUM, (char *)(regp + CTX_MDHI));
  supply_register (LO_REGNUM, (char *)(regp + CTX_MDLO));
  supply_register (CAUSE_REGNUM, (char *)(regp + CTX_CAUSE));
}

void
fill_gregset (gregsetp, regno)
     gregset_t *gregsetp;
     int regno;
{
  int regi;
  register greg_t *regp = &(*gregsetp)[0];

  for (regi = 0; regi <= CTX_RA; regi++)
    if ((regno == -1) || (regno == regi))
      *(regp + regi) = *(greg_t *) &registers[REGISTER_BYTE (regi)];

  if ((regno == -1) || (regno == PC_REGNUM))
    *(regp + CTX_EPC) = *(greg_t *) &registers[REGISTER_BYTE (PC_REGNUM)];

  if ((regno == -1) || (regno == CAUSE_REGNUM))
    *(regp + CTX_CAUSE) = *(greg_t *) &registers[REGISTER_BYTE (PS_REGNUM)];

  if ((regno == -1) || (regno == HI_REGNUM))
    *(regp + CTX_MDHI) = *(greg_t *) &registers[REGISTER_BYTE (HI_REGNUM)];

  if ((regno == -1) || (regno == LO_REGNUM))
    *(regp + CTX_MDLO) = *(greg_t *) &registers[REGISTER_BYTE (LO_REGNUM)];
}

/*
 * Now we do the same thing for floating-point registers.
 * We don't bother to condition on FP0_REGNUM since any
 * reasonable MIPS configuration has an R3010 in it.
 *
 * Again, see the comments in m68k-tdep.c.
 */

void
supply_fpregset (fpregsetp)
     fpregset_t *fpregsetp;
{
  register int regi;

  for (regi = 0; regi < 32; regi++)
    supply_register (FP0_REGNUM + regi,
		     (char *)&fpregsetp->fp_r.fp_regs[regi]);

  supply_register (FCRCS_REGNUM, (char *)&fpregsetp->fp_csr);

  /* FIXME: how can we supply FCRIR_REGNUM?  SGI doesn't tell us. */
}

void
fill_fpregset (fpregsetp, regno)
     fpregset_t *fpregsetp;
     int regno;
{
  int regi;
  char *from, *to;

  for (regi = FP0_REGNUM; regi < FP0_REGNUM + 32; regi++)
    {
      if ((regno == -1) || (regno == regi))
	{
	  from = (char *) &registers[REGISTER_BYTE (regi)];
	  to = (char *) &(fpregsetp->fp_r.fp_regs[regi - FP0_REGNUM]);
	  memcpy(to, from, REGISTER_RAW_SIZE (regi));
	}
    }

  if ((regno == -1) || (regno == FCRCS_REGNUM))
    fpregsetp->fp_csr = *(unsigned *) &registers[REGISTER_BYTE(FCRCS_REGNUM)];
}


/* Figure out where the longjmp will land.
   We expect the first arg to be a pointer to the jmp_buf structure from which
   we extract the pc (JB_PC) that we will land at.  The pc is copied into PC.
   This routine returns true on success. */

int
get_longjmp_target (pc)
     CORE_ADDR *pc;
{
  char buf[TARGET_PTR_BIT / TARGET_CHAR_BIT];
  CORE_ADDR jb_addr;

  jb_addr = read_register (A0_REGNUM);

  if (target_read_memory (jb_addr + JB_PC * JB_ELEMENT_SIZE, buf,
			  TARGET_PTR_BIT / TARGET_CHAR_BIT))
    return 0;

  *pc = extract_address (buf, TARGET_PTR_BIT / TARGET_CHAR_BIT);

  return 1;
}

void
fetch_core_registers (core_reg_sect, core_reg_size, which, reg_addr)
     char *core_reg_sect;
     unsigned core_reg_size;
     int which;			/* Unused */
     unsigned int reg_addr;	/* Unused */
{
  if (core_reg_size != REGISTER_BYTES)
    {
      warning ("wrong size gregset struct in core file");
      return;
    }

  memcpy ((char *)registers, core_reg_sect, core_reg_size);
}

/* Irix 5 uses what appears to be a unique form of shared library
   support.  This is a copy of solib.c modified for Irix 5.  */

#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <sys/param.h>
#include <fcntl.h>

/* <obj.h> includes <sym.h> and <symconst.h>, which causes conflicts
   with our versions of those files included by tm-mips.h.  Prevent
   <obj.h> from including them with some appropriate defines.  */
#define __SYM_H__
#define __SYMCONST_H__
#include <obj.h>

#include "symtab.h"
#include "bfd.h"
#include "symfile.h"
#include "objfiles.h"
#include "command.h"
#include "frame.h"
#include "regex.h"
#include "inferior.h"
#include "language.h"

/* We need to set a breakpoint at a point when we know that the
   mapping of shared libraries is complete.  dbx simply breaks at main
   (or, for FORTRAN, MAIN__), so we do the same.  We can not break at
   the very beginning of main, because the startup code will jump into
   main after the GP initialization instructions.  SOLIB_BKPT_OFFSET
   is used to skip those instructions.  */

#define SOLIB_BKPT_OFFSET 12

static char *bkpt_names[] = {
  "main",
  "MAIN__",
  NULL
};

/* The symbol which starts off the list of shared libraries.  */
#define DEBUG_BASE "__rld_obj_head"

/* How to get the loaded address of a shared library.  */
#define LM_ADDR(so) ((so)->lm.o_base_address)

char shadow_contents[BREAKPOINT_MAX];	/* Stash old bkpt addr contents */

extern CORE_ADDR sigtramp_address, sigtramp_end;

struct so_list {
  struct so_list *next;			/* next structure in linked list */
  struct obj_list ll;
  struct obj lm;			/* copy of link map from inferior */
  struct obj_list *lladdr;		/* addr in inferior lm was read from */
  CORE_ADDR lmend;			/* upper addr bound of mapped object */
  char symbols_loaded;			/* flag: symbols read in yet? */
  char from_tty;			/* flag: print msgs? */
  struct objfile *objfile;		/* objfile for loaded lib */
  struct section_table *sections;
  struct section_table *sections_end;
  struct section_table *textsection;
  bfd *abfd;
};

static struct so_list *so_list_head;	/* List of known shared objects */
static CORE_ADDR debug_base;		/* Base of dynamic linker structures */
static CORE_ADDR breakpoint_addr;	/* Address where end bkpt is set */

/* Local function prototypes */

static void
sharedlibrary_command PARAMS ((char *, int));

static int
enable_break PARAMS ((void));

static int
disable_break PARAMS ((void));

static void
info_sharedlibrary_command PARAMS ((char *, int));

static int
symbol_add_stub PARAMS ((char *));

static struct so_list *
find_solib PARAMS ((struct so_list *));

static struct obj_list *
first_link_map_member PARAMS ((void));

static CORE_ADDR
locate_base PARAMS ((void));

static void
solib_map_sections PARAMS ((struct so_list *));

/*

LOCAL FUNCTION

	solib_map_sections -- open bfd and build sections for shared lib

SYNOPSIS

	static void solib_map_sections (struct so_list *so)

DESCRIPTION

	Given a pointer to one of the shared objects in our list
	of mapped objects, use the recorded name to open a bfd
	descriptor for the object, build a section table, and then
	relocate all the section addresses by the base address at
	which the shared object was mapped.

FIXMES

	In most (all?) cases the shared object file name recorded in the
	dynamic linkage tables will be a fully qualified pathname.  For
	cases where it isn't, do we really mimic the systems search
	mechanism correctly in the below code (particularly the tilde
	expansion stuff?).
 */

static void
solib_map_sections (so)
     struct so_list *so;
{
  char *filename;
  char *scratch_pathname;
  int scratch_chan;
  struct section_table *p;
  struct cleanup *old_chain;
  bfd *abfd;
  
  filename = tilde_expand (so -> lm.o_path);
  old_chain = make_cleanup (free, filename);
  
  scratch_chan = openp (getenv ("PATH"), 1, filename, O_RDONLY, 0,
			&scratch_pathname);
  if (scratch_chan < 0)
    {
      scratch_chan = openp (getenv ("LD_LIBRARY_PATH"), 1, filename,
			    O_RDONLY, 0, &scratch_pathname);
    }
  if (scratch_chan < 0)
    {
      perror_with_name (filename);
    }
  /* Leave scratch_pathname allocated.  abfd->name will point to it.  */

  abfd = bfd_fdopenr (scratch_pathname, gnutarget, scratch_chan);
  if (!abfd)
    {
      close (scratch_chan);
      error ("Could not open `%s' as an executable file: %s",
	     scratch_pathname, bfd_errmsg (bfd_error));
    }
  /* Leave bfd open, core_xfer_memory and "info files" need it.  */
  so -> abfd = abfd;
  abfd -> cacheable = true;

  if (!bfd_check_format (abfd, bfd_object))
    {
      error ("\"%s\": not in executable format: %s.",
	     scratch_pathname, bfd_errmsg (bfd_error));
    }
  if (build_section_table (abfd, &so -> sections, &so -> sections_end))
    {
      error ("Can't find the file sections in `%s': %s", 
	     bfd_get_filename (exec_bfd), bfd_errmsg (bfd_error));
    }

  for (p = so -> sections; p < so -> sections_end; p++)
    {
      /* Relocate the section binding addresses as recorded in the shared
	 object's file by the base address to which the object was actually
	 mapped. */
      p -> addr += (CORE_ADDR) LM_ADDR (so);
      p -> endaddr += (CORE_ADDR) LM_ADDR (so);
      so -> lmend = (CORE_ADDR) max (p -> endaddr, so -> lmend);
      if (STREQ (p -> sec_ptr -> name, ".text"))
	{
	  so -> textsection = p;
	}
    }

  /* Free the file names, close the file now.  */
  do_cleanups (old_chain);
}

/*

LOCAL FUNCTION

	locate_base -- locate the base address of dynamic linker structs

SYNOPSIS

	CORE_ADDR locate_base (void)

DESCRIPTION

	For both the SunOS and SVR4 shared library implementations, if the
	inferior executable has been linked dynamically, there is a single
	address somewhere in the inferior's data space which is the key to
	locating all of the dynamic linker's runtime structures.  This
	address is the value of the symbol defined by the macro DEBUG_BASE.
	The job of this function is to find and return that address, or to
	return 0 if there is no such address (the executable is statically
	linked for example).

	For SunOS, the job is almost trivial, since the dynamic linker and
	all of it's structures are statically linked to the executable at
	link time.  Thus the symbol for the address we are looking for has
	already been added to the minimal symbol table for the executable's
	objfile at the time the symbol file's symbols were read, and all we
	have to do is look it up there.  Note that we explicitly do NOT want
	to find the copies in the shared library.

	The SVR4 version is much more complicated because the dynamic linker
	and it's structures are located in the shared C library, which gets
	run as the executable's "interpreter" by the kernel.  We have to go
	to a lot more work to discover the address of DEBUG_BASE.  Because
	of this complexity, we cache the value we find and return that value
	on subsequent invocations.  Note there is no copy in the executable
	symbol tables.

	Irix 5 is basically like SunOS.

	Note that we can assume nothing about the process state at the time
	we need to find this address.  We may be stopped on the first instruc-
	tion of the interpreter (C shared library), the first instruction of
	the executable itself, or somewhere else entirely (if we attached
	to the process for example).

 */

static CORE_ADDR
locate_base ()
{
  struct minimal_symbol *msymbol;
  CORE_ADDR address = 0;

  msymbol = lookup_minimal_symbol (DEBUG_BASE, symfile_objfile);
  if ((msymbol != NULL) && (SYMBOL_VALUE_ADDRESS (msymbol) != 0))
    {
      address = SYMBOL_VALUE_ADDRESS (msymbol);
    }
  return (address);
}

/*

LOCAL FUNCTION

	first_link_map_member -- locate first member in dynamic linker's map

SYNOPSIS

	static struct link_map *first_link_map_member (void)

DESCRIPTION

	Read in a copy of the first member in the inferior's dynamic
	link map from the inferior's dynamic linker structures, and return
	a pointer to the copy in our address space.
*/

static struct obj_list *
first_link_map_member ()
{
  struct obj_list *lm;
  struct obj_list s;

  read_memory (debug_base, (char *) &lm, sizeof (struct obj_list *));

  if (lm == NULL)
    return NULL;

  /* The first entry in the list is the object file we are debugging,
     so skip it.  */
  read_memory ((CORE_ADDR) lm, (char *) &s, sizeof (struct obj_list));

  return s.next;
}

/*

LOCAL FUNCTION

	find_solib -- step through list of shared objects

SYNOPSIS

	struct so_list *find_solib (struct so_list *so_list_ptr)

DESCRIPTION

	This module contains the routine which finds the names of any
	loaded "images" in the current process. The argument in must be
	NULL on the first call, and then the returned value must be passed
	in on subsequent calls. This provides the capability to "step" down
	the list of loaded objects. On the last object, a NULL value is
	returned.
 */

static struct so_list *
find_solib (so_list_ptr)
     struct so_list *so_list_ptr;	/* Last lm or NULL for first one */
{
  struct so_list *so_list_next = NULL;
  struct obj_list *lm = NULL;
  struct so_list *new;
  
  if (so_list_ptr == NULL)
    {
      /* We are setting up for a new scan through the loaded images. */
      if ((so_list_next = so_list_head) == NULL)
	{
	  /* We have not already read in the dynamic linking structures
	     from the inferior, lookup the address of the base structure. */
	  debug_base = locate_base ();
	  if (debug_base != 0)
	    {
	      /* Read the base structure in and find the address of the first
		 link map list member. */
	      lm = first_link_map_member ();
	    }
	}
    }
  else
    {
      /* We have been called before, and are in the process of walking
	 the shared library list.  Advance to the next shared object. */
      if ((lm = so_list_ptr->ll.next) == NULL)
	{
	  /* We have hit the end of the list, so check to see if any were
	     added, but be quiet if we can't read from the target any more. */
	  int status = target_read_memory ((CORE_ADDR) so_list_ptr -> lladdr,
					   (char *) &(so_list_ptr -> ll),
					   sizeof (struct obj_list));
	  if (status == 0)
	    {
	      lm = so_list_ptr->ll.next;
	    }
	  else
	    {
	      lm = NULL;
	    }
	}
      so_list_next = so_list_ptr -> next;
    }
  if ((so_list_next == NULL) && (lm != NULL))
    {
      /* Get next link map structure from inferior image and build a local
	 abbreviated load_map structure */
      new = (struct so_list *) xmalloc (sizeof (struct so_list));
      memset ((char *) new, 0, sizeof (struct so_list));
      new -> lladdr = lm;
      /* Add the new node as the next node in the list, or as the root
	 node if this is the first one. */
      if (so_list_ptr != NULL)
	{
	  so_list_ptr -> next = new;
	}
      else
	{
	  so_list_head = new;
	}      
      so_list_next = new;
      read_memory ((CORE_ADDR) lm, (char *) &(new -> ll),
		   sizeof (struct obj_list));
      read_memory ((CORE_ADDR) new->ll.data, (char *) &(new -> lm),
		   sizeof (struct obj));
      solib_map_sections (new);
    }
  return (so_list_next);
}

/* A small stub to get us past the arg-passing pinhole of catch_errors.  */

static int
symbol_add_stub (arg)
     char *arg;
{
  register struct so_list *so = (struct so_list *) arg;	/* catch_errs bogon */
  
  so -> objfile = symbol_file_add (so -> lm.o_path, so -> from_tty,
				   (unsigned int) so -> textsection -> addr,
				   0, 0, 0);
  return (1);
}

/*

GLOBAL FUNCTION

	solib_add -- add a shared library file to the symtab and section list

SYNOPSIS

	void solib_add (char *arg_string, int from_tty,
			struct target_ops *target)

DESCRIPTION

*/

void
solib_add (arg_string, from_tty, target)
     char *arg_string;
     int from_tty;
     struct target_ops *target;
{	
  register struct so_list *so = NULL;   	/* link map state variable */

  /* Last shared library that we read.  */
  struct so_list *so_last = NULL;

  char *re_err;
  int count;
  int old;
  
  if ((re_err = re_comp (arg_string ? arg_string : ".")) != NULL)
    {
      error ("Invalid regexp: %s", re_err);
    }
  
  /* Getting new symbols may change our opinion about what is
     frameless.  */
  reinit_frame_cache ();
  /* Not to mention where _sigtramp is.  */
  sigtramp_address = 0;
  
  while ((so = find_solib (so)) != NULL)
    {
      if (so -> lm.o_path[0] && re_exec (so -> lm.o_path))
	{
	  so -> from_tty = from_tty;
	  if (so -> symbols_loaded)
	    {
	      if (from_tty)
		{
		  printf_unfiltered ("Symbols already loaded for %s\n", so -> lm.o_path);
		}
	    }
	  else if (catch_errors
		   (symbol_add_stub, (char *) so,
		    "Error while reading shared library symbols:\n",
		    RETURN_MASK_ALL))
	    {
	      so_last = so;
	      so -> symbols_loaded = 1;
	    }
	}
    }
  
  /* Now add the shared library sections to the section table of the
     specified target, if any.  */
  if (target)
    {
      /* Count how many new section_table entries there are.  */
      so = NULL;
      count = 0;
      while ((so = find_solib (so)) != NULL)
	{
	  if (so -> lm.o_path[0])
	    {
	      count += so -> sections_end - so -> sections;
	    }
	}
      
      if (count)
	{
	  /* Reallocate the target's section table including the new size.  */
	  if (target -> to_sections)
	    {
	      old = target -> to_sections_end - target -> to_sections;
	      target -> to_sections = (struct section_table *)
		xrealloc ((char *)target -> to_sections,
			 (sizeof (struct section_table)) * (count + old));
	    }
	  else
	    {
	      old = 0;
	      target -> to_sections = (struct section_table *)
		xmalloc ((sizeof (struct section_table)) * count);
	    }
	  target -> to_sections_end = target -> to_sections + (count + old);
	  
	  /* Add these section table entries to the target's table.  */
	  while ((so = find_solib (so)) != NULL)
	    {
	      if (so -> lm.o_path[0])
		{
		  count = so -> sections_end - so -> sections;
		  memcpy ((char *) (target -> to_sections + old),
			  so -> sections, 
			  (sizeof (struct section_table)) * count);
		  old += count;
		}
	    }
	}
    }
}

/*

LOCAL FUNCTION

	info_sharedlibrary_command -- code for "info sharedlibrary"

SYNOPSIS

	static void info_sharedlibrary_command ()

DESCRIPTION

	Walk through the shared library list and print information
	about each attached library.
*/

static void
info_sharedlibrary_command (ignore, from_tty)
     char *ignore;
     int from_tty;
{
  register struct so_list *so = NULL;  	/* link map state variable */
  int header_done = 0;
  
  if (exec_bfd == NULL)
    {
      printf_unfiltered ("No exec file.\n");
      return;
    }
  while ((so = find_solib (so)) != NULL)
    {
      if (so -> lm.o_path[0])
	{
	  if (!header_done)
	    {
	      printf_unfiltered("%-12s%-12s%-12s%s\n", "From", "To", "Syms Read",
		     "Shared Object Library");
	      header_done++;
	    }
	  printf_unfiltered ("%-12s",
		  local_hex_string_custom ((unsigned long) LM_ADDR (so),
					   "08l"));
	  printf_unfiltered ("%-12s",
		  local_hex_string_custom ((unsigned long) so -> lmend,
					   "08l"));
	  printf_unfiltered ("%-12s", so -> symbols_loaded ? "Yes" : "No");
	  printf_unfiltered ("%s\n",  so -> lm.o_path);
	}
    }
  if (so_list_head == NULL)
    {
      printf_unfiltered ("No shared libraries loaded at this time.\n");	
    }
}

/*

GLOBAL FUNCTION

	solib_address -- check to see if an address is in a shared lib

SYNOPSIS

	int solib_address (CORE_ADDR address)

DESCRIPTION

	Provides a hook for other gdb routines to discover whether or
	not a particular address is within the mapped address space of
	a shared library.  Any address between the base mapping address
	and the first address beyond the end of the last mapping, is
	considered to be within the shared library address space, for
	our purposes.

	For example, this routine is called at one point to disable
	breakpoints which are in shared libraries that are not currently
	mapped in.
 */

int
solib_address (address)
     CORE_ADDR address;
{
  register struct so_list *so = 0;   	/* link map state variable */
  
  while ((so = find_solib (so)) != NULL)
    {
      if (so -> lm.o_path[0])
	{
	  if ((address >= (CORE_ADDR) so->lm.o_base_address) &&
	      (address < (CORE_ADDR) so -> lmend))
	    {
	      return (1);
	    }
	}
    }
  return (0);
}

/* Called by free_all_symtabs */

void 
clear_solib()
{
  struct so_list *next;
  char *bfd_filename;
  
  while (so_list_head)
    {
      if (so_list_head -> sections)
	{
	  free ((PTR)so_list_head -> sections);
	}
      if (so_list_head -> abfd)
	{
	  bfd_filename = bfd_get_filename (so_list_head -> abfd);
	  bfd_close (so_list_head -> abfd);
	}
      else
	/* This happens for the executable on SVR4.  */
	bfd_filename = NULL;
      
      next = so_list_head -> next;
      if (bfd_filename)
	free ((PTR)bfd_filename);
      free ((PTR)so_list_head);
      so_list_head = next;
    }
  debug_base = 0;
}

/*

LOCAL FUNCTION

	disable_break -- remove the "mapping changed" breakpoint

SYNOPSIS

	static int disable_break ()

DESCRIPTION

	Removes the breakpoint that gets hit when the dynamic linker
	completes a mapping change.

*/

static int
disable_break ()
{
  int status = 1;


  /* Note that breakpoint address and original contents are in our address
     space, so we just need to write the original contents back. */

  if (memory_remove_breakpoint (breakpoint_addr, shadow_contents) != 0)
    {
      status = 0;
    }

  /* For the SVR4 version, we always know the breakpoint address.  For the
     SunOS version we don't know it until the above code is executed.
     Grumble if we are stopped anywhere besides the breakpoint address. */

  if (stop_pc != breakpoint_addr)
    {
      warning ("stopped at unknown breakpoint while handling shared libraries");
    }

  return (status);
}

/*

LOCAL FUNCTION

	enable_break -- arrange for dynamic linker to hit breakpoint

SYNOPSIS

	int enable_break (void)

DESCRIPTION

	Both the SunOS and the SVR4 dynamic linkers have, as part of their
	debugger interface, support for arranging for the inferior to hit
	a breakpoint after mapping in the shared libraries.  This function
	enables that breakpoint.

	For SunOS, there is a special flag location (in_debugger) which we
	set to 1.  When the dynamic linker sees this flag set, it will set
	a breakpoint at a location known only to itself, after saving the
	original contents of that place and the breakpoint address itself,
	in it's own internal structures.  When we resume the inferior, it
	will eventually take a SIGTRAP when it runs into the breakpoint.
	We handle this (in a different place) by restoring the contents of
	the breakpointed location (which is only known after it stops),
	chasing around to locate the shared libraries that have been
	loaded, then resuming.

	For SVR4, the debugger interface structure contains a member (r_brk)
	which is statically initialized at the time the shared library is
	built, to the offset of a function (_r_debug_state) which is guaran-
	teed to be called once before mapping in a library, and again when
	the mapping is complete.  At the time we are examining this member,
	it contains only the unrelocated offset of the function, so we have
	to do our own relocation.  Later, when the dynamic linker actually
	runs, it relocates r_brk to be the actual address of _r_debug_state().

	The debugger interface structure also contains an enumeration which
	is set to either RT_ADD or RT_DELETE prior to changing the mapping,
	depending upon whether or not the library is being mapped or unmapped,
	and then set to RT_CONSISTENT after the library is mapped/unmapped.

	Irix 5, on the other hand, has no such features.  Instead, we
	set a breakpoint at main.
*/

static int
enable_break ()
{
  int success = 0;
  struct minimal_symbol *msymbol;
  char **bkpt_namep;
  CORE_ADDR bkpt_addr;

  /* Scan through the list of symbols, trying to look up the symbol and
     set a breakpoint there.  Terminate loop when we/if we succeed. */

  breakpoint_addr = 0;
  for (bkpt_namep = bkpt_names; *bkpt_namep != NULL; bkpt_namep++)
    {
      msymbol = lookup_minimal_symbol (*bkpt_namep, symfile_objfile);
      if ((msymbol != NULL) && (SYMBOL_VALUE_ADDRESS (msymbol) != 0))
	{
	  bkpt_addr = SYMBOL_VALUE_ADDRESS (msymbol);
#ifdef SOLIB_BKPT_OFFSET
	  /* We only want to skip if bkpt_addr is currently pointing
	     at a GP setting instruction.  */
	  {
	    char buf[4];

	    if (target_read_memory (bkpt_addr, buf, 4) == 0)
	      {
		unsigned long insn;

		insn = extract_unsigned_integer (buf, 4);
		if ((insn & 0xffff0000) == 0x3c1c0000) /* lui $gp,n */
		  bkpt_addr += SOLIB_BKPT_OFFSET;
	      }
	  }
#endif
	  if (target_insert_breakpoint (bkpt_addr, shadow_contents) == 0)
	    {
	      breakpoint_addr = bkpt_addr;
	      success = 1;
	      break;
	    }
	}
    }

  return (success);
}
  
/*
  
GLOBAL FUNCTION
  
	solib_create_inferior_hook -- shared library startup support
  
SYNOPSIS
  
	void solib_create_inferior_hook()
  
DESCRIPTION
  
	When gdb starts up the inferior, it nurses it along (through the
	shell) until it is ready to execute it's first instruction.  At this
	point, this function gets called via expansion of the macro
	SOLIB_CREATE_INFERIOR_HOOK.

	For SunOS executables, this first instruction is typically the
	one at "_start", or a similar text label, regardless of whether
	the executable is statically or dynamically linked.  The runtime
	startup code takes care of dynamically linking in any shared
	libraries, once gdb allows the inferior to continue.

	For SVR4 executables, this first instruction is either the first
	instruction in the dynamic linker (for dynamically linked
	executables) or the instruction at "start" for statically linked
	executables.  For dynamically linked executables, the system
	first exec's /lib/libc.so.N, which contains the dynamic linker,
	and starts it running.  The dynamic linker maps in any needed
	shared libraries, maps in the actual user executable, and then
	jumps to "start" in the user executable.

	For both SunOS shared libraries, and SVR4 shared libraries, we
	can arrange to cooperate with the dynamic linker to discover the
	names of shared libraries that are dynamically linked, and the
	base addresses to which they are linked.

	This function is responsible for discovering those names and
	addresses, and saving sufficient information about them to allow
	their symbols to be read at a later time.

FIXME

	Between enable_break() and disable_break(), this code does not
	properly handle hitting breakpoints which the user might have
	set in the startup code or in the dynamic linker itself.  Proper
	handling will probably have to wait until the implementation is
	changed to use the "breakpoint handler function" method.

	Also, what if child has exit()ed?  Must exit loop somehow.
  */

void 
solib_create_inferior_hook()
{
  if (!enable_break ())
    {
      warning ("shared library handler failed to enable breakpoint");
      return;
    }

  /* Now run the target.  It will eventually hit the breakpoint, at
     which point all of the libraries will have been mapped in and we
     can go groveling around in the dynamic linker structures to find
     out what we need to know about them. */

  clear_proceed_status ();
  stop_soon_quietly = 1;
  stop_signal = 0;
  do
    {
      target_resume (-1, 0, stop_signal);
      wait_for_inferior ();
    }
  while (stop_signal != SIGTRAP);
  stop_soon_quietly = 0;
  
  /* We are now either at the "mapping complete" breakpoint (or somewhere
     else, a condition we aren't prepared to deal with anyway), so adjust
     the PC as necessary after a breakpoint, disable the breakpoint, and
     add any shared libraries that were mapped in. */

  if (DECR_PC_AFTER_BREAK)
    {
      stop_pc -= DECR_PC_AFTER_BREAK;
      write_register (PC_REGNUM, stop_pc);
    }

  if (!disable_break ())
    {
      warning ("shared library handler failed to disable breakpoint");
    }

  solib_add ((char *) 0, 0, (struct target_ops *) 0);
}

/*

LOCAL FUNCTION

	sharedlibrary_command -- handle command to explicitly add library

SYNOPSIS

	static void sharedlibrary_command (char *args, int from_tty)

DESCRIPTION

*/

static void
sharedlibrary_command (args, from_tty)
char *args;
int from_tty;
{
  dont_repeat ();
  solib_add (args, from_tty, (struct target_ops *) 0);
}

void
_initialize_solib()
{
  
  add_com ("sharedlibrary", class_files, sharedlibrary_command,
	   "Load shared object library symbols for files matching REGEXP.");
  add_info ("sharedlibrary", info_sharedlibrary_command, 
	    "Status of loaded shared object libraries.");
}