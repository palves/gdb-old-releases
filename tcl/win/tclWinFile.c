/* 
 * tclWinFile.c --
 *
 *      This file contains temporary wrappers around UNIX file handling
 *      functions. These wrappers map the UNIX functions to Win32 HANDLE-style
 *      files, which can be manipulated through the Win32 console redirection
 *      interfaces.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include <sys/stat.h>
#include "tclInt.h"
#include "tclPort.h"


/*
 *----------------------------------------------------------------------
 *
 * TclCreateTempFile --
 *
 *	This function opens a unique file with the property that it
 *	will be deleted when its file handle is closed.  The temporary
 *	file is created in the system temporary directory.
 *
 * Results:
 *	Returns a valid C file descriptor, or -1 on failure.
 *
 * Side effects:
 *	Creates a new temporary file.
 *
 *----------------------------------------------------------------------
 */

int
TclCreateTempFile()
{
    SECURITY_ATTRIBUTES secAttr;
    char name[MAX_PATH];
    HANDLE handle;

    if (!GetTempPath(MAX_PATH, name)
	    || !GetTempFileName(name, "TCL", 0, name)) {
	return -1;
    }

    secAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    secAttr.lpSecurityDescriptor = NULL;
    secAttr.bInheritHandle = FALSE;

    handle = CreateFile(name, GENERIC_READ | GENERIC_WRITE, 0, &secAttr,
	    TRUNCATE_EXISTING,
	    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
	DWORD error = GetLastError();
	DeleteFile(name);
	TclWinConvertError(error);
	return -1;
    }

    return _open_osfhandle((long) handle, O_TEXT | O_APPEND);
}

/*
 *----------------------------------------------------------------------
 *
 * TclWinGetCwd --
 *
 *	This function is a wrapper for the getcwd call which converts
 *	backslashes path separators into forward slashes.
 *
 * Results:
 *	Same as getcwd().
 *
 * Side effects:
 *	Same as getcwd().  Modifies the buffer returned from getcwd.
 *
 *----------------------------------------------------------------------
 */

#undef getcwd

char *
TclWinGetCwd(buffer, maxlen)
    char *buffer;
    int maxlen;
{
    char *ret, *p;
    ret = getcwd(buffer, maxlen);
    if (ret != NULL) {
	TclWinConvertToStdPath(ret);
    }
    return ret;
}

/*
 *----------------------------------------------------------------------
 *
 * TclWinOpen --
 *
 *	This function wraps the normal system open() to ensure that
 *	files are opened with the _O_NOINHERIT flag set.
 *
 * Results:
 *	Same as open().
 *
 * Side effects:
 *	Same as open().
 *
 *----------------------------------------------------------------------
 */

#undef open

int
TclWinOpen(path, oflag, mode)
    char *path;
    int oflag;
    int mode;
{
    return open(path, oflag, mode|O_NOINHERIT);
}

/*
 *----------------------------------------------------------------------
 *
 * TclWinRead --
 *
 *	This function is a wrapper for the normal read() call.  It
 *	works around a bug that causes read to return an error when
 *	the write side of a pipe is closed.
 *
 * Results:
 *	Number of bytes read on success, 0 on eof, and -1 on failure.
 *
 * Side effects:
 *	Reads data from file indicated by fd.
 *
 *----------------------------------------------------------------------
 */

#undef read

int
TclWinRead(fd, buf, num)
    int fd;
    const void *buf;
    int num;
{
    int count = read(fd, buf, num);
    if ((count < 0) && (errno == EPIPE)) {
	return 0;
    }
    return count;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_FindExecutable --
 *
 *	This procedure computes the absolute path name of the current
 *	application, given its argv[0] value.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The variable tclExecutableName gets filled in with the file
 *	name for the application, if we figured it out.  If we couldn't
 *	figure it out, Tcl_FindExecutable is set to NULL.
 *
 *----------------------------------------------------------------------
 */

void
Tcl_FindExecutable(argv0)
    char *argv0;		/* The value of the application's argv[0]. */
{
    Tcl_DString buffer;
    int length;
    char *p;

    Tcl_DStringInit(&buffer);

    if (tclExecutableName != NULL) {
	ckfree(tclExecutableName);
	tclExecutableName = NULL;
    }

    /*
     * Under Windows we ignore argv0, and return the path for the file used to
     * create this process.
     */

    Tcl_DStringSetLength(&buffer, MAX_PATH+1);
    length = GetModuleFileName(NULL, Tcl_DStringValue(&buffer), MAX_PATH+1);
    if (length > 0) {
	tclExecutableName = (char *) ckalloc((unsigned) (length + 1));
	strcpy(tclExecutableName, Tcl_DStringValue(&buffer));
	TclWinConvertToStdPath(tclExecutableName);
    }
    Tcl_DStringFree(&buffer);
}

/*
 *----------------------------------------------------------------------
 *
 * TclWinConvertToStdPath --
 *
 *	Converts a file path to standard Tcl form.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Modifies the passed in string.
 *
 *----------------------------------------------------------------------
 */

void
TclWinConvertToStdPath(path)
    char *path;
{
    char *ptr;

    /*
     * Convert backslashes to slashes.
     */

    for (ptr = path; *ptr != '\0'; ptr++) {
	if (*ptr == '\\') {
	    *ptr = '/';
	}
    }
}

/*
 *----------------------------------------------------------------------
 *
 * TclWinConvertToNativePath --
 *
 *	Converts a file path to native form.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Modifies the passed in string.
 *
 *----------------------------------------------------------------------
 */

void
TclWinConvertToNativePath(path)
    char *path;
{
    char *ptr;

    /*
     * Convert slashes to backslashes.
     */

    for (ptr = path; *ptr != '\0'; ptr++) {
	if (*ptr == '/') {
	    *ptr = '\\';
	}
    }
}
