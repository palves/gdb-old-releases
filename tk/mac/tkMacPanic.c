/*
 * tkMacPanic.c --
 *
 *	Source code for the "panic" library procedure used in "Simple Shell";
 *	other Mac applications will probably override this with a more robust
 *	application-specific panic procedure.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include <Types.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

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

    /*
     * TODO: We should have our own notification scheme.
     *       Chances are printf won't work if we are about to crash.
     */
    
    vfprintf(stderr, format, varg);
    (void) fflush(stderr);
	
    va_end(varg);

#ifdef TCL_DEBUG
    Debugger();
#else
    abort();
#endif
    
}
#pragma ignore_oldstyle reset

