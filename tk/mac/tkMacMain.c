/* 
 * tkMacMain.c --
 *
 *	This file contains the main program for "wish", a windowing
 *	shell based on Tk and Tcl.  It also provides a template that
 *	can be used as the basis for main programs for other Tk
 *	applications.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include <Windows.h>
#include <GestaltEqu.h>
#include <ToolUtils.h>
#include <Fonts.h>
#include <Dialogs.h>
#include <Memory.h>
#include <SegLoad.h>

#include <stdio.h>
#include <tcl.h>
#include "tk.h"
#include "tkInt.h"
#include "tkMacInt.h"

/*
 * Declarations for various library procedures and variables (don't want
 * to include tkInt.h or tkConfig.h here, because people might copy this
 * file out of the Tk source directory to make their own modified versions).
 */

extern void	exit _ANSI_ARGS_((int status));
extern int	isatty _ANSI_ARGS_((int fd));
/* TODO: prototype for fileno */

/* TODO: should be in tkInt.h */
int		Console_Init _ANSI_ARGS_((Tcl_Interp *interp));
/* TODO: prototype for panic */

#ifdef __MWERKS__
void		GUSISetup _ANSI_ARGS_((void (*socketfamily)()));
void		GUSIwithInternetSockets _ANSI_ARGS_((void));
void		GUSIwithSIOUXSockets _ANSI_ARGS_((void));
#endif
/*
 * Global variables used by the main program:
 */

static Tk_Window mainWindow;	/* The main window for the application.  If
				 * NULL then the application no longer
				 * exists. */
static Tcl_Interp *interp;	/* Interpreter for this application. */
char *tcl_RcFileName = NULL;	/* Name of a user-specific startup script
				 * to source if the application is being run
				 * interactively (e.g. "~/.wishrc").  Set
				 * by Tcl_AppInit.  NULL means don't source
				 * anything ever. */
static int tty;			/* Non-zero means standard input is a
				 * terminal-like device.  Zero means it's
				 * a file. */
static char errorExitCmd[] = "exit 1";

/*
 * Command-line options:
 */

static char *fileName = NULL;
static char *geometry = NULL;

static Tk_ArgvInfo argTable[] = {
    {"-file", TK_ARGV_STRING, (char *) NULL, (char *) &fileName,
     "File from which to read commands"},
    {"-geometry", TK_ARGV_STRING, (char *) NULL, (char *) &geometry,
     "Initial geometry for window"},
    {(char *) NULL, TK_ARGV_END, (char *) NULL, (char *) NULL,
     (char *) NULL}
};

/*
 * Declaration for Tcl command procedure to create demo widget.  This
 * procedure is only invoked if SQUARE_DEMO is defined.
 */

extern int SquareCmd _ANSI_ARGS_((ClientData clientData,
	Tcl_Interp *interp, int argc, char *argv[]));

/*
 * Forward declarations for procedures defined later in this file:
 */

static int		MacintoshInit _ANSI_ARGS_((void));
static int		SetupMainInterp _ANSI_ARGS_((Tcl_Interp *interp));


/*
 *----------------------------------------------------------------------
 *
 * main --
 *
 *	Main program for Wish.
 *
 * Results:
 *	None. This procedure never returns (it exits the process when
 *	it's done
 *
 * Side effects:
 *	This procedure initializes the wish world and then starts
 *	interpreting commands;  almost anything could happen, depending
 *	on the script being interpreted.
 *
 *----------------------------------------------------------------------
 */

int
main(argc, argv)
    int argc;				/* Number of arguments. */
    char **argv;			/* Array of argument strings. */
{
    char *msg;
    int code;

    if (MacintoshInit()  != TCL_OK) {
	exit(0);
    }

    interp = Tcl_CreateInterp();
#ifdef TCL_MEM_DEBUG
    Tcl_InitMemory(interp);
#endif

    SetupMainInterp(interp);

    /*
     * Initialize the Tk application.
     * TODO: name (Simple Tk) should come from actual start up name
     */

    mainWindow = Tk_CreateMainWindow(interp, NULL, "Simple Tk", "Tk");
    if (mainWindow == NULL) {
	/*
	 * Very little we can do here.  Can't even print an error.
	 */
	exit(1);
    }
    Tk_GeometryRequest(mainWindow, 200, 200);

    /* Tk_EventInit(interp); */
    if (Console_Init(interp)  != TCL_OK) {
	exit(0);
    }

    /*
     * Make command-line arguments available in the Tcl variables "argc"
     * and "argv".  Also set the "geometry" variable from the geometry
     * specified on the command line.
     */

    Tcl_SetVar(interp, "argv", "", TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "argc", "0", TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "argv0", (fileName != NULL) ? fileName : argv[0],
	    TCL_GLOBAL_ONLY);
    if (geometry != NULL) {
	Tcl_SetVar(interp, "geometry", geometry, TCL_GLOBAL_ONLY);
    }

    /*
     * Set the "tcl_interactive" variable.
     */

    tty = isatty(0);
    Tcl_SetVar(interp, "tcl_interactive",
	    ((fileName == NULL) && tty) ? "1" : "0", TCL_GLOBAL_ONLY);

    /*
     * Add a few application-specific commands to the application's
     * interpreter.
     */

#ifdef SQUARE_DEMO
    Tcl_CreateCommand(interp, "square", SquareCmd, (ClientData) mainWindow,
	    (void (*)()) NULL);
#endif

    /*
     * Invoke application-specific initialization.
     */

    if (Tcl_AppInit(interp) != TCL_OK) {
	fprintf(stderr, "Tcl_AppInit failed: %s\n", interp->result);
    }

    /*
     * Set the geometry of the main window, if requested.
     */

    if (geometry != NULL) {
	code = Tcl_VarEval(interp, "wm geometry . ", geometry, (char *) NULL);
	if (code != TCL_OK) {
	    fprintf(stderr, "%s\n", interp->result);
	}
    }

    /*
     * Invoke the script specified on the command line, if any.
     */

    if (fileName != NULL) {
	code = Tcl_VarEval(interp, "source ", fileName, (char *) NULL);
	if (code != TCL_OK) {
	    goto error;
	}
	tty = 0;
    } else {
	/*
	 * Commands will come from standard input, so set up an event
	 * handler for standard input.  If the input device is aEvaluate the
	 * .rc file, if one has been specified, set up an event handler
	 * for standard input, and print a prompt if the input
	 * device is a terminal.
	 */

	if (tcl_RcFileName != NULL) {
	    Tcl_DString buffer;
	    char *fullName;
	    FILE *f;
    
	    fullName = Tcl_TildeSubst(interp, tcl_RcFileName, &buffer);
	    if (fullName == NULL) {
		fprintf(stderr, "%s\n", interp->result);
	    } else {
		f = fopen(fullName, "r");
		if (f != NULL) {
		    code = Tcl_EvalFile(interp, fullName);
		    if (code != TCL_OK) {
			fprintf(stderr, "%s\n", interp->result);
		    }
		    fclose(f);
		}
	    }
	    Tcl_DStringFree(&buffer);
	}
    }

    /*
     * Attach the global interpreter to tk's expected global console
     */
    tkMacInstallMWConsole(interp);

    /*
     * Loop infinitely, waiting for commands to execute.  When there
     * are no windows left, Tk_MainLoop returns and we exit.
     */

    Tk_MainLoop();
    
    /*
     * Don't exit directly, but rather invoke the Tcl "exit" command.
     * This gives the application the opportunity to redefine "exit"
     * to do additional cleanup.
     */

    Tcl_Eval(interp, "exit");
    exit(1);

    error:
    msg = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    if (msg == NULL) {
	msg = interp->result;
    }
    fprintf(stderr, "%s\n", msg);
    Tcl_Eval(interp, errorExitCmd);
    return 1;			/* Needed only to prevent compiler warnings. */
}

/*
 *----------------------------------------------------------------------
 *
 * MacintoshInit --
 *
 *	This procedure calls Mac specific initilization calls.  Most of
 *	these calls must be made as soon as possible in the startup
 *	process.
 *
 * Results:
 *	Returns TCL_OK if everything went fine.  If it didn't the 
 *	application should probably fail.
 *
 * Side effects:
 *	Inits the application.
 *
 *----------------------------------------------------------------------
 */

static int
MacintoshInit()
{
    int i;
    long result, mask=0x0700; 		/* mask = system 7.x */
    
    InitGraf(&qd.thePort);
    InitFonts();
    InitWindows();
    InitMenus();
    InitDialogs((long) NULL);		
    InitCursor();

    /*
     * Make sure we are running on system 7 or higher
     */
    if (!((Gestalt(gestaltSystemVersion, &result) == noErr)
	    && (mask == (result & mask)))) {
	panic("Tcl/Tk requires system 7 or higher.");
    }

    
    FlushEvents(everyEvent, 0);
    SetEventMask(everyEvent);

    /*
     * Set up stack & heap sizes
     */
    /* TODO: stack size
       size = StackSpace();
       SetAppLimit(GetAppLimit() - 8192);
     */
    MaxApplZone();
    for (i=0; i<4; i++) {
	/* TODO: how many master pointers will Tk really need? */
	(void) MoreMasters();
    }

    GUSISetup(GUSIwithInternetSockets);
    GUSISetup(GUSIwithSIOUXSockets);

    return TCL_OK;
} /* MacintoshInit */

/*
 *----------------------------------------------------------------------
 *
 * SetupMainInterp --
 *
 *	This procedure calls initalization routines require a Tcl 
 *	interp as an argument.  This call effectively makes the passed
 *	iterpreter the "main" interpreter for the application.
 *
 * Results:
 *	Returns TCL_OK if everything went fine.  If it didn't the 
 *	application should probably fail.
 *
 * Side effects:
 *	More initilization.
 *
 *----------------------------------------------------------------------
 */

static int
SetupMainInterp(interp)
    Tcl_Interp *interp;
{
    /*
     * Set Mac display to some constant value.
     */

    Tcl_SetVar2(interp, "env", "DISPLAY", "Macintosh", TCL_GLOBAL_ONLY);

    TkMacInitAppleEvents(interp);
    TkMacInitMenus(interp);
    
    return TCL_OK;
}
