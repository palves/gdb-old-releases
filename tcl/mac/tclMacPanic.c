/* 
 * tclMacPanic.c --
 *
 *	Source code for the "panic" library procedure used in "Simple Shell";
 *	other Mac applications will probably override this with a more robust
 *	application-specific panic procedure.
 *
 * Copyright 1993-1994 Lockheed Missle & Space Company, AI Center
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */



#include <Types.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef THINK_C
#   include <console.h>
#elif defined(__MWERKS__)
#   include <SIOUX.h>
#endif

#include "tclInt.h"

/*
 *----------------------------------------------------------------------
 *
 * panic --
 *
 *	Print an error message and kill the process.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The process dies, entering the debugger if possible.
 *
 *----------------------------------------------------------------------
 */

#pragma ignore_oldstyle on
void
panic(char * format, ...)
{
    va_list varg;
	
    va_start(varg, format);
	
    vfprintf(stderr, format, varg);
    (void) fflush(stderr);
	
    va_end(varg);

    /*
     * Make console pause so user can see error
     */
#if defined(THINK_C)
    console_options.pause_atexit = 1;
#endif

    abort();
}
#pragma ignore_oldstyle reset
