/* 
 * tkMacInit.c --
 *
 *	This file contains Mac-specific interpreter initialization
 *	functions.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include <Files.h>
#include <TextUtils.h>
#include <Strings.h>
#include "tkInt.h"

/*
 *----------------------------------------------------------------------
 *
 * TkPlatformInit --
 *
 *	Performs Mac-specific interpreter initialization related to the
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
    char *libDir, *tempPath;
    Tcl_DString path;
    FSSpec macDir;

    Tcl_DStringInit(&path);

    /*
     * The tk_library path can be found in several places.  Here is the order
     * in which the are searched.
     *		1) the variable may alreasy exist
     *		2) env array
     *		3) System Folder:Extensions:Tool Command Language:
     *		4) use TK_LIBRARY - which probably won't work
     */
     
    libDir = Tcl_GetVar(interp, "tk_library", TCL_GLOBAL_ONLY);
    if (libDir == NULL) {
	libDir = Tcl_GetVar2(interp, "env", "TK_LIBRARY", TCL_GLOBAL_ONLY);
    }
    if (libDir == NULL) {
	tempPath = Tcl_GetVar2(interp, "env", "EXT_FOLDER", TCL_GLOBAL_ONLY);
	if (tempPath != NULL) {
	    Tcl_DStringAppend(&path, tempPath, -1);
	    Tcl_DStringAppend(&path, ":Tool Command Language:lib:tk", -1);
	    Tcl_DStringAppend(&path, TK_VERSION, -1);
	    c2pstr(path.string);
	    if (FSMakeFSSpec(0, 0, (StringPtr) path.string, &macDir) == noErr) {
		p2cstr((StringPtr) path.string);
	    	libDir = path.string;
	    }
	}
    }
    if (libDir == NULL) {
	libDir = TK_LIBRARY;
    }
    Tcl_SetVar(interp, "tk_library", libDir, TCL_GLOBAL_ONLY);
    Tcl_DStringFree(&path);
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
	"if {[catch {source -rsrc tk}] != 0} {\n\
	if [file exists $tk_library:tk.tcl] {\n\
	    source $tk_library:tk.tcl\n\
	} else {\n\
	    set msg \"can't find tk resource or $tk_library:tk.tcl;\"\n\
	    append msg \" perhaps you need to\\ninstall Tk or set your \"\n\
	    append msg \"TK_LIBRARY environment variable?\"\n\
	    error $msg\n\
	}\n}";

    return Tcl_Eval(interp, initCmd);
}
