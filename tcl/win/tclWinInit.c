/* 
 * tclWinInit.c --
 *
 *	Contains the Windows-specific interpreter initialization functions.
 *
 * Copyright (c) 1994-1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#include "tclInt.h"
#include "tclPort.h"
#include <winreg.h>

/*
 * The following arrays contain the human readable strings for the Windows
 * platform and processor values.
 */


static char* platforms[] = { "Win32s", "Windows 95", "Windows NT" };
static const int numPlatforms = sizeof(platforms);
static char* processors[] = { "intel", "mips", "alpha", "ppc" };
static const int numProcessors = sizeof(processors);

/*
 *----------------------------------------------------------------------
 *
 * TclPlatformInit --
 *
 *	Performs Windows-specific interpreter initialization related to the
 *	tcl_library variable.  Also sets up the HOME environment variable
 *	if it is not already set.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Sets "tcl_library" and "env(HOME)" Tcl variables
 *
 *----------------------------------------------------------------------
 */

void
TclPlatformInit(interp)
    Tcl_Interp *interp;
{
    char *ptr;
    char buffer[13];
    DWORD result;
    HKEY regKey;
    long size = MAX_PATH;
    Tcl_DString ds;
    OSVERSIONINFO osInfo;
    SYSTEM_INFO sysInfo;
    int isWin32s;		/* True if we are running under Win32s. */
    
    Tcl_DStringInit(&ds);

    /*
     * Find out what kind of system we are running on.
     */

    osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osInfo);

    isWin32s = (osInfo.dwPlatformId == VER_PLATFORM_WIN32s);

    /*
     * Since Win32s doesn't support GetSystemInfo, we use a default value.
     */

    if (!isWin32s) {
	GetSystemInfo(&sysInfo);
    } else {
#ifdef VISUAL_CPLUSPLUS
	sysInfo.wProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
#else
	sysInfo.u.s.wProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
#endif
    }

    /*
     * Define the tcl_platform array.
     */

    Tcl_SetVar2(interp, "tcl_platform", "platform", "windows",
	    TCL_GLOBAL_ONLY);
    if ((osInfo.dwPlatformId >= 0) && (osInfo.dwPlatformId < numPlatforms)) {
	Tcl_SetVar2(interp, "tcl_platform", "os",
		platforms[osInfo.dwPlatformId], TCL_GLOBAL_ONLY);
    }
    sprintf(buffer, "%d.%d", osInfo.dwMajorVersion, osInfo.dwMinorVersion);
    Tcl_SetVar2(interp, "tcl_platform", "osVersion", buffer, TCL_GLOBAL_ONLY);
#ifdef VISUAL_CPLUSPLUS
    if ((sysInfo.wProcessorArchitecture >= 0)
	    && (sysInfo.wProcessorArchitecture < numProcessors)) {
	Tcl_SetVar2(interp, "tcl_platform", "machine",
		processors[sysInfo.wProcessorArchitecture],
		TCL_GLOBAL_ONLY);
    }
#else
    if ((sysInfo.u.s.wProcessorArchitecture >= 0)
	    && (sysInfo.u.s.wProcessorArchitecture < numProcessors)) {
	Tcl_SetVar2(interp, "tcl_platform", "machine",
		processors[sysInfo.u.s.wProcessorArchitecture],
		TCL_GLOBAL_ONLY);
    }
#endif

    /*
     * Look in the environment and the registry for a TCL_LIBRARY
     * entry.  Store the result in the dynamic string.
     */

    ptr = Tcl_GetVar2(interp, "env", "TCL_LIBRARY", TCL_GLOBAL_ONLY);
    if (ptr == NULL) {
	Tcl_DStringAppend(&ds, "Software\\SUNW\\Tcl\\", -1);
	Tcl_DStringAppend(&ds, TCL_VERSION, -1);

	/*
	 * Open the registry and look for the TCL_LIBRARY key.  Note that
	 * Windows 3.1 only supports HKEY_CLASSES_ROOT and single valued
	 * keys.
	 */

	if (isWin32s) {
	    RegOpenKeyEx(HKEY_CLASSES_ROOT, Tcl_DStringValue(&ds), 0,
		    KEY_READ, &regKey);
	    Tcl_DStringSetLength(&ds, MAX_PATH);
	    result = RegQueryValue(regKey, "TCL_LIBRARY",
		    Tcl_DStringValue(&ds), &size);
	} else {
	    RegOpenKeyEx(HKEY_LOCAL_MACHINE, Tcl_DStringValue(&ds), 0,
		KEY_READ, &regKey);
	    Tcl_DStringSetLength(&ds, MAX_PATH);
	    result = RegQueryValueEx(regKey, "TCL_LIBRARY", NULL, NULL,
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
     * tcl.dll library.
     */

    if ((Tcl_DStringLength(&ds) == 0)
	    || (access(Tcl_DStringValue(&ds), 0) != 0)) {
	char *lastSlash = NULL;
	HMODULE hModule;	/* Handle to the tclxx.dll module. */

	/*
	 * Path was invalid, so we need to construct a default path from the
	 * location of the executable.  Convert backslashes at the same time.
	 */

	Tcl_DStringSetLength(&ds, MAX_PATH);
	sprintf(buffer, "tcl%d%d.dll", TCL_MAJOR_VERSION, TCL_MINOR_VERSION);
	hModule = GetModuleHandle(buffer);
	Tcl_DStringSetLength(&ds, GetModuleFileName(hModule,
		Tcl_DStringValue(&ds), MAX_PATH));
	if (Tcl_DStringLength(&ds) > 0) {
	    lastSlash = strrchr(Tcl_DStringValue(&ds), '\\');
	    Tcl_DStringSetLength(&ds, lastSlash - Tcl_DStringValue(&ds));
	    Tcl_DStringAppend(&ds, "../lib/tcl", -1);
	    Tcl_DStringAppend(&ds, TCL_VERSION, -1);
	}
    }

    /*
     * Convert backslashes to slashes.
     */

    TclWinConvertToStdPath(Tcl_DStringValue(&ds));
    Tcl_SetVar(interp, "tcl_library", Tcl_DStringValue(&ds), TCL_GLOBAL_ONLY);

    /*
     * Set up the HOME environment variable from the HOMEDRIVE & HOMEPATH
     * environment variables, if necessary.
     */

    ptr = Tcl_GetVar2(interp, "env", "HOME", TCL_GLOBAL_ONLY);
    if (ptr == NULL) {
	Tcl_DStringSetLength(&ds, 0);
	ptr = Tcl_GetVar2(interp, "env", "HOMEDRIVE", TCL_GLOBAL_ONLY);
	if (ptr != NULL) {
	    Tcl_DStringAppend(&ds, ptr, -1);
	}
	ptr = Tcl_GetVar2(interp, "env", "HOMEPATH", TCL_GLOBAL_ONLY);
	if (ptr != NULL) {
	    Tcl_DStringAppend(&ds, ptr, -1);
	}
	if (Tcl_DStringLength(&ds) > 0) {
	    TclWinConvertToStdPath(Tcl_DStringValue(&ds));
	    Tcl_SetVar2(interp, "env", "HOME", Tcl_DStringValue(&ds),
		    TCL_GLOBAL_ONLY);
	} else {
	    Tcl_SetVar2(interp, "env", "HOME", "c:/", TCL_GLOBAL_ONLY);
	}
    }

    Tcl_DStringFree(&ds);
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
