/* 
 * winMain.c --
 *
 *	Main entry point for tclsh and other Tcl-based applications.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <dos.h>
#include "tcl.h"

/*
 * Declarations for various procedures and variables that are needed by this
 * file.
 */

BOOL RegisterTerminalClass _ANSI_ARGS_((HINSTANCE hInstance));
HWND CreateTerminal _ANSI_ARGS_((HINSTANCE hInstance, Tcl_Interp *interp));

int initShowState;		/* Initial window startup state. */
HINSTANCE appInstance;		/* Application instance handle. */
static char buffer[1000];	/* Buffer for misc temporary strings. */
Tcl_Interp *interp;		/* Interpreter used by application. */
#ifdef TCL_MEM_DEBUG
static char dumpFile[100];	/* Records where to dump memory allocation
				 * information. */
static int quitFlag = 0;	/* 1 means the "checkmem" command was
				 * invoked, so the application should quit
				 * and dump memory allocation information. */
#endif

/*
 * Forward references for procedures defined later in this file:
 */

#ifdef TCL_MEM_DEBUG
static int		CheckmemCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char *argv[]));
#endif

#ifdef TCL_TEST
EXTERN int		TclTest_Init _ANSI_ARGS_((Tcl_Interp *interp));
#endif /* TCL_TEST */


/*
 *----------------------------------------------------------------------
 *
 * WinMain --
 *
 *	Main entry point from Windows.
 *
 * Results:
 *	Returns false if initialization fails, otherwise it never
 *	returns. 
 *
 * Side effects:
 *	Just about anything, since from here we call arbitrary Tcl code.
 *
 *----------------------------------------------------------------------
 */

int APIENTRY
WinMain(hInstance, hPrevInstance, lpszCmdLine, nCmdShow)
    HINSTANCE hInstance;
    HINSTANCE hPrevInstance;
    LPSTR lpszCmdLine;
    int nCmdShow;
{
    MSG msg;
    HWND hWnd;
    char *args, *fileName, **argv;
    int argc, code, tty;
    int exitCode = 0;

    /*
     * Register console window class only the first time the program
     * is run.
     */

    if (!hPrevInstance) {
	if (!RegisterTerminalClass(hInstance)) {
	    return FALSE;
	}
    }
    appInstance = hInstance;
    initShowState = nCmdShow;

    /* Increase the application queue size from default value of 8.
     * At the default value, cross application SendMessage of WM_KILLFOCUS
     * will fail because the handler will not be able to do a PostMessage!
     * This is only needed for Windows 3.x, since NT dynamically expands
     * the queue.
     */
    SetMessageQueue(64);

    Tcl_FindExecutable(NULL);
    interp = Tcl_CreateInterp();
#ifdef TCL_MEM_DEBUG
    Tcl_InitMemory(interp);
    Tcl_CreateCommand(interp, "checkmem", CheckmemCmd, (ClientData) 0,
	    (Tcl_CmdDeleteProc *) NULL);
#endif

    /*
     * Make command-line arguments available in the Tcl variables "argc"
     * and "argv".  If the first argument doesn't start with a "-" then
     * strip it off and use it as the name of a script file to process.
     */

    Tcl_SplitList(NULL, lpszCmdLine, &argc, &argv);
    fileName = NULL;
    if ((argc > 0) && (argv[0][0] != '-')) {
	fileName = argv[0];
	args = Tcl_Merge(argc, argv);
	Tcl_SetVar(interp, "argv", args, TCL_GLOBAL_ONLY);
	Tcl_SetVar(interp, "argv0", argv[0], TCL_GLOBAL_ONLY);
	ckfree(args);
	argc--;
    } else {
	GetModuleFileName(NULL, buffer, 1000);
	Tcl_SetVar(interp, "argv", lpszCmdLine, TCL_GLOBAL_ONLY);
	Tcl_SetVar(interp, "argv0", buffer, TCL_GLOBAL_ONLY);
    }
    sprintf(buffer, "%d", argc);
    Tcl_SetVar(interp, "argc", buffer, TCL_GLOBAL_ONLY);

    /*
     * Set the "tcl_interactive" variable.
     */

    Tcl_SetVar(interp, "tcl_interactive",
	    (fileName == NULL) ? "1" : "0", TCL_GLOBAL_ONLY);

    /*
     * Invoke application-specific initialization.
     */

    if (Tcl_AppInit(interp) != TCL_OK) {
	sprintf(buffer, "Tcl_AppInit failed: %s\n", interp->result);
	MessageBox(NULL, buffer, "Tclsh", MB_OK);
    }

    /*
     * If a script file was specified then just source that file
     * and quit.
     */

    if (fileName != NULL) {
	code = Tcl_EvalFile(interp, fileName);
	ckfree(argv);
	if (code != TCL_OK) {
	    sprintf(buffer, "%s\n", interp->result);
	    MessageBox(NULL, buffer, "Tclsh", MB_OK);
	    exitCode = 1;
	}
	goto done;
    } else {
	ckfree(argv);
    }

    /*
     * We're running interactively.  Source a user-specific startup
     * file if Tcl_AppInit specified one and if the file exists.
     */

    fileName = Tcl_GetVar(interp, "tcl_rcFileName", TCL_GLOBAL_ONLY);

    if (fileName != NULL) {
	Tcl_DString str;
	char *fullName;
	FILE *f;

	fullName = Tcl_TildeSubst(interp, fileName, &str);
	if (fullName == NULL) {
	    sprintf(buffer, "%s\n", interp->result);
	    MessageBox(NULL, buffer, "Tclsh", MB_OK);
	} else {
	    f = fopen(fullName, "r");
	    if (f != NULL) {
		code = Tcl_EvalFile(interp, fullName);
		if (code != TCL_OK) {
		    sprintf(buffer, "%s\n", interp->result);
		    MessageBox(NULL, buffer, "Tclsh", MB_OK);
		}
		fclose(f);
	    }
	}
	Tcl_DStringFree(&str);
    }

    /*
     * Create and display the console window.
     */

    hWnd = CreateTerminal(appInstance, interp);
    if (!hWnd) {
	return;
    }
    ShowWindow(hWnd, initShowState);
    UpdateWindow(hWnd);

    /*
     * Enter the Windows event loop and process commands from the
     * console until a WM_QUIT message arrives.
     */

    while (GetMessage (&msg, NULL, 0, 0)) {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
#ifdef TCL_MEM_DEBUG
	if (quitFlag) {
	    Tcl_DeleteInterp(interp);
	    Tcl_DumpActiveMemory(dumpFile);
	    exit(0);
	}
#endif
    }

    /*
     * Rather than calling exit, invoke the "exit" command so that
     * users can replace "exit" with some other command to do additional
     * cleanup on exit.  The Tcl_Eval call should never return.
     */

    done:
    sprintf(buffer, "exit %d", exitCode);
    Tcl_Eval(interp, buffer);
}

/*
 *----------------------------------------------------------------------
 *
 * CheckmemCmd --
 *
 *	This is the command procedure for the "checkmem" command, which
 *	causes the application to exit after printing information about
 *	memory usage to the file passed to this command as its first
 *	argument.
 *
 * Results:
 *	Returns a standard Tcl completion code.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */
#ifdef TCL_MEM_DEBUG

	/* ARGSUSED */
static int
CheckmemCmd(clientData, interp, argc, argv)
    ClientData clientData;		/* Not used. */
    Tcl_Interp *interp;			/* Interpreter for evaluation. */
    int argc;				/* Number of arguments. */
    char *argv[];			/* String values of arguments. */
{
    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" fileName\"", (char *) NULL);
	return TCL_ERROR;
    }
    strcpy(dumpFile, argv[1]);
    quitFlag = 1;
    return TCL_OK;
}
#endif

/*
 *----------------------------------------------------------------------
 *
 * Tcl_AppInit --
 *
 *	This procedure performs application-specific initialization.
 *	Most applications, especially those that incorporate additional
 *	packages, will have their own version of this procedure.
 *
 * Results:
 *	Returns a standard Tcl completion code, and leaves an error
 *	message in interp->result if an error occurs.
 *
 * Side effects:
 *	Depends on the startup script.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_AppInit(interp)
    Tcl_Interp *interp;		/* Interpreter for application. */
{
    if (Tcl_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }

#ifdef TCL_TEST
    if (TclTest_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
#endif /* TCL_TEST */

    /*
     * Call the init procedures for included packages.  Each call should
     * look like this:
     *
     * if (Mod_Init(interp) == TCL_ERROR) {
     *     return TCL_ERROR;
     * }
     *
     * where "Mod" is the name of the module.
     */

    /*
     * Call Tcl_CreateCommand for application-specific commands, if
     * they weren't already created by the init procedures called above.
     */

    /*
     * Specify a user-specific startup file to invoke if the application
     * is run interactively.  Typically the startup file is "~/.apprc"
     * where "app" is the name of the application.  If this line is deleted
     * then no user-specific startup file will be run under any conditions.
     */

    Tcl_SetVar(interp, "tcl_rcFileName", "~/.tclshrc", TCL_GLOBAL_ONLY);
    return TCL_OK;
}
