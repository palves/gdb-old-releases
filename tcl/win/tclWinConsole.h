/*
 * tclWinConsole.h --
 *
 *	Declarations of console class functions.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef _TCLWINCONSOLE
#define _TCLWINCONSOLE

#include <windows.h>
#include "tcl.h"

BOOL RegisterTerminalClass _ANSI_ARGS_((HINSTANCE hInstance));
HWND CreateTerminal _ANSI_ARGS_((HINSTANCE hInstance, Tcl_Interp *interp));

#endif /* _TCLWINCONSOLE */
