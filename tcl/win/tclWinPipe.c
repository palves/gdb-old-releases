/* 
 * tclWinPipe.c -- This file implements the Windows-specific pipeline exec
 *                 functions.
 *      
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tclInt.h"
#include "tclPort.h"

#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include <sys/stat.h>

#include "tclWinInt.h"

static BOOL IsWinApp _ANSI_ARGS_((char *filename, Tcl_Interp *interp));

static int Win32Pipeline _ANSI_ARGS_((Tcl_Interp *interp, int *pidPtr,
	int *numPids, int argc, char **argv, int *errorId, int *inputId,
	int *outputId, int *closeInput, int *pipeIds, int *errpipeIds,
	int *maxFd));
static int Win32sPipeline _ANSI_ARGS_((Tcl_Interp *interp, int argc,
	char **argv, char *IntIn, char *FinalOut, int *pidPtr));

/*
 *----------------------------------------------------------------------
 *
 * TclSpawnPipeline --
 *
 *      Calls either Win32Pipeline or Win32sPipeline, depending on which
 *      version of Windows is running.
 *
 * Results:
 *      The return value is 1 on success, 0 on error
 *
 * Side effects:
 *      Processes and pipes are created.
 *
 *----------------------------------------------------------------------
 */

int
TclSpawnPipeline(interp, pidPtr, numPids, argc, argv, errorId, inputId,
	outputId, closeInput, pipeIds, errpipeIds, maxFd,
	IntIn, FinalOut)
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
    if (TclCheckForPipes()) {
	return(Win32Pipeline(interp, pidPtr, numPids, argc, argv, errorId,
		inputId, outputId, closeInput, pipeIds, errpipeIds, maxFd));
    }
    
    return(Win32sPipeline(interp, argc, argv, IntIn, FinalOut, pidPtr));
}

/*
 *----------------------------------------------------------------------
 *
 * Win32Pipeline --
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

static int
Win32Pipeline(interp, pidPtr, numPids, argc, argv, errorId, inputId, outputId,
	closeInput, pipeIds, errpipeIds, maxFd)
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
{
    STARTUPINFO startInfo;           /* Win32 structure used to spawn
				      * processes (through CreateProcess()).
				      * Contains initial status of process. */
    PROCESS_INFORMATION procInfo;    /* Win32 structure used to spawn
				      * processes (through CreateProcess()).
				      * Contains information such as the pid */
    SECURITY_ATTRIBUTES secAtts;     /* Win32 structure used to specify the
				      * security descriptor for a file. */
    int firstArg, lastArg;           /* Indices of first and last arguments
				      * in current command. */
    int pid, i, err;
    Tcl_DString execProc;
    int emptyPipe[2];
    HANDLE dupIn, dupOut, dupError, handle;
    int curOutputId = -1;
    int joinThisError;
    int result = TCL_OK;
    char *ptr;

    startInfo.cb = sizeof(startInfo);
    startInfo.lpReserved = NULL;
    startInfo.lpDesktop = NULL;
    startInfo.lpTitle = NULL;
    startInfo.dwX = startInfo.dwY = 0;
    startInfo.dwXSize = startInfo.dwYSize = 0;
    startInfo.dwXCountChars = startInfo.dwYCountChars = 0;
    startInfo.dwFillAttribute = 0;
    startInfo.dwFlags = STARTF_USESTDHANDLES;
    startInfo.wShowWindow = 0;
    startInfo.cbReserved2 = 0;
    startInfo.lpReserved2 = NULL;

    secAtts.nLength = sizeof(SECURITY_ATTRIBUTES);
    secAtts.lpSecurityDescriptor = NULL;
    secAtts.bInheritHandle = TRUE;

    emptyPipe[0] = -1;
    emptyPipe[1] = -1;
    pipeIds[0] = -1;
    pipeIds[1] = -1;
    
    Tcl_DStringInit(&execProc);

    for (firstArg = 0; firstArg < argc; firstArg = lastArg+1) { 

	dupIn = dupOut = dupError = NULL;
	startInfo.hStdInput = startInfo.hStdOutput = startInfo.hStdError =
	    INVALID_HANDLE_VALUE;

	/* 
	 * NT 3.1 has a bug. Documentation claims that NT will ignore any
	 * startInfo handles which are set to INVALID_HANDLE_VALUE. This is not
	 * true under NT 3.1. MS claims that this is fixed under NT 3.5 but
	 * that appears to be incorrect. So, the fix is to create a pipe to
	 * which we redirect any output we don't care about. Pipe is then
	 * destroyed at the end of the loop.
	 */

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
	    if (!TclCreatePipe(pipeIds)) {
		Tcl_AppendResult(interp, "couldn't create pipe: ",
			Tcl_PosixError(interp), (char *) NULL);
		result = TCL_ERROR;
		goto cleanup;
	    }
	    curOutputId = pipeIds[1];
	}

	Tcl_DStringSetLength(&execProc, 0);
	Tcl_DStringAppend(&execProc, argv[firstArg], -1);

	/*
	 * Some programs fail if the program name doesn't use backslash
	 * as the path separator, so we must convert forward slashes to
	 * backslashes here.
	 */

	TclWinConvertToNativePath(Tcl_DStringValue(&execProc));
	
	/*
	 * Now append the rest of the command line arguments.
	 */

	for (i = firstArg + 1; i < lastArg; i++) {
	    Tcl_DStringAppend(&execProc, " ", -1);
	    Tcl_DStringAppend(&execProc, argv[i], -1);
	}

	/*
	 * Duplicate all the handles which will be passed off as stdin, stdout
	 * and stderr of the child process. The duplicate handles are set to
	 * be inheritable, so the child process can use them.
	 */

	if (curOutputId != -1) {
	    handle = (HANDLE)_get_osfhandle(curOutputId);
	    err = DuplicateHandle(GetCurrentProcess(), handle,
		    GetCurrentProcess(), &dupOut, 0, TRUE,
		    DUPLICATE_SAME_ACCESS);
	    startInfo.hStdOutput = dupOut;
	}
	if (joinThisError) {
	    if (curOutputId != -1) {
		startInfo.hStdError = dupOut;
	    }
	} else {
	    if (*errorId != -1) {
		DuplicateHandle(GetCurrentProcess(),
			(HANDLE)_get_osfhandle(*errorId), GetCurrentProcess(),
			&dupError, 0, TRUE, DUPLICATE_SAME_ACCESS);
		startInfo.hStdError = dupError;
	    }
	}
	if (*inputId != -1) {
	    handle = (HANDLE)_get_osfhandle(*inputId);
	    err = DuplicateHandle(GetCurrentProcess(), handle,
		    GetCurrentProcess(), &dupIn, 0, TRUE,
		    DUPLICATE_SAME_ACCESS);
	    startInfo.hStdInput = dupIn;
	}
	
	/*
	 * If any handle was not set, open the null device instead.
	 */

	if (startInfo.hStdInput == INVALID_HANDLE_VALUE) {
	    startInfo.hStdInput = CreateFile("NUL:", GENERIC_READ, 0,
		    &secAtts, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}
	if (startInfo.hStdOutput == INVALID_HANDLE_VALUE) {
	    startInfo.hStdOutput = CreateFile("NUL:", GENERIC_WRITE, 0,
		    &secAtts, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}
	if (startInfo.hStdError == INVALID_HANDLE_VALUE) {
	    startInfo.hStdError = CreateFile("NUL:", GENERIC_WRITE, 0,
		    &secAtts, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	if (!CreateProcess(NULL, Tcl_DStringValue(&execProc), NULL, NULL,
		TRUE, DETACHED_PROCESS, NULL, NULL, &startInfo, &procInfo)) {
	    Tcl_DStringSetLength(&execProc, Tcl_DStringLength(&execProc)+7);
	    ptr = Tcl_DStringValue(&execProc);
	    memmove(ptr+7, ptr, strlen(ptr));
	    strncpy(ptr, "cmd /C ", 7);

	    if (!CreateProcess(NULL, ptr, NULL, NULL, TRUE,
		    DETACHED_PROCESS, NULL, NULL, &startInfo, &procInfo)) {
		TclWinConvertError(GetLastError());
		Tcl_AppendResult(interp, "couldn't fork child process: ",
			Tcl_PosixError(interp), (char *) NULL);
		result = TCL_ERROR;
	    }
	}

	if (result == TCL_OK) {
	    pid = (int) procInfo.hProcess;
	    if (pid == -1) { 
		Tcl_AppendResult(interp, "couldn't fork child process: ",
			Tcl_PosixError(interp), (char *) NULL);
		result = TCL_ERROR;
	    } else {
		pidPtr[*numPids] = pid;
		(*numPids)++;
		CloseHandle(procInfo.hThread);
	    }
	}

	/*
	 * The following code cleans up after the process creation, and exits
	 * after an error.  Otherwise it just closes off our copies of file
	 * descriptors that were set up for this child, and sets up the input
	 * for the next child.
	 */

	cleanup:
	if ((*inputId != -1) && *closeInput) { 
	    close(*inputId);
	    *inputId = -1;
	}
	if ((curOutputId != -1) && (curOutputId != *outputId)) {
	    close(curOutputId);
	    curOutputId = -1;
	}

	*inputId = pipeIds[0];
	*closeInput = 1;
	pipeIds[0] = pipeIds[1] = -1;
	
	if (startInfo.hStdInput != INVALID_HANDLE_VALUE) {
	    CloseHandle(startInfo.hStdInput);
	}
	if (startInfo.hStdOutput != INVALID_HANDLE_VALUE) {
	    CloseHandle(startInfo.hStdOutput);
	}
	if ((startInfo.hStdError != INVALID_HANDLE_VALUE)
		&& (startInfo.hStdError != startInfo.hStdOutput)) {
	    CloseHandle(startInfo.hStdError);
	}
	
	if (result == TCL_ERROR) {
	    Tcl_DStringFree(&execProc);
	    return 0;
	}
    }
    Tcl_DStringFree(&execProc);
    return 1;
}

/*
 *----------------------------------------------------------------------
 *
 * TclCheckForPipes --
 *
 *      Checks to see if it's running under Win32s or Win32.
 *
 * Results:
 *      Returns 1 if running under Win32, 0 otherwise.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

int
TclCheckForPipes(void)
{
    OSVERSIONINFO info;
    info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&info);
    return((info.dwPlatformId == VER_PLATFORM_WIN32s) ? 0 : 1);
}

/*
 *----------------------------------------------------------------------
 *
 * TclCreatePipe --
 *
 *      Creates a pipe.
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
int pipeIds[2];
{
    return _pipe(pipeIds, 1024, O_TEXT|O_NOINHERIT) ? 0 : 1;
}

/*
 *----------------------------------------------------------------------
 *
 * Win32sPipeline --
 *
 *      Given an argc/argv array, instantiate a pipeline of processes
 *      as described by the argv. This is the Win32s version, which works
 *      by creating a batch file to run the pipeline.
 *
 * Results:
 *      The return value is 1 on success, 0 on error
 *
 * Side effects:
 *      A batch file describing the pipeline is executed.
 *
 *----------------------------------------------------------------------
 */

static int
Win32sPipeline(interp, argc, argv, intIn, finalOut, pidPtr)
    Tcl_Interp *interp;		/* Interpreter in which to process pipeline. */
    int argc;			/* Number of entries in argv. */
    char **argv;		/* Array of strings describing commands in
				 * pipeline plus I/O redirection with <,
				 * <<, >, etc. argv[argc] must be NULL. */
    char *intIn;		/* File name for initial input (for Win32s). */
    char *finalOut;		/* File name for final output (for Win32s). */
    int *pidPtr;		/* Array of pids which are created. */
{
    char batchFile[20];		/* Name of batch file */
    int batchHandle;		/* Handle to the batch file */ 
    int outputHandle;		/* Handle to final output file */
    char string[100];		/* Temporary string */
    int numBytes;
    int firstArg, lastArg;	/* Indices of first and last arguments
				 * in current command. */
    Tcl_DString execProc;	/* Dynamic string used to construct
				 * intermediate commands */
    char *ptr;			/* Temporary pointer - used to build
				 * execProc strings */
    char intOut[L_tmpnam];	/* Name of temporary file */
    int result = 1;		/* Return value of this function. */
    int dirLength, offset;
    
    Tcl_DStringInit(&execProc);

    /*
     * Create batch file
     */

    tmpnam(batchFile);
    ptr = strchr(batchFile, '.');
    strcpy(ptr, ".bat");
    batchHandle = open(batchFile, O_CREAT | O_WRONLY | O_TEXT, 0600);
    if (batchHandle < 0) {
	Tcl_AppendResult(interp,
		"couldn't create batch file \"", batchFile,
		"\" for command: ", Tcl_PosixError(interp),
		(char *) NULL);
	result = 0;
	goto cleanup;
    }

    /*
     * Cycle through the pipeline, generating commands to place in batch file.
     */ 

    for (firstArg = 0; firstArg < argc; firstArg = lastArg+1) { 
	Tcl_DStringSetLength(&execProc, 0);
	for (lastArg = firstArg; lastArg < argc; lastArg++) {
	    if (argv[lastArg][0] == '|') {
		if (argv[lastArg][1] == 0) {
		    break;
		}
		if ((argv[lastArg][1] == '&') && (argv[lastArg][2] == 0)) {
		    break;
		}
	    } else {
		Tcl_DStringAppend(&execProc, argv[lastArg], -1);
		Tcl_DStringAppend(&execProc, " ", -1);
	    }
	}

	/*
	 * Check if this pipeline command is a Windows application.
	 */

	if (IsWinApp(argv[firstArg], interp)) {
	    if ((firstArg != 0) && (lastArg != argc)) {
		Tcl_AppendResult(interp,
			"Can't pipe input in or out of Windows app", finalOut,
			(char *) NULL);
		result = 0;
		goto cleanup;
	    }
	}

	/*
	 * Set input redirection for the current command in the pipeline.
	 */

	if (intIn[0]) {
	    Tcl_DStringAppend(&execProc, " < ", -1);
	    Tcl_DStringAppend(&execProc, intIn, -1);
	}

	/*
	 * Set output redirection for the current command in the pipeline.
	 */

	tmpnam(intOut);
	Tcl_DStringAppend(&execProc, " > ", -1);
	if (lastArg == argc) {
	    Tcl_DStringAppend(&execProc, finalOut, -1);
	} else {
	    Tcl_DStringAppend(&execProc, intOut, -1);
	}
	Tcl_DStringAppend(&execProc, "\r\n", -1);
	
	if (intIn[0]) {
	    Tcl_DStringAppend(&execProc, "del ", -1);
	    Tcl_DStringAppend(&execProc, intIn, -1);
	    Tcl_DStringAppend(&execProc, "\r\n", -1);
	}

	write(batchHandle, Tcl_DStringValue(&execProc),
		Tcl_DStringLength(&execProc));

	strcpy(intIn, intOut);
    }
    close(batchHandle);

    /*
     * Set up the command to execute the batch file
     */

    dirLength = GetCurrentDirectory(0, NULL);
    Tcl_DStringSetLength(&execProc, 0);
    Tcl_DStringAppend(&execProc, "command.com /C ", -1);
    offset = Tcl_DStringLength(&execProc);
    Tcl_DStringSetLength(&execProc, (dirLength - 1) + offset);
    GetCurrentDirectory(dirLength, Tcl_DStringValue(&execProc) + offset);
    Tcl_DStringAppend(&execProc, "\\", -1);
    Tcl_DStringAppend(&execProc, batchFile, -1);

    /*
     * Do a synchronous spawn of the batch file
     */

    TclSynchSpawn(Tcl_DStringValue(&execProc), SW_SHOWNORMAL);

    /*
     * Read the output from the command pipeline
     */

    outputHandle = open(finalOut, O_RDONLY | O_TEXT, 0600);
    if (outputHandle < 0) { 
	Tcl_AppendResult(interp,
		"couldn't read output file \"", finalOut,
		"\" for command: ", Tcl_PosixError(interp),
		(char *) NULL);
	result = 0;
	goto cleanup;
    }
    
    do {
	numBytes = read(outputHandle, string, 100);
	string[numBytes] = '\0';
	Tcl_AppendResult(interp, string, (char *)NULL);
    } while(numBytes > 0);
    close(outputHandle);

  cleanup:
    unlink(batchFile);
    unlink(finalOut);
    
    Tcl_DStringFree(&execProc);
    return(result);
}

/*
 *----------------------------------------------------------------------
 *
 * IsWinApp --
 *
 *      Given a filename, checks to see if that file is a Windows executable.
 *
 * Results:
 *      TRUE if it is a Windows executable, FALSE otherwise.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

static BOOL
IsWinApp(filename, interp)
    char *filename;                /* Name of file to check */
    Tcl_Interp *interp;            /* Interpreter in which pipeline is being
				    * processed. */ 
{
    int fileId;
    char buffer[MAX_PATH+1], *dummy;

   /*
    * If the word value at offset 18h of the file is 40h or greater,
    * the word value at 3Ch is an offset to a Windows header. Sooo.... we
    * must read the file header and check that offset to determine whether
    * "filename" is a Windows app.
    */

    if (!SearchPath(NULL, filename, ".exe", MAX_PATH, buffer, &dummy)) {
	return FALSE;
    }
	
    fileId = open(buffer, O_RDONLY, 0600);
    if (fileId < 0) {
	Tcl_AppendResult(interp,
		"couldn't open file \"", filename,
		"\" for command: ", Tcl_PosixError(interp),
		(char *) NULL);
    } else {
	read(fileId, buffer, MAX_PATH);
	close(fileId);
	if (buffer[24] >= 64) {
	    return(TRUE);
	}
    }
    return(FALSE);
}
