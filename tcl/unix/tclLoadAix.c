/* 
 * tclLoadAix.c --
 *
 *	This procedure provides a version of the TclLoadFile that works
 *	under AIX.  As of 8/12/95 this code has never been tested and
 *	probably doesn't work.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tcl.h"
#include <sys/types.h>
#include <sys/ldr.h>

/*
 *----------------------------------------------------------------------
 *
 * TclLoadFile --
 *
 *	Dynamically loads a binary code file into memory and returns
 *	the addresses of two procedures within that file, if they
 *	are defined.
 *
 * Results:
 *	A standard Tcl completion code.  If an error occurs, an error
 *	message is left in interp->result.  *proc1Ptr and *proc2Ptr
 *	are filled in with the addresses of the symbols given by
 *	*sym1 and *sym2, or NULL if those symbols can't be found.
 *
 * Side effects:
 *	New code suddenly appears in memory.
 *
 *----------------------------------------------------------------------
 */

int
TclLoadFile(interp, fileName, sym1, sym2, proc1Ptr, proc2Ptr)
    Tcl_Interp *interp;		/* Used for error reporting. */
    char *fileName;		/* Name of the file containing the desired
				 * code. */
    char *sym1, *sym2;		/* Names of two procedures to look up in
				 * the file's symbol table. */
    Tcl_PackageInitProc **proc1Ptr, **proc2Ptr;
				/* Where to return the addresses corresponding
				 * to sym1 and sym2. */
{
    char *errMsgs[100];
    int i;

    /*
     * Ideally, we should be looking up the names of two symbols in
     * the library's symbol table, but it appears that no symbol table
     * is available so we just use the entry point as the first symbol
     * and return NULL for the second symbol.
    *proc1Ptr = (Tcl_PackageInitProc *) load(fileName, 1, (char *) NULL);
    if (*proc1Ptr != NULL) {
	*proc2Ptr = NULL;
	return TCL_OK;
    }
    Tcl_AppendResult(interp, "couldn't load file \"", fileName,
	"\":", (char *) NULL);
    if (loadquery(L_GETMESSAGES, (void *) errMsgs, 100) < 0) {
	Tcl_AppendResult(interp, " reason unknown", (char *) NULL);
    } else {
	for (i = 0; errMsgs[i] != NULL; i++) {
	    Tcl_AppendResult(interp, "\n    ", errMsgs[i], (char *) NULL);
	}
    }
    return TCL_ERROR;
}
