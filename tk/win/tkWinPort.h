/*
 * winPort.h --
 *
 *	This header file handles porting issues that occur because of
 *	differences between Windows and Unix. It should be the only
 *	file that contains #ifdefs to handle different flavors of OS.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef _WINPORT
#define _WINPORT

#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <malloc.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <time.h>

#ifdef VISUAL_CPLUSPLUS
#    define strncasecmp strnicmp
#    define hypot _hypot
#else /* VISUAL_CPLUSPLUS */
#    define strncasecmp strncmpi
#endif /* VISUAL_CPLUSPLUS */

#define NBBY 8

#define OPEN_MAX 32

/*
 * The following macro checks to see whether there is buffered
 * input data available for a stdio FILE.
 */

#ifdef VISUAL_CPLUSPLUS
#    define TK_READ_DATA_PENDING(f) ((f)->_cnt > 0)
#else /* VISUAL_CPLUSPLUS */
#    define TK_READ_DATA_PENDING(f) ((f)->level > 0)
#endif /* VISUAL_CPLUSPLUS */

/*
 * Define timezone for gettimeofday.
 */

struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};

extern int gettimeofday(struct timeval *, struct timezone *);
extern void panic();

#endif /* _WINPORT */
