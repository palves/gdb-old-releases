/* 
 * tclUnixFile.c --
 *
 *      This file contains wrappers around UNIX file handling functions.
 *      These wrappers mask differences between Windows and UNIX.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tclInt.h"
#include "tclPort.h"


/*
 *----------------------------------------------------------------------
 *
 * TclOpen, etc. --
 *
 *	Below are a bunch of procedures that are used by Tcl instead
 *	of system calls.  Each of the procedures executes the
 *	corresponding system call and retries automatically
 *	if the system call was interrupted by a signal.
 *
 * Results:
 *	Whatever the system call would normally return.
 *
 * Side effects:
 *	Whatever the system call would normally do.
 *
 * NOTE:
 *	This should be the last page of this file, since it undefines
 *	the macros that redirect read etc. to the procedures below.
 *
 *----------------------------------------------------------------------
 */

#undef open
int
TclOpen(path, oflag, mode)
    char *path;
    int oflag;
    int mode;
{
    int result;
    while (1) {
	result = open(path, oflag, mode);
	if ((result != -1) || (errno != EINTR)) {
	    return result;
	}
    }
}

#undef read
int
TclRead(fd, buf, numBytes)
    int fd;
    VOID *buf;
    size_t numBytes;
{
    int result;
    while (1) {
	result = read(fd, buf, (size_t) numBytes);
	if ((result != -1) || (errno != EINTR)) {
	    return result;
	}
    }
}

#undef waitpid
#if defined (__Lynx__) || defined (Lynx)
extern pid_t waitpid ();
#else
extern pid_t waitpid _ANSI_ARGS_((pid_t pid, int *stat_loc, int options));
#endif

/*
 * Note:  the #ifdef below is needed to avoid compiler errors on systems
 * that have ANSI compilers and also define pid_t to be short.  The
 * problem is a complex one having to do with argument type promotion.
 */

#ifdef _USING_PROTOTYPES_
int
TclWaitpid _ANSI_ARGS_((pid_t pid, int *statPtr, int options))
#else
int
TclWaitpid(pid, statPtr, options)
    pid_t pid;
    int *statPtr;
    int options;
#endif /* _USING_PROTOTYPES_ */
{
    int result;

    while (1) {
	result = waitpid(pid, statPtr, options);
	if ((result != -1) || (errno != EINTR)) {
	    return result;
	}
    }
}

#undef write
int
TclWrite(fd, buf, numBytes)
    int fd;
    VOID *buf;
    size_t numBytes;
{
    int result;
    while (1) {
	result = write(fd, buf, (size_t) numBytes);
	if ((result != -1) || (errno != EINTR)) {
	    return result;
	}
    }
}

/*
 *----------------------------------------------------------------------
 *
 * TclCreateTempFile --
 *
 *	This function simply creates a file and returns a handle to it.
 *
 * Results:
 *	A handle to a file.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */
int
TclCreateTempFile()
{
    char fileName[L_tmpnam];
    int fd;

    tmpnam(fileName);
    fd = open(fileName, O_RDWR|O_CREAT|O_TRUNC, 0600);
    unlink(fileName);
    return fd;
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
    char *name, *p, *cwd;
    Tcl_DString buffer;
    int length;

    Tcl_DStringInit(&buffer);
    if (tclExecutableName != NULL) {
	ckfree(tclExecutableName);
	tclExecutableName = NULL;
    }

    name = argv0;
    for (p = name; *p != 0; p++) {
	if (*p == '/') {
	    /*
	     * The name contains a slash, so use the name directly
	     * without doing a path search.
	     */

	    goto gotName;
	}
    }

    p = getenv("PATH");
    if (p == NULL) {
	/*
	 * There's no PATH environment variable; use the default that
	 * is used by sh.
	 */

	p = ":/bin:/usr/bin";
    }

    /*
     * Search through all the directories named in the PATH variable
     * to see if argv[0] is in one of them.  If so, use that file
     * name.
     */

    while (*p != 0) {
	while (isspace(UCHAR(*p))) {
	    p++;
	}
	name = p;
	while ((*p != ':') && (*p != 0)) {
	    p++;
	}
	Tcl_DStringSetLength(&buffer, 0);
	if (p != name) {
	    Tcl_DStringAppend(&buffer, name, p-name);
	    if (p[-1] != '/') {
		Tcl_DStringAppend(&buffer, "/", 1);
	    }
	}
	Tcl_DStringAppend(&buffer, argv0, -1);
	if (access(Tcl_DStringValue(&buffer), X_OK) == 0) {
	    name = Tcl_DStringValue(&buffer);
	    goto gotName;
	}
	p++;
    }
    goto done;

    /*
     * If the name starts with "/" then just copy it to tclExecutableName.
     */

    gotName:
    if (name[0] == '/')  {
	tclExecutableName = (char *) ckalloc((unsigned) (strlen(name) + 1));
	strcpy(tclExecutableName, name);
	goto done;
    }

    /*
     * The name is relative to the current working directory.  First
     * strip off a leading "./", if any, then add the full path name of
     * the current working directory.
     */

    if ((name[0] == '.') && (name[1] == '/')) {
	name += 2;
    }
    cwd = TclGetCwd((Tcl_Interp *) NULL);
    if (cwd == NULL) {
	tclExecutableName = NULL;
	goto done;
    }
    length = strlen(cwd);
    tclExecutableName = (char *) ckalloc((unsigned)
	    (length + strlen(name) + 2));
    strcpy(tclExecutableName, cwd);
    tclExecutableName[length] = '/';
    strcpy(tclExecutableName + length + 1, name);

    done:
    Tcl_DStringFree(&buffer);
}
