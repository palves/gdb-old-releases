/* 
 * tclWinUtil.c --
 *
 *	This file contains a collection of utility procedures that
 *	are present in Tcl's Windows core but not in the generic
 *	core.  For example, they do file manipulation and process
 *	manipulation.
 *
 * Copyright (c) 1994 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tclInt.h"
#include "tclPort.h"


/*
 *----------------------------------------------------------------------
 *
 * TclWinWaitPid --
 *
 *	Does the waitpid system call.
 *
 * Results:
 *	Returns return value of pid it's waiting for.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */
int
TclWinWaitpid(pid, statPtr, options)
    pid_t pid;
    int *statPtr;
    int options;
{
    int flags;
    DWORD ret;

    if (options & WNOHANG) {
	flags = 0;
    } else {
	flags = INFINITE;
    }
    ret = WaitForSingleObject((HANDLE)pid, flags);
    if (ret == WAIT_TIMEOUT) {
	*statPtr = 0;
	return 0;
    } else if (ret != WAIT_FAILED) {
	GetExitCodeProcess((HANDLE)pid, statPtr);
	*statPtr = ((*statPtr << 8) & 0xff00);
	ret = CloseHandle((HANDLE)pid);
	return pid;
    } else {
	errno = ECHILD;
	return -1;
    }
}
