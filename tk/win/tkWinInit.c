/* 
 * tkWinInit.c --
 *
 *	This file contains Windows-specific interpreter initialization
 *	functions.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tkWinInt.h"
#include <winreg.h>


/*
 *----------------------------------------------------------------------
 *
 * TkPlatformInit --
 *
 *	Performs Windows-specific interpreter initialization related to the
 *      tk_library variable.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Sets "tk_library" Tcl variable
 *
 *----------------------------------------------------------------------
 */

void
TkPlatformInit(interp)
    Tcl_Interp *interp;
{
    char *ptr;
    HKEY regKey;
    DWORD size = MAX_PATH;
    DWORD result;
    OSVERSIONINFO info;
    int isWin32s;		/* True if we are running under Win32s. */
    Tcl_DString ds;
    Tcl_DStringInit(&ds);

    /*
     * Find out if we're running on Win32s
     */

    info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&info);
    isWin32s = (info.dwPlatformId == VER_PLATFORM_WIN32s);

    /*
     * Look in the environment and the registry for a TK_LIBRARY
     * entry.  Store the result in the dynamic string.
     */

    ptr = Tcl_GetVar2(interp, "env", "TK_LIBRARY", TCL_GLOBAL_ONLY);
    if (ptr == NULL) {
	Tcl_DStringAppend(&ds, "Software\\SUNW\\Tk\\", -1);
	Tcl_DStringAppend(&ds, TK_VERSION, -1);

	/*
	 * Open the registry and look for the TK_LIBRARY key.  Note that
	 * Windows 3.1 only supports HKEY_CLASSES_ROOT and single valued
	 * keys.
	 */

	if (isWin32s) {
	    RegOpenKeyEx(HKEY_CLASSES_ROOT, Tcl_DStringValue(&ds), 0,
		    KEY_READ, &regKey);
	    Tcl_DStringSetLength(&ds, MAX_PATH);
	    result = RegQueryValue(regKey, "TK_LIBRARY",
		    Tcl_DStringValue(&ds), &size);
	} else {
	    RegOpenKeyEx(HKEY_LOCAL_MACHINE, Tcl_DStringValue(&ds), 0,
		KEY_READ, &regKey);
	    Tcl_DStringSetLength(&ds, MAX_PATH);
	    result = RegQueryValueEx(regKey, "TK_LIBRARY", NULL, NULL,
		Tcl_DStringValue(&ds), &size);
	}

	if (result == ERROR_SUCCESS) {
	    RegCloseKey(regKey);
	    Tcl_DStringSetLength(&ds, size);
	} else {
	    Tcl_DStringSetLength(&ds, 0);
	}
    } else {
	Tcl_DStringSetLength(&ds, 0);
	Tcl_DStringAppend(&ds, ptr, -1);
    }

    /*
     * If the path doesn't exist, look for the library relative to the
     * tk.dll library.
     */

    if ((Tcl_DStringLength(&ds) == 0)
	    || (access(Tcl_DStringValue(&ds), 0) != 0)) {
	char *lastSlash = NULL;
	char libName[13];	/* used to hold constructed library name */
	HMODULE hModule;	/* Handle to the tkxx.dll module. */

	/*
	 * Path was invalid, so we need to construct a default path from the
	 * location of the executable.  Convert backslashes at the same time.
	 */

	Tcl_DStringSetLength(&ds, MAX_PATH);
	sprintf(libName, "tk%d%d.dll", TK_MAJOR_VERSION, TK_MINOR_VERSION);
	hModule = GetModuleHandle(libName);
	Tcl_DStringSetLength(&ds, GetModuleFileName(hModule,
		Tcl_DStringValue(&ds), MAX_PATH));
	if (Tcl_DStringLength(&ds) > 0) {
	    lastSlash = strrchr(Tcl_DStringValue(&ds), '\\');
	    Tcl_DStringSetLength(&ds, lastSlash - Tcl_DStringValue(&ds));
	    Tcl_DStringAppend(&ds, "../lib/tk", -1);
	    Tcl_DStringAppend(&ds, TK_VERSION, -1);
	}
    }

    /*
     * Convert backslashes to forward slashes in path names.
     */

    for (ptr = Tcl_DStringValue(&ds); *ptr != '\0'; ptr++) {
	if (*ptr == '\\') {
	    *ptr = '/';
	}
    }

    Tcl_SetVar(interp, "tk_library", Tcl_DStringValue(&ds), TCL_GLOBAL_ONLY);
    Tcl_DStringFree(&ds);
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
