/* 
 * tclUnixInit.c --
 *
 *	Contains the Unix-specific interpreter initialization functions.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tclInt.h"
#ifndef NO_UNAME
#   include <sys/utsname.h>
#endif

/*
 *----------------------------------------------------------------------
 *
 * TclPlatformInit --
 *
 *	Performs Unix-specific interpreter initialization related to the
 *      tcl_library and tcl_platform.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Sets "tcl_library" and "tcl_platform" Tcl variables.
 *
 *----------------------------------------------------------------------
 */

void
TclPlatformInit(interp)
    Tcl_Interp *interp;
{
    char *libDir;
#ifndef NO_UNAME
    struct utsname name;
#endif
    int unameOK;
    int freeLib = 0;

    libDir = Tcl_GetVar2(interp, "env", "TCL_LIBRARY", TCL_GLOBAL_ONLY);
    if (libDir == NULL) {
        /* CYGNUS LOCAL: location independence.  */
#ifdef TCL_LIB_TRAILER
        extern char *Tcl_findSelfBase _ANSI_ARGS_ ((char *argv0));
        libDir = Tcl_findSelfBase (tclExecutableName);
	if (libDir != NULL) {
	    char *newLib;
	    newLib = (char *) ckalloc((unsigned) (strlen (libDir)
						  + strlen (TCL_LIB_TRAILER)
						  + 1));
	    strcpy(newLib, libDir);
	    strcat(newLib, TCL_LIB_TRAILER);
	    libDir = newLib;
	    freeLib = 1;
	} else
#endif /* TCL_LIB_TRAILER */
        /* END CYGNUS LOCAL */
	  libDir = TCL_LIBRARY;
    }
    Tcl_SetVar(interp, "tcl_library", libDir, TCL_GLOBAL_ONLY);
    if (freeLib) {
        ckfree (libDir);
    }

    Tcl_SetVar2(interp, "tcl_platform", "platform", "unix", TCL_GLOBAL_ONLY);
    unameOK = 0;
#ifndef NO_UNAME
    if (uname(&name) >= 0) {
	unameOK = 1;
	Tcl_SetVar2(interp, "tcl_platform", "os", name.sysname,
		TCL_GLOBAL_ONLY);
	Tcl_SetVar2(interp, "tcl_platform", "osVersion", name.release,
		TCL_GLOBAL_ONLY);
	Tcl_SetVar2(interp, "tcl_platform", "machine", name.machine,
		TCL_GLOBAL_ONLY);
    }
#endif
    if (!unameOK) {
	Tcl_SetVar2(interp, "tcl_platform", "os", "", TCL_GLOBAL_ONLY);
	Tcl_SetVar2(interp, "tcl_platform", "osVersion", "", TCL_GLOBAL_ONLY);
	Tcl_SetVar2(interp, "tcl_platform", "machine", "", TCL_GLOBAL_ONLY);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_Init --
 *
 *	This procedure is typically invoked by Tcl_AppInit procedures
 *	to perform additional initialization for a Tcl interpreter,
 *	such as sourcing the "init.tcl" script.
 *
 * Results:
 *	Returns a standard Tcl completion code and sets interp->result
 *	if there is an error.
 *
 * Side effects:
 *	Depends on what's in the init.tcl script.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_Init(interp)
    Tcl_Interp *interp;		/* Interpreter to initialize. */
{
    static char initCmd[] =
	"if [file exists [info library]/init.tcl] {\n\
	    source [info library]/init.tcl\n\
	} else {\n\
	    set msg \"can't find [info library]/init.tcl; perhaps you \"\n\
	    append msg \"need to\\ninstall Tcl or set your TCL_LIBRARY \"\n\
	    append msg \"environment variable?\"\n\
	    error $msg\n\
	}";

    return Tcl_Eval(interp, initCmd);
}
