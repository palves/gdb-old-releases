/* 
 * tclUnixPipe.c -- This file implements the UNIX-specific exec pipeline 
 *                  functions.
 *
 * Copyright (c) 1991-1994 The Regents of the University of California.
 * Copyright (c) 1994-1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tclInt.h"
#include "tclPort.h"

/*
 * Declarations for local procedures defined in this file:
 */

static void             RestoreSignals _ANSI_ARGS_((void));

/*
 *----------------------------------------------------------------------
 *
 * RestoreSignals --
 *
 *      This procedure is invoked in a forked child process just before
 *      exec-ing a new program to restore all signals to their default
 *      settings.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Signal settings get changed.
 *
 *----------------------------------------------------------------------
 */
 
static void
RestoreSignals()
{
#ifdef SIGABRT
    signal(SIGABRT, SIG_DFL);
#endif
#ifdef SIGALRM
    signal(SIGALRM, SIG_DFL);
#endif
#ifdef SIGFPE
    signal(SIGFPE, SIG_DFL);
#endif
#ifdef SIGHUP
    signal(SIGHUP, SIG_DFL);
#endif
#ifdef SIGILL
    signal(SIGILL, SIG_DFL);
#endif
#ifdef SIGINT
    signal(SIGINT, SIG_DFL);
#endif
#ifdef SIGPIPE
    signal(SIGPIPE, SIG_DFL);
#endif
#ifdef SIGQUIT
    signal(SIGQUIT, SIG_DFL);
#endif
#ifdef SIGSEGV
    signal(SIGSEGV, SIG_DFL);
#endif
#ifdef SIGTERM
    signal(SIGTERM, SIG_DFL);
#endif
#ifdef SIGUSR1
    signal(SIGUSR1, SIG_DFL);
#endif
#ifdef SIGUSR2
    signal(SIGUSR2, SIG_DFL);
#endif
#ifdef SIGCHLD
    signal(SIGCHLD, SIG_DFL);
#endif
#ifdef SIGCONT
    signal(SIGCONT, SIG_DFL);
#endif
#ifdef SIGTSTP
    signal(SIGTSTP, SIG_DFL);
#endif
#ifdef SIGTTIN
    signal(SIGTTIN, SIG_DFL);
#endif
#ifdef SIGTTOU
    signal(SIGTTOU, SIG_DFL);
#endif
}

/*
 *----------------------------------------------------------------------
 *
 * TclSpawnPipeline --
 *
 *      Given an argc/argv array, instantiate a pipeline of processes
 *      as described by the argv.
 *
 * Results:
 *      The return value is 1 on success, 0 on error
 *
 * Side effects:
 *      Processes and pipes are created.
 *
 *----------------------------------------------------------------------
 */
int TclSpawnPipeline(interp, pidPtr, numPids, argc, argv, errorId, inputId,
	outputId, closeInput, pipeIds, errpipeIds, maxFd, IntIn, FinalOut)
    Tcl_Interp *interp;       /* Interpreter in which to process pipeline. */
    int *pidPtr;              /* Array of pids which are created. */
    int *numPids;             /* Number of pids created. */
    int argc;                 /* Number of entries in argv. */
    char **argv;              /* Array of strings describing commands in
			       * pipeline plus I/O redirection with <,
			       * <<, >, etc. argv[argc] must be NULL. */
    int *errorId;             /* Writable file id for error output from all
			       * commands in the pipeline. -1 means use
			       * stderr */
    int *inputId;             /* If >=0, gives file id to use as input for
			       * first process in pipeline (specified via < or
			       * <@). */
    int *outputId;            /* Writable file id for output from last command
			       * in pipeline (could be file or pipe). -1 means
			       * use stdout. */ 
    int *closeInput;          /* If non-0, then must close inputId when
			       * cleaning up. (0 means the file needs to stay
			       * open for some other reason). */
    int *pipeIds;             /* File ids for pipe that's being created. */
    int *errpipeIds;          /* Used for communication between parent and
			       * child processes.  If child encounters
			       * error during startup it returns error
			       * message via pipe.  If child starts up
			       * OK, it closes pipe without anything in
			       * it. */
    int *maxFd;               /* Highest known file descriptor (used to
			       * close off extraneous file descriptors in
			       * child process). Gratuitous under Windows */
    char *IntIn;              /* File name for initial input (for Win32s). */
    char *FinalOut;           /* File name for final output (for Win32s). */
{
    int firstArg, lastArg;
    int i, pid, count;
    Tcl_DString buffer;
    char *execName;
    char errSpace[200];
    
    for (firstArg = 0; firstArg < argc; firstArg = lastArg+1) {
	int joinThisError;
	int curOutputId;

	joinThisError = 0;
	for (lastArg = firstArg; lastArg < argc; lastArg++) {
	    if (argv[lastArg][0] == '|') {
		if (argv[lastArg][1] == 0) {
		    break;
		}
		if ((argv[lastArg][1] == '&') && (argv[lastArg][2] == 0)) {
		    joinThisError = 1;
		    break;
		}
	    }
	}
	argv[lastArg] = NULL;
	if (lastArg == argc) {
	    curOutputId = *outputId;
	} else {
	    if (TclCreatePipe(pipeIds) == 0) {
		Tcl_AppendResult(interp, "couldn't create pipe: ",
			Tcl_PosixError(interp), (char *) NULL);
		return 0;
	    }
	    curOutputId = pipeIds[1];
	    if (pipeIds[0] > *maxFd) {
		*maxFd = pipeIds[0];
	    }
	    if (pipeIds[1] > *maxFd) {
		*maxFd = pipeIds[1];
	    }
	}

	/*
	 * Create a pipe that the child can use to return error
	 * information if anything goes wrong.  Set the close-on-exec
	 * flag for the write end of the pipe so that it will be
	 * closed automatically if the child succesfully execs the
	 * new subprocess.
	 */

	if (TclCreatePipe(errpipeIds) == 0) {
	    Tcl_AppendResult(interp, "couldn't create pipe: ",
		    Tcl_PosixError(interp), (char *) NULL);
	    return 0;
	}
	if (errpipeIds[0] > *maxFd) {
	    *maxFd = errpipeIds[0];
	}
	if (errpipeIds[1] > *maxFd) {
	    *maxFd = errpipeIds[1];
	}
	if (fcntl(errpipeIds[1], F_SETFD, FD_CLOEXEC) != 0) {
	    Tcl_AppendResult(interp,
		    "couldn't set close on exec for pipe: ",
		    Tcl_PosixError(interp), (char *) NULL);
	    return 0;
	}

	execName = Tcl_TildeSubst(interp, argv[firstArg], &buffer);
	if (execName == NULL) {
	    return 0;
	}
	pid = vfork();
	if (pid == 0) {
	    if (((*inputId != -1) && (dup2(*inputId, 0) == -1))
		    || ((curOutputId != -1) && (dup2(curOutputId, 1) == -1))
		    || (joinThisError && (dup2(1, 2) == -1))
		    || (!joinThisError && (*errorId != -1)
			    && (dup2(*errorId, 2) == -1))) {
		sprintf(errSpace,
			"%dforked process couldn't set up input/output: ",
			errno);
		write(errpipeIds[1], errSpace, (size_t) strlen(errSpace));
		_exit(1);
	    }
	    for (i = 3; i <= *maxFd; i++) {
		if (i != errpipeIds[1]) {
		    close(i);
		}
	    }
	    RestoreSignals();
	    execvp(execName, &argv[firstArg]);
	    sprintf(errSpace, "%dcouldn't execute \"%.150s\": ", errno,
		    argv[firstArg]);
	    write(errpipeIds[1], errSpace, (size_t) strlen(errSpace));
	    _exit(1);
	}
	Tcl_DStringFree(&buffer);
	if (pid == -1) {
	    Tcl_AppendResult(interp, "couldn't fork child process: ",
		    Tcl_PosixError(interp), (char *) NULL);
	    return 0 ;
	}

	/*
	 * Add the child process to the list of those to be reaped.
	 * Note: must do it now, so that the process will be reaped even if
	 * an error occurs during its startup.
	 */
	
	pidPtr[*numPids] = pid;
	(*numPids)++;
	
	/*
	 * Read back from the error pipe to see if the child startup
	 * up OK.  The info in the pipe (if any) consists of a decimal
	 * errno value followed by an error message.
	 */

	close(errpipeIds[1]);
	errpipeIds[1] = -1;
	count = read(errpipeIds[0], errSpace, (size_t) (sizeof(errSpace) - 1));
	if (count > 0) {
	    char *end;
	    errSpace[count] = 0;
	    errno = strtol(errSpace, &end, 10);
	    Tcl_AppendResult(interp, end, Tcl_PosixError(interp),
		    (char *) NULL);
	    return 0;
	}
	close(errpipeIds[0]);
	errpipeIds[0] = -1;

	/*
	 * Close off our copies of file descriptors that were set up for
	 * this child, then set up the input for the next child.
	 */

	if ((*inputId != -1) && *closeInput) {
	    close(*inputId);
	}
	if ((curOutputId != -1) && (curOutputId != *outputId)) {
	    close(curOutputId);
	}
	*inputId = pipeIds[0];
	*closeInput = 1;
	pipeIds[0] = pipeIds[1] = -1;
    }

    return 1;
}


/*
 *----------------------------------------------------------------------
 *
 * TclCreatePipe --
 *
 *      Creates a pipe - simply calls the pipe() function.
 *
 * Results:
 *      Returns 1 on success, 0 on failure. 
 *
 * Side effects:
 *      Creates a pipe.
 *
 *----------------------------------------------------------------------
 */
int
TclCreatePipe(pipeIds)
    int pipeIds[2];        /* File ids for pipe that's being created. */
{
    return (pipe(pipeIds) == 0 ? 1 : 0);
}


/*
 *----------------------------------------------------------------------
 *
 * TclCheckForPipes --
 *
 *      Checks to see if OS has pipe capability. Always returns 1 (UNIX)
 *
 * Results:
 *      Returns 1.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
int
TclCheckForPipes()
{
    return 1;
}
