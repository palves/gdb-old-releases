/*
 * tkMacPort.h --
 *
 *	This file is included by all of the Tk C files.  It contains
 *	information that may be configuration-dependent, such as
 *	#includes for system include files and a few other things.
 *
 * Copyright (c) 1991-1993 The Regents of the University of California.
 * Copyright (c) 1994 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef _TKMACPORT
#define _TKMACPORT

/*
 * Macro to use instead of "void" for arguments that must have
 * type "void *" in ANSI C;  maps them to type "char *" in
 * non-ANSI systems.  This macro may be used in some of the include
 * files below, which is why it is defined here.
 */

#ifndef VOID
#   ifdef __STDC__
#       define VOID void
#   else
#       define VOID char
#   endif
#endif

#ifndef _TCL
#   include <tcl.h>
#endif
#ifndef _TCLPORT
#   include <tclPort.h>
#endif

#include <limits.h>
#include <stdlib.h>
#include <string.h>


#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#ifdef HAVE_SYS_SELECT_H
#   include <sys/select.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <Xlib.h>
#include <cursorfont.h>
#include <keysym.h>
#include <Xatom.h>
#include <Xfuncproto.h>
#include <Xutil.h>

/*
 * Not all systems declare the errno variable in errno.h. so this
 * file does it explicitly.
 */

extern int errno;

/*
 * Define OPEN_MAX if it isn't already defined for this system.
 */

#ifndef OPEN_MAX
#   define OPEN_MAX 256
#endif

/*
 * The following macro defines the type of the mask arguments to
 * select:
 */

#ifndef NO_FD_SET
#   define SELECT_MASK fd_set
#else
#   ifndef _AIX
	typedef long fd_mask;
#   endif
#   if defined(_IBMR2)
#	define SELECT_MASK void
#   else
#	define SELECT_MASK int
#   endif
#endif

/*
 * Define "NBBY" (number of bits per byte) if it's not already defined.
 */

#ifndef NBBY
#   define NBBY 8
#endif

/*
 * The following macro defines the number of fd_masks in an fd_set:
 */

#if !defined(howmany)
#   define howmany(x, y) (((x)+((y)-1))/(y))
#endif
#ifdef NFDBITS
#   define MASK_SIZE howmany(FD_SETSIZE, NFDBITS)
#else
#   define MASK_SIZE howmany(OPEN_MAX, NBBY*sizeof(fd_mask))
#endif

/*
 * The following macro checks to see whether there is buffered
 * input data available for a stdio FILE.  This has to be done
 * in different ways on different systems.  TK_FILE_GPTR and
 * TK_FILE_COUNT are #defined by autoconf.
 */

#ifdef TK_FILE_COUNT
#   define TK_READ_DATA_PENDING(f) ((f)->TK_FILE_COUNT > 0)
#else
#   ifdef TK_FILE_GPTR
#       define TK_READ_DATA_PENDING(f) ((f)->_gptr < (f)->_egptr)
#   else
#       ifdef TK_FILE_READ_PTR
#	    define TK_READ_DATA_PENDING(f) ((f)->_IO_read_ptr != (f)->_IO_read_end)
#	else
	    /*
	     * Don't know what to do for this system; whoever installs
	     * Tk will have to write a function TkReadDataPending to do
	     * the job.
	     */
	    EXTERN int TkReadDataPending _ANSI_ARGS_((FILE *f));
#           define TK_READ_DATA_PENDING(f) TkReadDataPending(f)
#	endif
#   endif
#endif

/*
 * Substitute Tcl's own versions for several system calls.  The
 * Tcl versions retry automatically if interrupted by signals.
 */

#define open(a,b,c) MacOpen(a,b,c)
#define read(a,b,c) MacRead(a,b,c)
#define waitpid(a,b,c) MacWaitpid(a,b,c)
#define write(a,b,c) MacWrite(a,b,c)
EXTERN int	MacOpen _ANSI_ARGS_((char *path, int oflag, int mode));
EXTERN int	MacRead _ANSI_ARGS_((int fd, VOID *buf, size_t numBytes));
EXTERN int	MacWaitpid _ANSI_ARGS_((pid_t pid, int *statPtr, int options));
EXTERN int	MacWrite _ANSI_ARGS_((int fd, VOID *buf, size_t numBytes));

/*
 * Declarations for various library procedures that may not be declared
 * in any other header file.
 */

extern void		panic();
extern int strncasecmp _ANSI_ARGS_((const char *s1, const char *s2,
	size_t length));
extern int select _ANSI_ARGS_((int width, fd_set *readfds, fd_set *writefds,
	fd_set *exceptfds, struct timeval *timeout));
extern int gettimeofday _ANSI_ARGS_((struct timeval *tp, struct timezone *tz));

#endif /* _TKMACPORT */
