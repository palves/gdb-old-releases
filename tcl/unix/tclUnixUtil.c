/* 
 * tclUnixUtil.c --
 *
 *	This file contains a collection of utility procedures that
 *	are present in the Tcl's UNIX core but not in the generic
 *	core.  For example, they do file manipulation and process
 *	manipulation.
 *
 *	Parts of this file are based on code contributed by Karl
 *	Lehenbauer, Mark Diekhans and Peter da Silva.
 *
 * Copyright (c) 1991-1994 The Regents of the University of California.
 * Copyright (c) 1994 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tclInt.h"
#include "tclPort.h"

/*
 * A linked list of the following structures is used to keep track
 * of child processes that have been detached but haven't exited
 * yet, so we can make sure that they're properly "reaped" (officially
 * waited for) and don't lie around as zombies cluttering the
 * system.
 */

typedef struct Detached {
    int pid;				/* Id of process that's been detached
					 * but isn't known to have exited. */
    struct Detached *nextPtr;		/* Next in list of all detached
					 * processes. */
} Detached;

static Detached *detList = NULL;	/* List of all detached proceses. */

/*
 * Declarations for local procedures defined in this file:
 */

static int		FileForRedirect _ANSI_ARGS_((Tcl_Interp *interp,
	                    char *spec, int atOk, char *arg, int flags,
	                    char *nextArg, int *skipPtr, int *closePtr));
static void		Tcl_DeleteFileTable _ANSI_ARGS_((
			    ClientData clientData, Tcl_Interp *interp));

/*
 *----------------------------------------------------------------------
 *
 * Tcl_EvalFile --
 *
 *	Read in a file and process the entire file as one gigantic
 *	Tcl command.
 *
 * Results:
 *	A standard Tcl result, which is either the result of executing
 *	the file or an error indicating why the file couldn't be read.
 *
 * Side effects:
 *	Depends on the commands in the file.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_EvalFile(interp, fileName)
    Tcl_Interp *interp;		/* Interpreter in which to process file. */
    char *fileName;		/* Name of file to process.  Tilde-substitution
				 * will be performed on this name. */
{
    int fileId, result;
    struct stat statBuf;
    char *cmdBuffer, *oldScriptFile;
    Interp *iPtr = (Interp *) interp;
    Tcl_DString buffer;

    Tcl_ResetResult(interp);
    oldScriptFile = iPtr->scriptFile;
    iPtr->scriptFile = fileName;
    fileName = Tcl_TildeSubst(interp, fileName, &buffer);
    if (fileName == NULL) {
	goto error;
    }

    /*
     * If Tcl_TildeSubst didn't already copy the file name, do it
     * here.  This way we don't depend on fileName staying constant
     * throughout the execution of the script (e.g., what if it happens
     * to point to a Tcl variable that the script could change?).
     */

    if (fileName != Tcl_DStringValue(&buffer)) {
	Tcl_DStringSetLength(&buffer, 0);
	Tcl_DStringAppend(&buffer, fileName, -1);
	fileName = Tcl_DStringValue(&buffer);
    }
    fileId = open(fileName, O_RDONLY, 0);
    if (fileId < 0) {
	Tcl_AppendResult(interp, "couldn't read file \"", fileName,
		"\": ", Tcl_PosixError(interp), (char *) NULL);
	goto error;
    }
    if (fstat(fileId, &statBuf) == -1) {
	Tcl_AppendResult(interp, "couldn't stat file \"", fileName,
		"\": ", Tcl_PosixError(interp), (char *) NULL);
	close(fileId);
	goto error;
    }
    cmdBuffer = (char *) ckalloc((unsigned) statBuf.st_size+1);
    result = read(fileId, cmdBuffer, (size_t) statBuf.st_size);
    if (result < 0) {
	Tcl_AppendResult(interp, "error in reading file \"", fileName,
		"\": ", Tcl_PosixError(interp), (char *) NULL);
	close(fileId);
	ckfree(cmdBuffer);
	goto error;
    }
    statBuf.st_size = result;
    if (close(fileId) != 0) {
	Tcl_AppendResult(interp, "error closing file \"", fileName,
		"\": ", Tcl_PosixError(interp), (char *) NULL);
	ckfree(cmdBuffer);
	goto error;
    }
    cmdBuffer[statBuf.st_size] = 0;
#ifdef MAC_TCL
    /*
     * Convert all carriage returns to newlines
     * This will not work for DOS files
     */
	{
	    int i;
	    
	    for (i=0; i<statBuf.st_size; i++) {
		if (cmdBuffer[i] == '\r') {
		    cmdBuffer[i] = '\n';
		}
	    }
	}
#endif
    result = Tcl_Eval(interp, cmdBuffer);
    if (result == TCL_RETURN) {
	result = TclUpdateReturnInfo(iPtr);
    } else if (result == TCL_ERROR) {
	char msg[200];

	/*
	 * Record information telling where the error occurred.
	 */

	sprintf(msg, "\n    (file \"%.150s\" line %d)", fileName,
		interp->errorLine);
	Tcl_AddErrorInfo(interp, msg);
    }
    ckfree(cmdBuffer);
    iPtr->scriptFile = oldScriptFile;
    Tcl_DStringFree(&buffer);
    return result;

    error:
    iPtr->scriptFile = oldScriptFile;
    Tcl_DStringFree(&buffer);
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_DetachPids --
 *
 *	This procedure is called to indicate that one or more child
 *	processes have been placed in background and will never be
 *	waited for;  they should eventually be reaped by
 *	Tcl_ReapDetachedProcs.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void
Tcl_DetachPids(numPids, pidPtr)
    int numPids;		/* Number of pids to detach:  gives size
				 * of array pointed to by pidPtr. */
    int *pidPtr;		/* Array of pids to detach. */
{
    register Detached *detPtr;
    int i;

    for (i = 0; i < numPids; i++) {
	detPtr = (Detached *) ckalloc(sizeof(Detached));
	detPtr->pid = pidPtr[i];
	detPtr->nextPtr = detList;
	detList = detPtr;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_ReapDetachedProcs --
 *
 *	This procedure checks to see if any detached processes have
 *	exited and, if so, it "reaps" them by officially waiting on
 *	them.  It should be called "occasionally" to make sure that
 *	all detached processes are eventually reaped.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Processes are waited on, so that they can be reaped by the
 *	system.
 *
 *----------------------------------------------------------------------
 */

void
Tcl_ReapDetachedProcs()
{
    register Detached *detPtr;
    Detached *nextPtr, *prevPtr;
    int status, result;

    for (detPtr = detList, prevPtr = NULL; detPtr != NULL; ) {
	result = waitpid(detPtr->pid, &status, WNOHANG);
	if ((result == 0) || ((result == -1) && (errno != ECHILD))) {
	    prevPtr = detPtr;
	    detPtr = detPtr->nextPtr;
	    continue;
	}
	nextPtr = detPtr->nextPtr;
	if (prevPtr == NULL) {
	    detList = detPtr->nextPtr;
	} else {
	    prevPtr->nextPtr = detPtr->nextPtr;
	}
	ckfree((char *) detPtr);
	detPtr = nextPtr;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_CreatePipeline --
 *
 *	Given an argc/argv array, instantiate a pipeline of processes
 *	as described by the argv.
 *
 * Results:
 *	The return value is a count of the number of new processes
 *	created, or -1 if an error occurred while creating the pipeline.
 *	*pidArrayPtr is filled in with the address of a dynamically
 *	allocated array giving the ids of all of the processes.  It
 *	is up to the caller to free this array when it isn't needed
 *	anymore.  If inPipePtr is non-NULL, *inPipePtr is filled in
 *	with the file id for the input pipe for the pipeline (if any):
 *	the caller must eventually close this file.  If outPipePtr
 *	isn't NULL, then *outPipePtr is filled in with the file id
 *	for the output pipe from the pipeline:  the caller must close
 *	this file.  If errFilePtr isn't NULL, then *errFilePtr is filled
 *	with a file id that may be used to read error output after the
 *	pipeline completes.
 *
 * Side effects:
 *	Processes and pipes are created.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_CreatePipeline(interp, argc, argv, pidArrayPtr, inPipePtr,
	outPipePtr, errFilePtr)
    Tcl_Interp *interp;		/* Interpreter to use for error reporting. */
    int argc;			/* Number of entries in argv. */
    char **argv;		/* Array of strings describing commands in
				 * pipeline plus I/O redirection with <,
				 * <<,  >, etc.  Argv[argc] must be NULL. */
    int **pidArrayPtr;		/* Word at *pidArrayPtr gets filled in with
				 * address of array of pids for processes
				 * in pipeline (first pid is first process
				 * in pipeline). */
    int *inPipePtr;		/* If non-NULL, input to the pipeline comes
				 * from a pipe (unless overridden by
				 * redirection in the command).  The file
				 * id with which to write to this pipe is
				 * stored at *inPipePtr.  -1 means command
				 * specified its own input source. */
    int *outPipePtr;		/* If non-NULL, output to the pipeline goes
				 * to a pipe, unless overriden by redirection
				 * in the command.  The file id with which to
				 * read frome this pipe is stored at
				 * *outPipePtr.  -1 means command specified
				 * its own output sink. */
    int *errFilePtr;		/* If non-NULL, all stderr output from the
				 * pipeline will go to a temporary file
				 * created here, and a descriptor to read
				 * the file will be left at *errFilePtr.
				 * The file will be removed already, so
				 * closing this descriptor will be the end
				 * of the file.  If this is NULL, then
				 * all stderr output goes to our stderr.
				 * If the pipeline specifies redirection
				 * then the file will still be created
				 * but it will never get any data. */
{
#if defined( MAC_TCL )
    return -1;
#else /* !MAC_TCL */
    int *pidPtr = NULL;		/* Points to malloc-ed array holding all
				 * the pids of child processes. */
    int numPids = 0;		/* Actual number of processes that exist
				 * at *pidPtr right now. */
    int cmdCount;		/* Count of number of distinct commands
				 * found in argc/argv. */
    char *input = NULL;		/* If non-null, then this points to a
				 * string containing input data (specified
				 * via <<) to be piped to the first process
				 * in the pipeline. */
    int inputId = -1;		/* If >= 0, gives file id to use as input for
				 * first process in pipeline (specified via
				 * < or <@). */
    int closeInput = 0;		/* If non-zero, then must close inputId
				 * when cleaning up (zero means the file needs
				 * to stay open for some other reason). */
    int outputId = -1;		/* Writable file id for output from last
				 * command in pipeline (could be file or pipe).
				 * -1 means use stdout. */
    int closeOutput = 0;	/* Non-zero means must close outputId when
				 * cleaning up (similar to closeInput). */
    int errorId = -1;		/* Writable file id for error output from
				 * all commands in pipeline. -1 means use
				 * stderr. */
    int closeError = 0;		/* Non-zero means must close errorId when
				 * cleaning up. */
    int pipeIds[2];		/* File ids for pipe that's being created. */
    int skip;			/* Number of arguments to skip (because they
				 * specify redirection). */
    int maxFd;			/* Highest known file descriptor (used to
				 * close off extraneous file descriptors in
				 * child process). */
    int errPipeIds[2];		/* Used for communication between parent and
				 * child processes.  If child encounters
				 * error during startup it returns error
				 * message via pipe.  If child starts up
				 * OK, it closes pipe without anything in
				 * it. */
    int lastBar;
    int i, j;
    char *p;
    int hasPipes = TclCheckForPipes();
    char finalOut[L_tmpnam];
    char intIn[L_tmpnam];

    finalOut[0]  = '\0';
    intIn[0] = '\0';
    
    if (inPipePtr != NULL) {
	*inPipePtr = -1;
    }
    if (outPipePtr != NULL) {
	*outPipePtr = -1;
    }
    if (errFilePtr != NULL) {
	*errFilePtr = -1;
    }
    pipeIds[0] = pipeIds[1] = -1;
    errPipeIds[0] = errPipeIds[1] = -1;

    /*
     * First, scan through all the arguments to figure out the structure
     * of the pipeline.  Process all of the input and output redirection
     * arguments and remove them from the argument list in the pipeline.
     * Count the number of distinct processes (it's the number of "|"
     * arguments plus one) but don't remove the "|" arguments.
     */

    cmdCount = 1;
    lastBar = -1;
    for (i = 0; i < argc; i++) {
	if ((argv[i][0] == '|') && (((argv[i][1] == 0))
		|| ((argv[i][1] == '&') && (argv[i][2] == 0)))) {
	    if ((i == (lastBar+1)) || (i == (argc-1))) {
		interp->result = "illegal use of | or |& in command";
		return -1;
	    }
	    lastBar = i;
	    cmdCount++;
	    continue;
	} else if (argv[i][0] == '<') {
	    if ((inputId >= 0) && closeInput) {
		close(inputId);
	    }
	    inputId = -1;
	    skip = 1;
	    if (argv[i][1] == '<') {
		input = argv[i]+2;
		if (*input == 0) {
		    input = argv[i+1];
		    if (input == 0) {
			Tcl_AppendResult(interp, "can't specify \"", argv[i],
				"\" as last word in command", (char *) NULL);
			goto error;
		    }
		    skip = 2;
		}
	    } else {
		input = 0;
		inputId = FileForRedirect(interp, argv[i]+1, 1, argv[i],
			O_RDONLY, argv[i+1], &skip, &closeInput);
		if (inputId < 0 && hasPipes) {
		    goto error;
		}

		/* When Win32s dies out, this code can be removed */
		if(!hasPipes) {
		    strcpy(intIn, skip == 1 ? argv[i]+1 : argv[i+skip]);
		    }
	    }
	} else if (argv[i][0] == '>') {
	    int append, useForStdErr, useForStdOut, mustClose, fd, atOk, flags;

	    skip = atOk = 1;
	    append = useForStdErr = 0;
	    useForStdOut = 1;
	    if (argv[i][1] == '>') {
		p = argv[i] + 2;
		append = 1;
		atOk = 0;
		flags = O_WRONLY|O_CREAT;
	    } else {
		p = argv[i] + 1;
		flags = O_WRONLY|O_CREAT|O_TRUNC;
	    }
	    if (*p == '&') {
		useForStdErr = 1;
		p++;
	    }
	    fd = FileForRedirect(interp, p, atOk, argv[i], flags, argv[i+1],
		    &skip, &mustClose);
	    if (hasPipes && fd < 0) {
		goto error;
	    }

	    /* When Win32s dies out, this code can be removed */
	    if(!hasPipes) {
		strcpy(finalOut, skip == 1 ? p : argv[i + 1]);
		}

	    if (hasPipes && append) {
		lseek(fd, 0L, 2);
	    }

	    /*
	     * Got the file descriptor.  Now use it for standard output,
	     * standard error, or both, depending on the redirection.
	     */

	    if (useForStdOut) {
		if (hasPipes && (outputId > 0) && closeOutput) {
		    close(outputId);
		}
		outputId = fd;
		closeOutput = mustClose;
	    }
	    if (useForStdErr) {
		if (hasPipes && (errorId > 0) && closeError) {
		    close(errorId);
		}
		errorId = fd;
		closeError = (useForStdOut) ? 0 : mustClose;
	    }
	} else if ((argv[i][0] == '2') && (argv[i][1] == '>')) {
	    int append, atOk, flags;

	    if ((errorId > 0) && closeError) {
		close(errorId);
	    }
	    skip = 1;
	    p = argv[i] + 2;
	    if (*p == '>') {
		p++;
		append = 1;
		atOk = 0;
		flags = O_WRONLY|O_CREAT;
	    } else {
		append = 0;
		atOk = 1;
		flags = O_WRONLY|O_CREAT|O_TRUNC;
	    }
	    errorId = FileForRedirect(interp, p, atOk, argv[i], flags,
		    argv[i+1], &skip, &closeError);
	    if (hasPipes && errorId < 0) {
		goto error;
	    }
	    if (hasPipes && append) {
		lseek(errorId, 0L, 2);
	    }
	} else {
	    continue;
	}
	for (j = i+skip; j < argc; j++) {
	    argv[j-skip] = argv[j];
	}
	argc -= skip;
	i -= 1;			/* Process next arg from same position. */
    }
    if (argc == 0) {
	interp->result =  "didn't specify command to execute";
	return -1;
    }

    if((hasPipes && inputId < 0) || (!hasPipes && intIn[0] == '\0')) {
	if (input != NULL) {
	    int length;
	    
	    /*
	     * The input for the first process is immediate data coming from
	     * Tcl.  Create a temporary file for it and put the data into the
	     * file.
	     */
	    
	    inputId = TclCreateTempFile();
	    closeInput = 1;
	    if (inputId < 0) {
		Tcl_AppendResult(interp,
			"couldn't create input file for command: ",
			Tcl_PosixError(interp), (char *) NULL);
		goto error;
	    }
	    length = strlen(input);
	    if (write(inputId, input, (size_t) length) != length) {
		Tcl_AppendResult(interp,
			"couldn't write file input for command: ",
			Tcl_PosixError(interp), (char *) NULL);
		goto error;
	    }

	    if (lseek(inputId, 0L, 0) == -1) {
		Tcl_AppendResult(interp,
			"couldn't reset input file for command: ",
			Tcl_PosixError(interp), (char *) NULL);
		goto error;
	    }
	} else if (inPipePtr != NULL) {
	    /*
	     * The input for the first process in the pipeline is to
	     * come from a pipe that can be written from this end.
	     */

	    if (!hasPipes || TclCreatePipe(pipeIds) == 0) {
		Tcl_AppendResult(interp,
			"couldn't create input pipe for command: ",
			Tcl_PosixError(interp), (char *) NULL);
		goto error;
	    }
	    inputId = pipeIds[0];
	    closeInput = 1;
	    *inPipePtr = pipeIds[1];
	    pipeIds[0] = pipeIds[1] = -1;
	}
    }

    /*
     * Set up a pipe to receive output from the pipeline, if no other
     * output sink has been specified.
     */

    if ((outputId < 0) && (outPipePtr != NULL)) {
	if (!hasPipes) {
	    tmpnam(finalOut);
	} else {
	    if(TclCreatePipe(pipeIds) == 0) {
		Tcl_AppendResult(interp,
			"couldn't create output pipe: ",
			Tcl_PosixError(interp), (char *) NULL);
		goto error;
	    }
	    outputId = pipeIds[1];
	    closeOutput = 1;
	    *outPipePtr = pipeIds[0];
	    pipeIds[0] = pipeIds[1] = -1;
	}
    }

    /*
     * Set up the standard error output sink for the pipeline, if
     * requested.  Use a temporary file which is opened, then deleted.
     * Could potentially just use pipe, but if it filled up it could
     * cause the pipeline to deadlock:  we'd be waiting for processes
     * to complete before reading stderr, and processes couldn't complete
     * because stderr was backed up.
     */

    if ((errFilePtr != NULL) && (errorId < 0)) {
	*errFilePtr = TclCreateTempFile();
	if (*errFilePtr < 0) {
	    errFileError:
	    Tcl_AppendResult(interp,
		    "couldn't create error file for command: ",
		    Tcl_PosixError(interp), (char *) NULL);
	    goto error;
	}
	if (errorId < 0) {
	    errorId = dup(*errFilePtr);

	    if (errorId < 0) {
		goto errFileError;
	    }
	    closeError = 1;
	}
    }
	
    /*
     * Find the largest file descriptor used so far, so that we can
     * clean up all the extraneous file descriptors in the child
     * processes we create.
     */

    maxFd = inputId;
    if (outputId > maxFd) {
	maxFd = outputId;
    }
    if (errorId > maxFd) {
	maxFd = errorId;
    }
    if ((inPipePtr != NULL) && (*inPipePtr > maxFd)) {
	maxFd = *inPipePtr;
    }
    if ((outPipePtr != NULL) && (*outPipePtr > maxFd)) {
	maxFd = *outPipePtr;
    }
    if ((errFilePtr != NULL) && (*errFilePtr > maxFd)) {
	maxFd = *errFilePtr;
    }
	
    /*
     * Scan through the argc array, forking off a process for each
     * group of arguments between "|" arguments.
     */

    pidPtr = (int *) ckalloc((unsigned) (cmdCount * sizeof(int)));
    Tcl_ReapDetachedProcs();

    if(TclSpawnPipeline(interp, pidPtr, &numPids, argc, argv, &errorId,
	    &inputId, &outputId, &closeInput, 
	    pipeIds, errPipeIds, &maxFd, intIn, finalOut) == 0) {
	goto error;
    }
    *pidArrayPtr = pidPtr;

    /*
     * All done.  Cleanup open files lying around and then return.
     */

cleanup:
    if ((inputId != -1) && closeInput) {
	close(inputId);
    }
    if ((outputId != -1) && closeOutput) {
	close(outputId);
    }
    if ((errorId != -1) && closeError) {
	close(errorId);
    }
    return numPids;

    /*
     * An error occurred.  There could have been extra files open, such
     * as pipes between children.  Clean them all up.  Detach any child
     * processes that have been created.
     */

    error:
    if ((inPipePtr != NULL) && (*inPipePtr != -1)) {
	close(*inPipePtr);
	*inPipePtr = -1;
    }
    if ((outPipePtr != NULL) && (*outPipePtr != -1)) {
	close(*outPipePtr);
	*outPipePtr = -1;
    }
    if ((errFilePtr != NULL) && (*errFilePtr != -1)) {
	close(*errFilePtr);
	*errFilePtr = -1;
    }
    if (pipeIds[0] != -1) {
	close(pipeIds[0]);
    }
    if (pipeIds[1] != -1) {
	close(pipeIds[1]);
    }
    if (errPipeIds[0] != -1) {
	close(errPipeIds[0]);
    }
    if (errPipeIds[1] != -1) {
	close(errPipeIds[1]);
    }
    if (pidPtr != NULL) {
	for (i = 0; i < numPids; i++) {
	    if (pidPtr[i] != -1) {
		Tcl_DetachPids(1, &pidPtr[i]);
	    }
	}
	ckfree((char *) pidPtr);
    }
    numPids = -1;
    goto cleanup;
#endif /* !MAC_TCL */
}

/*
 *----------------------------------------------------------------------
 *
 * FileForRedirect --
 *
 *	This procedure does much of the work of parsing redirection
 *	operators.  It handles "@" if specified and allowed, and a file
 *	name, and opens the file if necessary.
 *
 * Results:
 *	The return value is the descriptor number for the file.  If an
 *	error occurs then -1 is returned and an error message is left
 *	in interp->result.  Several arguments are side-effected; see
 *	the argument list below for details.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
FileForRedirect(interp, spec, atOk, arg, flags, nextArg, skipPtr, closePtr)
    Tcl_Interp *interp;			/* Intepreter to use for error
					 * reporting. */
    register char *spec;			/* Points to character just after
					 * redirection character. */
    int atOk;				/* Non-zero means '@' notation is
					 * OK, zero means it isn't. */
    char *arg;				/* Pointer to entire argument
					 * containing spec:  used for error
					 * reporting. */
    int flags;				/* Flags to use for opening file. */
    char *nextArg;			/* Next argument in argc/argv
					 * array, if needed for file name.
					 * May be NULL. */
    int *skipPtr;			/* This value is incremented if
					 * nextArg is used for redirection
					 * spec. */
    int *closePtr;			/* This value is set to 1 if the file
					 * that's returned must be closed, 0
					 * if it was specified with "@" so
					 * it must be left open. */
{
    int writing = (flags & O_WRONLY);
    FILE *f;
    int fd;

    if (atOk && (*spec == '@')) {
	spec++;
	if (*spec == 0) {
	    spec = nextArg;
	    if (spec == NULL) {
		goto badLastArg;
	    }
	    *skipPtr += 1;
	}
	if (Tcl_GetOpenFile(interp, spec, writing, 1, &f) != TCL_OK) {
	    return -1;
	}
	*closePtr = 0;
	fd = fileno(f);
	if (writing) {
	    /*
	     * Be sure to flush output to the file, so that anything
	     * written by the child appears after stuff we've already
	     * written.
	     */

	    fflush(f);
	}
    } else {
	if (*spec == 0) {
	    spec = nextArg;
	    if (spec == NULL) {
		goto badLastArg;
	    }
	    *skipPtr += 1;
	}
	fd = open(spec, flags, 0666);
	if (fd < 0) {
	    Tcl_AppendResult(interp, "couldn't ",
		    (writing) ? "write" : "read", " file \"", spec, "\": ",
		    Tcl_PosixError(interp), (char *) NULL);
	    return -1;
	}
	*closePtr = 1;
    }
    return fd;

    badLastArg:
    Tcl_AppendResult(interp, "can't specify \"", arg,
	    "\" as last word in command", (char *) NULL);
    return -1;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_PosixError --
 *
 *	This procedure is typically called after UNIX kernel calls
 *	return errors.  It stores machine-readable information about
 *	the error in $errorCode returns an information string for
 *	the caller's use.
 *
 * Results:
 *	The return value is a human-readable string describing the
 *	error.
 *
 * Side effects:
 *	The global variable $errorCode is reset.
 *
 *----------------------------------------------------------------------
 */

char *
Tcl_PosixError(interp)
    Tcl_Interp *interp;		/* Interpreter whose $errorCode variable
				 * is to be changed. */
{
    char *id, *msg;

    msg = Tcl_ErrnoMsg(errno);
    id = Tcl_ErrnoId();
    Tcl_SetErrorCode(interp, "POSIX", id, msg, (char *) NULL);
    return msg;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_DeleteFileTable --
 *
 *	Helper function called when an interpreter is deleted to delete
 *	the file table for that interpreter. This will close any files
 *	whose refcount has reached zero, and decrement the refcount for
 *	all other files. This function is called from the cleanup of
 *	the assocData for the interpreter being deleted.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Deletes a file table and may close files.
 *
 *----------------------------------------------------------------------
 */

static void
Tcl_DeleteFileTable(clientData, interp)
    ClientData	clientData;		/* The hash table for files. */
    Tcl_Interp	*interp;		/* The interpreter being deleted. */
{
    Tcl_HashTable *hTablePtr;
    Tcl_HashEntry *hPtr;
    Tcl_HashSearch hSearch;
    TclOpenFile *oFilePtr;
    int i;

    hTablePtr = (Tcl_HashTable *) clientData;

    if (hTablePtr == (Tcl_HashTable *) NULL) {
        return;
    }
    for (hPtr = Tcl_FirstHashEntry(hTablePtr, &hSearch);
         hPtr != (Tcl_HashEntry *) NULL;
	/* Use Tcl_FirstHashEntry again because we've modified the
	   hash table.  */
        hPtr = Tcl_FirstHashEntry(hTablePtr, &hSearch)) {
        oFilePtr = (TclOpenFile *) Tcl_GetHashValue(hPtr);
        Tcl_DeleteHashEntry(hPtr);
        if (oFilePtr == (TclOpenFile *) NULL) {
            continue;
        }
        oFilePtr->refCount--;
        if (oFilePtr->refCount <= 0) {
            if (oFilePtr->f != NULL) {
                fclose(oFilePtr->f);
            }
            if (oFilePtr->f2 != NULL) {
                fclose(oFilePtr->f2);
            }
            oFilePtr->refCount = 0;
            if ((oFilePtr->numPids > 0) && (oFilePtr->pidPtr != NULL)) {
                for (i = 0; i < oFilePtr->numPids; i++) {
                    if (oFilePtr->pidPtr[i] != -1) {
                        Tcl_DetachPids(1, &oFilePtr->pidPtr[i]);
                    }
                }
                ckfree((char *) oFilePtr->pidPtr);
            }
            ckfree((char *) oFilePtr);
        } 
    }
    Tcl_DeleteHashTable(hTablePtr);
    ckfree((char *) hTablePtr);
}

/*
 *----------------------------------------------------------------------
 *
 * TclMakeFileTable --
 *
 *	Create a hash table for files. If noStdio is set do not install
 *	stdin, stdout and stderr, otherwise do install them.
 *
 * Results:
 *	Standard Tcl result.
 *
 * Side effects:
 *	The file table for the interpreter will be created if it doesn't
 *	exist and entries will be added for stdin, stdout, and stderr if
 *	noStdio is zero.
 *
 *----------------------------------------------------------------------
 */
	/* ARGSUSED */
int
TclMakeFileTable(interp, noStdio)
    Tcl_Interp *interp;		/* Interpreter whose table of files is
				 * to be manipulated. */
    int noStdio;		/* If non-zero do not install stdin, stdout
                                 * and stderr files. */
{
    Tcl_HashTable *hTablePtr;
    Tcl_HashEntry *hPtr;
    int new;
    
    hTablePtr = (Tcl_HashTable *) Tcl_GetAssocData(interp, "tclFileTable",
            NULL);
    if (hTablePtr == (Tcl_HashTable *) NULL) {
	TclOpenFile *oFilePtr;

        hTablePtr = (Tcl_HashTable *) ckalloc(sizeof(Tcl_HashTable));
        Tcl_InitHashTable(hTablePtr, TCL_STRING_KEYS);

        Tcl_SetAssocData(interp, "tclFileTable", Tcl_DeleteFileTable,
                (ClientData) hTablePtr);

        if (noStdio != 0) {
            return TCL_OK;
        }
        
	oFilePtr = (TclOpenFile *) ckalloc(sizeof(TclOpenFile));
	oFilePtr->f = stdin;
	oFilePtr->f2 = NULL;
	oFilePtr->permissions = TCL_FILE_READABLE;
	oFilePtr->numPids = 0;
	oFilePtr->pidPtr = NULL;
	oFilePtr->errorId = -1;
        oFilePtr->refCount = 1;
        hPtr = Tcl_CreateHashEntry(hTablePtr, "stdin", &new);
        Tcl_SetHashValue(hPtr, oFilePtr);

	oFilePtr = (TclOpenFile *) ckalloc(sizeof(TclOpenFile));
	oFilePtr->f = stdout;
	oFilePtr->f2 = NULL;
	oFilePtr->permissions = TCL_FILE_WRITABLE;
	oFilePtr->numPids = 0;
	oFilePtr->pidPtr = NULL;
	oFilePtr->errorId = -1;
        oFilePtr->refCount = 1;
        hPtr = Tcl_CreateHashEntry(hTablePtr, "stdout", &new);
        Tcl_SetHashValue(hPtr, oFilePtr);

	oFilePtr = (TclOpenFile *) ckalloc(sizeof(TclOpenFile));
	oFilePtr->f = stderr;
	oFilePtr->f2 = NULL;
	oFilePtr->permissions = TCL_FILE_WRITABLE;
	oFilePtr->numPids = 0;
	oFilePtr->pidPtr = NULL;
	oFilePtr->errorId = -1;
        oFilePtr->refCount = 1;
        hPtr = Tcl_CreateHashEntry(hTablePtr, "stderr", &new);
        Tcl_SetHashValue(hPtr, oFilePtr);
    }

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_EnterFile --
 *
 *	This procedure is used to enter an already-open file into the
 *	file table for an interpreter so that the file can be read
 *	and written with Tcl commands.
 *
 * Results:
 *	There is no return value, but interp->result is set to
 *	hold Tcl's id for the open file, such as "file4".
 *
 * Side effects:
 *	"File" is added to the files accessible from interp.
 *
 *----------------------------------------------------------------------
 */

void
Tcl_EnterFile(interp, file, permissions)
    Tcl_Interp *interp;		/* Interpreter in which to make file
				 * available. */
    FILE *file;			/* File to make available in interp. */
    int permissions;		/* Ops that may be done on file:  OR-ed
				 * combinination of TCL_FILE_READABLE and
				 * TCL_FILE_WRITABLE. */
{
    int fd, new, i;
    register TclOpenFile *oFilePtr;
    Tcl_HashTable *hTablePtr;
    Tcl_HashEntry *hPtr;

    (void) TclMakeFileTable(interp, 0);
    hTablePtr = (Tcl_HashTable *) Tcl_GetAssocData(interp, "tclFileTable",
            NULL);
    fd = fileno(file);
    if (fd <= 2) {
	if (fd == 0) {
	    interp->result = "stdin";
	} else if (fd == 1) {
	    interp->result = "stdout";
	} else {
	    interp->result = "stderr";
	}
    } else {
	sprintf(interp->result, "file%d", fd);
    }
    hPtr = Tcl_CreateHashEntry(hTablePtr, interp->result, &new);
    if (new == 0) {
        oFilePtr = (TclOpenFile *) Tcl_GetHashValue(hPtr);
        oFilePtr->refCount--;
        if (oFilePtr->refCount <= 0) {
            if (oFilePtr->f != NULL) {
                fclose(oFilePtr->f);
            }
            if (oFilePtr->f2 != NULL) {
                fclose(oFilePtr->f2);
            }
            oFilePtr->refCount = 0;
            if ((oFilePtr->numPids > 0) && (oFilePtr->pidPtr != NULL)) {
                for (i = 0; i < oFilePtr->numPids; i++) {
                    if (oFilePtr->pidPtr[i] != -1) {
                        Tcl_DetachPids(1, &oFilePtr->pidPtr[i]);
                    }
                }
                ckfree((char *) oFilePtr->pidPtr);
            }
            ckfree((char *) oFilePtr);
        } 
    }
    oFilePtr = (TclOpenFile *) ckalloc((unsigned) sizeof(TclOpenFile));
    oFilePtr->f = file;
    oFilePtr->f2 = NULL;
    oFilePtr->permissions = permissions;
    oFilePtr->numPids = 0;
    oFilePtr->pidPtr = NULL;
    oFilePtr->errorId = -1;
    oFilePtr->refCount = 1;
    Tcl_SetHashValue(hPtr, oFilePtr);
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_GetOpenFile --
 *
 *	Given a string identifier for an open file, find the corresponding
 *	open file structure, if there is one.
 *
 * Results:
 *	A standard Tcl return value.  If the open file is successfully
 *	located and meets any usage check requested by checkUsage, TCL_OK
 *	is returned and *filePtr is modified to hold a pointer to its
 *	FILE structure.  If an error occurs then TCL_ERROR is returned
 *	and interp->result contains an error message.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_GetOpenFile(interp, string, forWriting, checkUsage, filePtr)
    Tcl_Interp *interp;		/* Interpreter in which to find file. */
    char *string;		/* String that identifies file. */
    int forWriting;		/* 1 means the file is going to be used
				 * for writing, 0 means for reading. */
    int checkUsage;		/* 1 means verify that the file was opened
				 * in a mode that allows the access specified
				 * by "forWriting". */
    FILE **filePtr;		/* Store pointer to FILE structure here. */
{
    TclOpenFile *oFilePtr;
    int fd = 0;			/* Initial value needed only to stop compiler
				 * warnings. */
    Tcl_HashTable *hTablePtr;
    Tcl_HashEntry *hPtr;

    if (strncmp(string, "file", 4) == 0) {
	char *end;

	fd = strtoul(string+4, &end, 10);
	if ((end == string+4) || (*end != 0)) {
	    goto badId;
	}
        switch (fd) {
            case 0:
                string = "stdin";
                break;
            case 1:
                string = "stdout";
                break;
            case 2:
                string = "stderr";
                break;
            default:
                break;
        }
    } else if ((strcmp(string, "stdin") != 0)
	    && (strcmp(string, "stdout") != 0)
	    && (strcmp(string, "stderr") != 0)) {
	goto badId;
    }

    (void) TclMakeFileTable(interp, 0);
    hTablePtr = (Tcl_HashTable *) Tcl_GetAssocData(interp, "tclFileTable",
            NULL);
    hPtr = Tcl_FindHashEntry(hTablePtr, string);
    if (hPtr == (Tcl_HashEntry *) NULL) {
        Tcl_AppendResult(interp, "file \"", string, "\" isn't open",
                (char *) NULL);
        return TCL_ERROR;
    }
    oFilePtr = (TclOpenFile *) Tcl_GetHashValue(hPtr);
    if (oFilePtr == NULL) {
        Tcl_AppendResult(interp, "file \"", string, "\" isn't open",
                (char *) NULL);
        return TCL_ERROR;
    }
    if (forWriting) {
	if (checkUsage && !(oFilePtr->permissions & TCL_FILE_WRITABLE)) {
	    Tcl_AppendResult(interp, "file \"", string,
		    "\" wasn't opened for writing", (char *) NULL);
	    return TCL_ERROR;
	}
	if (oFilePtr->f2 != NULL) {
	    *filePtr = oFilePtr->f2;
	} else {
	    *filePtr = oFilePtr->f;
	}
    } else {
	if (checkUsage && !(oFilePtr->permissions & TCL_FILE_READABLE)) {
	    Tcl_AppendResult(interp, "file \"", string,
		    "\" wasn't opened for reading", (char *) NULL);
	    return TCL_ERROR;
	}
	*filePtr = oFilePtr->f;
    }
    return TCL_OK;

    badId:
    Tcl_AppendResult(interp, "bad file identifier \"", string,
	    "\"", (char *) NULL);
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_FilePermissions --
 *
 *	Given a FILE * pointer, return the read/write permissions
 *	associated with the open file.
 *
 * Results:
 *	If file is currently open, the return value is an OR-ed
 *	combination of TCL_FILE_READABLE and TCL_FILE_WRITABLE,
 *	which indicates the operations permitted on the open file.
 *	If the file isn't open then the return value is -1.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_FilePermissions(interp, file)
    Tcl_Interp *interp;		/* Interp which opened this file. */
    FILE *file;			/* File for which permissions are wanted. */
{
    register TclOpenFile *oFilePtr;
    int fd;
    Tcl_HashTable *hTablePtr;
    Tcl_HashEntry *hPtr;
    char *bufPtr;
    char buffer[128];


    if (file == NULL) {
        return -1;
    }
    fd = fileno(file);
    switch (fd) {
        case 0:
            bufPtr = "stdin";
            break;
        case 1:
            bufPtr = "stdout";
            break;
        case 2:
            bufPtr = "stderr";
            break;
        default:
            bufPtr = buffer;
            sprintf(buffer, "file%d", fd);
            break;
    }
    hTablePtr = (Tcl_HashTable *) Tcl_GetAssocData(interp, "tclFileTable",
            NULL);
    if (hTablePtr == (Tcl_HashTable *) NULL) {
        return -1;
    }
    hPtr = Tcl_FindHashEntry(hTablePtr, bufPtr);
    if (hPtr == (Tcl_HashEntry *) NULL) {
        return -1;
    }
    oFilePtr = (TclOpenFile *) Tcl_GetHashValue(hPtr);
    if (oFilePtr == (TclOpenFile *) NULL) {
        return -1;
    }
    return oFilePtr->permissions;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_ShareChannel -
 *
 *	Shares an IO channel between two interpreters. This
 *	enables Tcl programs in the other interpreter to use the resource ID
 *	to refer to the same resource.
 *
 *	At this time this function is only able to share file IDs. When we
 *	implement generic IO channels the function will be generalized to
 *	enable sharing these too.
 *
 * Results:
 *	Standard Tcl result.
 *
 * Side effects:
 *	Causes the IO channel to become referenced from both interpreters.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_ShareChannel(channelId, interp, srcInterp, destInterp)
    char *channelId;		/* Hash key for file to be shared. */
    Tcl_Interp *interp;		/* Interpreter to return result in. */
    Tcl_Interp *srcInterp;	/* Interpreter to get file from. */
    Tcl_Interp *destInterp;	/* Interpreter to give file to. */
{
    TclOpenFile *oFilePtr;	/* Pointer to file to share. */
    TclOpenFile *dFilePtr;	/* Pointer to any previous file in destination
                                 * interpreter known by the same channel Id. */
    Tcl_HashTable *hTabSrcPtr;	/* Source file table. */
    Tcl_HashTable *hTabDestPtr;	/* Destination file table. */
    Tcl_HashEntry *hSrcPtr;	/* Source hash entry. */
    Tcl_HashEntry *hDestPtr;	/* Destination hash entry. */
    int fd, new, i;
    
    if (channelId == NULL) {
        Tcl_AppendResult(interp, "NULL resource identifier", (char *) NULL);
        return TCL_ERROR;
    }

    if (strncmp(channelId, "file", 4) == 0) {
        char *end;

	fd = strtoul(channelId+4, &end, 10);
	if ((end == channelId+4) || (*end != 0)) {
            Tcl_AppendResult(interp, "bad file identifier \"", channelId,
                    "\"", (char *) NULL);
            return TCL_ERROR;
	}
        switch (fd) {
            case 0:
                channelId = "stdin";
                break;
            case 1:
                channelId = "stdout";
                break;
            case 2:
                channelId = "stderr";
                break;
            default:
                break;
        }
    }        
    
    if (srcInterp == destInterp) {
        return TCL_OK;
    }
    
    (void) TclMakeFileTable(srcInterp, 0);
    hTabSrcPtr = (Tcl_HashTable *) Tcl_GetAssocData(srcInterp, "tclFileTable",
            NULL);
    if (hTabSrcPtr == (Tcl_HashTable *) NULL) {
        panic("Tcl_ShareChannel: could not find file table");
    }
    (void) TclMakeFileTable(destInterp, 0);
    hTabDestPtr = (Tcl_HashTable *) Tcl_GetAssocData(destInterp,
            "tclFileTable", NULL);
    if (hTabDestPtr == (Tcl_HashTable *) NULL) {
        panic("Tcl_ShareChannel: could not find file table");
    }
    
    hSrcPtr = Tcl_FindHashEntry(hTabSrcPtr, channelId);
    if (hSrcPtr == (Tcl_HashEntry *) NULL) {
        Tcl_AppendResult(interp, "bad file identifier \"", channelId, "\"",
                (char *) NULL);
        return TCL_ERROR;
    }

    hDestPtr = Tcl_CreateHashEntry(hTabDestPtr, channelId, &new);

    if (new == 0) {
        oFilePtr = (TclOpenFile *) Tcl_GetHashValue(hSrcPtr);
        dFilePtr = (TclOpenFile *) Tcl_GetHashValue(hDestPtr);
        if (dFilePtr == oFilePtr) {
            return TCL_OK;
        }
        dFilePtr->refCount--;
        if (dFilePtr->refCount <= 0) {
            if (dFilePtr->f != NULL) {
                fclose(dFilePtr->f);
            }
            if (dFilePtr->f2 != NULL) {
                fclose(dFilePtr->f2);
            }
            if ((dFilePtr->numPids > 0) && (dFilePtr->pidPtr != NULL)) {
                for (i = 0; i < dFilePtr->numPids; i++) {
                    if (dFilePtr->pidPtr[i] != -1) {
                        Tcl_DetachPids(1, &dFilePtr->pidPtr[i]);
                    }
                }
                ckfree((char *) dFilePtr->pidPtr);
            }
            ckfree((char *) dFilePtr);
        }
    }
    oFilePtr = (TclOpenFile *) Tcl_GetHashValue(hSrcPtr);
    oFilePtr->refCount++;
    Tcl_SetHashValue(hDestPtr, (ClientData) oFilePtr);

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_TransferChannel --
 *
 *	Transfers an IO channel between two interpreters. The second
 *	interpreter gets a reference to the channel and the first one
 *	loses its reference. This enables Tcl programs to transfer the
 *	channel ID from one interpreter to another to enable the receiving
 *	interpreter to refer to this channel in Tcl programs.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	Causes the channel to become referenced in one interpreter and
 *	unreferenced in another.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_TransferChannel(channelId, interp, srcInterp, destInterp)
    char *channelId;		/* Hash key for file to be shared. */
    Tcl_Interp *interp;		/* Interpreter to return result in. */
    Tcl_Interp *srcInterp;	/* Interpreter to get file from. */
    Tcl_Interp *destInterp;	/* Interpreter to give file to. */
{
    int result;			/* Result of transfer. */
    Tcl_HashTable *hTabSrcPtr;	/* Source file table. */
    Tcl_HashEntry *hSrcPtr;	/* Source hash entry. */
    
    if (srcInterp == destInterp) {
        return TCL_OK;
    }
    result = Tcl_ShareChannel(channelId, interp, srcInterp, destInterp);
    if (result != TCL_OK) {
        return result;
    }
    hTabSrcPtr = (Tcl_HashTable *) Tcl_GetAssocData(srcInterp, "tclFileTable",
            NULL);
    if (hTabSrcPtr == (Tcl_HashTable *) NULL) {
        panic("Tcl_TransferChannel: could not find file table\n");
    }
    hSrcPtr = Tcl_FindHashEntry(hTabSrcPtr, channelId);
    if (hSrcPtr == (Tcl_HashEntry *) NULL) {
        Tcl_AppendResult(interp, "bad file identifier \"", channelId, "\"",
                (char *) NULL);
        return TCL_ERROR;
    }
    Tcl_DeleteHashEntry(hSrcPtr);

    return TCL_OK;
}
