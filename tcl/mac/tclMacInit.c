/*
 * tclMacInit.c --
 *
 *	Contains the Mac-specific interpreter initialization functions.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include <Files.h>
#include <Gestalt.h>
#include <TextUtils.h>
#include <Strings.h>
#include "tclInt.h"
#include "tclPort.h"

/*
 *----------------------------------------------------------------------
 *
 * TclPlatformInit --
 *
 *	Performs Mac-specific interpreter initialization related to the
 *      tcl_platform and tcl_library variables.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Sets "tcl_library" & "tcl_platfrom" Tcl variable
 *
 *----------------------------------------------------------------------
 */

void
TclPlatformInit(interp)
    Tcl_Interp *interp;
{
    char *libDir, *tempPath;
    Tcl_DString path;
    FSSpec macDir;
    long int gestaltResult;
    int minor, major;
    char versStr[10];

    /*
     * Define the tcl_platfrom variable.
     */
    Tcl_SetVar2(interp, "tcl_platform", "platform", "macintosh",
	    TCL_GLOBAL_ONLY);
    Tcl_SetVar2(interp, "tcl_platform", "os", "MacOS", TCL_GLOBAL_ONLY);
    Gestalt(gestaltSystemVersion, &gestaltResult);
    major = (gestaltResult & 0x0000FF00) >> 8;
    minor = (gestaltResult & 0x000000F0) >> 4;
    sprintf(versStr, "%d.%d", major, minor);
    Tcl_SetVar2(interp, "tcl_platform", "osVersion", versStr, TCL_GLOBAL_ONLY);
#if GENERATINGPOWERPC
    Tcl_SetVar2(interp, "tcl_platform", "machine", "ppc", TCL_GLOBAL_ONLY);
#else
    Tcl_SetVar2(interp, "tcl_platform", "machine", "68k", TCL_GLOBAL_ONLY);
#endif

    /*
     * The tcl_library path can be found in several places.  Here is the order
     * in which the are searched.
     *		1) env array
     *		2) System Folder:Extensions:Tool Command Language:
     *		3) use TCL_LIBRARY - which probably won't work
     */
     
    Tcl_DStringInit(&path);
    libDir = Tcl_GetVar2(interp, "env", "TCL_LIBRARY", TCL_GLOBAL_ONLY);
    if (libDir == NULL) {
	tempPath = Tcl_GetVar2(interp, "env", "EXT_FOLDER", TCL_GLOBAL_ONLY);
	if (tempPath != NULL) {
	    Tcl_DStringAppend(&path, tempPath, -1);
	    Tcl_DStringAppend(&path, ":Tool Command Language:lib:tcl", -1);
	    Tcl_DStringAppend(&path, TCL_VERSION, -1);
	    c2pstr(path.string);
	    if (FSMakeFSSpec(0, 0, (StringPtr) path.string, &macDir) == noErr) {
		p2cstr((StringPtr) path.string);
	    	libDir = path.string;
	    }
	}
    }
    if (libDir == NULL) {
	libDir = TCL_LIBRARY;
    }
    Tcl_SetVar(interp, "tcl_library", libDir, TCL_GLOBAL_ONLY);
    Tcl_DStringFree(&path);
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
	"if {[catch {source -rsrc Init}] != 0} {\n\
	if [file exists [info library]:init.tcl] {\n\
	    source [info library]:init.tcl\n\
	} else {\n\
	    set msg \"can't find Init resource or [info library]:init.tcl;\"\n\
	    append msg \" perhaps you need to\\ninstall Tcl or set your \"\n\
	    append msg \"TCL_LIBRARY environment variable?\"\n\
	    error $msg\n\
	}\n}";

    /*
     * We will also add two Macintosh specific commands here.
     */
    Tcl_CreateCommand(interp, "beep", Mac_BeepCmd, NULL, NULL);
    Tcl_CreateCommand(interp, "source", Mac_SourceCmd, NULL, NULL);

    /*
     * For Macintosh applications the Init function may be contained in
     * the application resources.  If it exists we use it - otherwise we
     * look in the tcl_library directory.
     */
    
    return Tcl_Eval(interp, initCmd);
}
