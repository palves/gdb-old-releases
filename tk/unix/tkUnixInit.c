/* 
 * tkUnixInit.c --
 *
 *	This file contains Unix-specific interpreter initialization
 *	functions.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tkInt.h"

/*
 *----------------------------------------------------------------------
 *
 * TkPlatformInit --
 *
 *	Performs Unix-specific interpreter initialization related to the
 *      tk_library variable.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Sets "tk_library" Tcl variable.
 *
 *----------------------------------------------------------------------
 */

void
TkPlatformInit(interp)
    Tcl_Interp *interp;
{
    char *libDir = Tcl_GetVar(interp, "tk_library", TCL_GLOBAL_ONLY);
    int freeLib = 0;
    
    if (libDir == NULL) {
	/*
	 * A library directory hasn't already been set, so figure out
	 * which one to use.
	 */

	libDir = getenv("TK_LIBRARY");
	if (libDir == NULL) {
	    /* CYGNUS LOCAL: location independence.  */
#ifdef TK_LIB_TRAILER
	    extern char *tclExecutableName;
	    extern char *Tcl_findSelfBase _ANSI_ARGS_ ((char *argv0));
	    libDir = Tcl_findSelfBase (tclExecutableName);
	    if (libDir != NULL) {
	        char *newLib;
		newLib = (char *) ckalloc((unsigned) (strlen (libDir)
						  + strlen (TK_LIB_TRAILER)
						  + 1));
		strcpy(newLib, libDir);
		strcat(newLib, TK_LIB_TRAILER);
		libDir = newLib;
		freeLib = 1;
	    } else
#endif /* TK_LIB_TRAILER */
	    /* END CYGNUS LOCAL */
	      libDir = TK_LIBRARY;
	}
	Tcl_SetVar(interp, "tk_library", libDir, TCL_GLOBAL_ONLY);
	if (freeLib)
	  ckfree (libDir);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_Init --
 *
 *	This procedure is typically invoked by Tcl_AppInit procedures
 *	to perform additional Tk initialization for a Tcl interpreter,
 *	such as sourcing the "tk.tcl" script.
 *
 * Results:
 *	Returns a standard Tcl completion code and sets interp->result
 *	if there is an error.
 *
 * Side effects:
 *	Depends on what's in the tk.tcl script.
 *
 *----------------------------------------------------------------------
 */

int
Tk_Init(interp)
    Tcl_Interp *interp;		/* Interpreter to initialize. */
{
    static char initCmd[] =
	"if [file exists $tk_library/tk.tcl] {\n\
	    source $tk_library/tk.tcl\n\
	} else {\n\
	    set msg \"can't find $tk_library/tk.tcl; perhaps you \"\n\
	    append msg \"need to\\ninstall Tk or set your TK_LIBRARY \"\n\
	    append msg \"environment variable?\"\n\
	    error $msg\n\
	}";

    return Tcl_Eval(interp, initCmd);
}
