/* 
 * tkWin32Dll.c --
 *
 *	This file contains a stub dll entry point.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include <windows.h>


/*
 *----------------------------------------------------------------------
 *
 * DllEntryPoint --
 *
 *	DLL entry point.
 *
 * Results:
 *	TRUE on sucess, FALSE on failure.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

BOOL APIENTRY DllEntryPoint(HINSTANCE hInst, DWORD Reason, LPVOID Reserved)
{
   return(TRUE);
}
