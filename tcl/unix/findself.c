/* findself.c - find executable in the filesystem.
 * Copyright 1996 Cygnus Support.
 * Cygnus Support grants unlimited permission to use, copy,
 * distribute, and modify.
 */

/* NOTE: This file must compile in both the Tcl and the Perl build
   environments.  This can make things a little weird.  We use the
   define TCL_LIBRARY to differentiate between the two.  In the future
   we might have to handle more cases here.

   If you change this file, please change it in all places in which it
   occurs.  Currently this is devo/perl and devo/tcl/unix.  Yes, it
   would be good to fix this.  Yes, it is a pain to fix.  */

/* Another note: the caching implementation is required.  Perl in
   particular knows that caching is used here.  */


/*
 * Copyright (c) 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jan-Simon Pendry.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* From Tcl:
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * The contents of "license.terms" are as follows:
 *
 * -------------------------------------------------------------------------
 * This software is copyrighted by the Regents of the University of
 * California, Sun Microsystems, Inc., and other parties.  The following
 * terms apply to all files associated with the software unless explicitly
 * disclaimed in individual files.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 *
 * IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY
 * DERIVATIVES THEREOF, EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE
 * IS PROVIDED ON AN "AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE
 * NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 * MODIFICATIONS.
 *
 * RESTRICTED RIGHTS: Use, duplication or disclosure by the government
 * is subject to the restrictions as set forth in subparagraph (c) (1) (ii)
 * of the Rights in Technical Data and Computer Software Clause as DFARS
 * 252.227-7013 and FAR 52.227-19.
 * -------------------------------------------------------------------------
 */

#ifdef TCL_LIBRARY

/*
 * Tcl build environment.
 */

/* Exported function names.  */
#define find_self_base Tcl_findSelfBase

#define find_malloc ckalloc
#define find_free ckfree
#define find_realloc ckrealloc

#include "tclInt.h"
#include "tclPort.h"

#else /* TCL_LIBRARY */

/*
 * Perl build environment.
 */

/* Exported function names.  */
#define find_self_base Perl_find_self_base

#define find_malloc safemalloc
#define find_free safefree
#define find_realloc saferealloc

#include "EXTERN.h"
#include "perl.h"

/* perl.c says this causes problems.  Let's hope not.  */
#ifdef I_UNISTD
#include <unistd.h>
#endif

/* Use <stdlib.h> in all cases.  If perl.h has already included
   it, then avoid it.  */
#if defined(I_STDLIB) && ! defined(STANDARD_C)
#    include <stdlib.h>
#endif

/* Turn Metaconfig defines into Autoconf defines.  */
#ifdef HAS_REALPATH
#define HAVE_REALPATH
#endif
#ifdef HAS_GETCWD
#define HAVE_GETCWD
#endif

#endif /* TCL_LIBRARY */



/* Temporary.  */
static char resolved[MAXPATHLEN + 1];

/* Saved base result.  */
static char *cached_base = NULL;

/* Whether the cache is valid.  Separate because we can cache NULL.  */
static int cache_valid = 0;

#ifndef HAVE_REALPATH

#ifndef S_IFLNK
/* If S_IFLNK isn't defined, then we don't have symlinks.  So use stat
   instead of lstat.  */
#define lstat stat
#endif /* HAVE_LSTAT */

#ifndef HAVE_GETCWD
#define getcwd(Dir,Len)  getwd (Dir)
#endif /* HAVE_GETCWD */

/* Replacement for machines missing this function.  Find real
   pathname, removing all ".", "..", and symlink components.  Returns
   RESOLVED on success, NULL on failure.  This implementation doesn't
   mess around with errno; we don't need it.

   Implementation derived from FreeBSD implementation.  */
static char *
realpath (path, resolved)
     char *path;
     char *resolved;
{
  char saved[MAXPATHLEN];
  struct stat sb;
  int n, rootd;
  char *p, *q, wbuf[MAXPATHLEN];

  /* Save the current directory.  This isn't ideal, but fchdir isn't
     universal, and even if it exists it might not work (read the
     SunOS fchdir man page).  */
  if (getcwd (saved, MAXPATHLEN) == NULL)
    return (NULL);

  /*
   * Find the dirname and basename from the path to be resolved.
   * Change directory to the dirname component.
   * lstat the basename part.
   *     if it is a symlink, read in the value and loop.
   *     if it is a directory, then change to that directory.
   * get the current directory name and append the basename.
   */
  (void)strncpy(resolved, path, MAXPATHLEN - 1);
  resolved[MAXPATHLEN - 1] = '\0';
loop:
  q = strrchr(resolved, '/');
  if (q != NULL)
    {
      p = q + 1;
      if (q == resolved)
	q = "/";
      else
	{
	  do
	    {
	      --q;
	    }
	  while (q > resolved && *q == '/');
	  q[1] = '\0';
	  q = resolved;
	}
      if (chdir(q) < 0)
	goto err1;
    }
  else
    p = resolved;

  /* Deal with the last component. */
  if (*p != '\0' && lstat(p, &sb) == 0)
    {
#ifdef S_ISLNK
      if (S_ISLNK(sb.st_mode))
	{
	  n = readlink(p, resolved, MAXPATHLEN);
	  if (n < 0)
	    goto err1;
	  resolved[n] = '\0';
	  goto loop;
	}
#endif /* S_ISLNK */
      if (S_ISDIR(sb.st_mode))
	{
	  if (chdir(p) < 0)
	    goto err1;
	  p = "";
	}
    }

  /*
   * Save the last component name and get the full pathname of
   * the current directory.
   */
  strcpy(wbuf, p);
  if (getcwd(resolved, MAXPATHLEN) == 0)
    goto err1;

  /*
   * Join the two strings together, ensuring that the right thing
   * happens if the last component is empty, or the dirname is root.
   */
  if (resolved[0] == '/' && resolved[1] == '\0')
    rootd = 1;
  else
    rootd = 0;

  if (*wbuf)
    {
      if (strlen(resolved) + strlen(wbuf) + rootd + 1 > MAXPATHLEN)
	goto err1;
      if (rootd == 0)
	(void)strcat(resolved, "/");
      (void)strcat(resolved, wbuf);
    }

  chdir (saved);
  return (resolved);

err1:
  chdir (saved);
err2:
  return (NULL);
}
#endif /* HAVE_REALPATH */

/* Use PATH environment variable to search filesystem for this
   program.  Implementation mostly lifted from tcl/unix/tclUnixFile.c.
   Return value is malloc()d.  */
static char *
search_path (argv0)
     char *argv0;
{
  int avlen = strlen (argv0);
  char *path, *name;
  char *scratch;
  int scratch_size;

  path = getenv ("PATH");
  if (path == NULL)
    {
      /* Use the default from sh.  This is unlikely to be very
	 important.  */
      path = ":/bin:/usr/bin";
    }

  scratch_size = 255;
  scratch = (char *) find_malloc (scratch_size);

  while (*path)
    {
      name = path;
      while ((*path != ':') && *path)
	++path;

      if (scratch_size <= path - name + 1 + avlen)
	{
	  scratch_size = path - name + 1 + avlen;
	  scratch = (char *) find_realloc (scratch, scratch_size);
	  if (scratch == NULL)
	    return (NULL);
	}
      strncpy (scratch, name, path - name);
      if (path[-1] != '/')
	{
	  scratch[path - name] = '/';
	  scratch[path - name + 1] = '\0';
	}
      else
	scratch[path - name] = '\0';
      strcat (scratch, argv0);

      if (access (scratch, X_OK) == 0)
	{
	  /* Note that we don't care if the returned path is relative
	     to ".".  That is automatically handled by realpath()
	     later on.  */
	  return (scratch);
	}

      ++path;
    }

  /* Failure.  */
  find_free (scratch);
  return (NULL);
}

/* Argument is the value of argv[0].
 * Return is the absolute file name of the program.  The return value
 * will not contain and ".", "..", or symlink components.
 * The return value is private and should not be freed.
 */
static char *
find_self (argv0)
     char *argv0;
{
  char *found_name, *rp;

  /* Look for relative path.  If the path contains a "/", we assume it
     is relative to ".".  Else we search $PATH.  Since paths relative
     to "." and absolute paths are handled the same way, we merge the
     two tests.  */
  if (argv0 != NULL && strchr (argv0, '/') == NULL)
    found_name = search_path (argv0);
  else
    found_name = argv0;
  if (found_name == NULL)
    return (NULL);

  /* It is easier to cast than to try to write a decl that can be used
     everywhere.  */
  rp = (char *) realpath (found_name, resolved);
  if (found_name != argv0)
      find_free (found_name);
  if (rp == NULL)
      return (NULL);

  return (resolved);
}

/* Like find_self, but returns the path to the base of the hierarchy
   in which the program is assumed to be rooted.  Basically this
   strips off the program name and the last three path segments (assumed
   to be "arch/ARCH/bin".

   Return value points to static memory which should not be
   overwritten.  */
char *
find_self_base (argv0)
     char *argv0;
{
  char *self;

  if (cache_valid)
    return (cached_base);

  self = getenv ("INETHOME");
  if (self != NULL)
      return (self);

  self = find_self (argv0);
  if (self != NULL)
    {
      int i;
      char *base;

      cached_base = (char *) find_malloc (strlen (self) + 1);
      strcpy (cached_base, self);
      for (i = 0; i < 3; ++i)
	{
	  base = strrchr (cached_base, '/');
	  if (base == NULL	/* Missing slash means failure.  */
	      || (i == 1 && strcmp (base + 1, "bin"))
				/* Third component must be "bin".  */
	      || (i == 2 && strncmp (base + 1, "H-", 2))
				/* First component must start with "H-".  */
	      )
	    {
	      find_free (cached_base);
	      cached_base = NULL;
	      break;
	    }
	  *base = '\0';
	}
      cache_valid = 1;
    }
  return (cached_base);
}
