/* 
 * winMain.c --
 *
 *	Main entry point for wish and other Tk-based applications.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tkWinInt.h"
#include <dos.h>

static void WishPanic _ANSI_ARGS_(TCL_VARARGS(char *,format));

/*
 * Global variables used by the main program:
 */

HINSTANCE appInstance;		/* Application instance handle. */
int initShowState;		/* Initial window startup state. */

static Tk_Window mainWindow;	/* The main window for the application.  If
				 * NULL then the application no longer
				 * exists. */
static Tcl_Interp *interp;	/* Interpreter for this application. */
char *tcl_RcFileName = NULL;	/* Name of a user-specific startup script
				 * to source if the application is being run
				 * interactively (e.g. "~/.wishrc").  Set
				 * by Tcl_AppInit.  NULL means don't source
				 * anything ever. */
static Tcl_DString command;	/* Used to assemble lines of terminal input
				 * into Tcl commands. */
static int tty;			/* Non-zero means standard input is a
				 * terminal-like device.  Zero means it's
				 * a file. */
static char defaultDisplay[] = "localhost:0";
static char argv0[255];		/* Buffer used to hold argv0. */

/*
 * Command-line options:
 */

static int synchronize = 0;
static char *fileName = NULL;
static char *name = NULL;
static char *display = NULL;
static char *geometry = NULL;

static Tk_ArgvInfo argTable[] = {
    {"-display", TK_ARGV_STRING, (char *) NULL, (char *) &display,
	"Display to use"},
    {"-geometry", TK_ARGV_STRING, (char *) NULL, (char *) &geometry,
	"Initial geometry for window"},
    {"-name", TK_ARGV_STRING, (char *) NULL, (char *) &name,
	"Name to use for application"},
    {"-sync", TK_ARGV_CONSTANT, (char *) 1, (char *) &synchronize,
	"Use synchronous mode for display server"},
    {(char *) NULL, TK_ARGV_END, (char *) NULL, (char *) NULL,
	(char *) NULL}
};

/*
 * Forward declarations for procedures defined later in this file:
 */

#ifdef TK_TEST
EXTERN int		TkTest_Init _ANSI_ARGS_((Tcl_Interp *interp));
#endif /* TK_TEST */


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
    char **argv, **argvlist, *args, *p, *class;
    int argc, size, i, code;
    char buf[20];
    size_t length;

    appInstance = hInstance;
    initShowState = nCmdShow;

    Tcl_SetPanicProc(WishPanic);

    TkWinXInit(hInstance);

    /*
     * Increase the application queue size from default value of 8.
     * At the default value, cross application SendMessage of WM_KILLFOCUS
     * will fail because the handler will not be able to do a PostMessage!
     * This is only needed for Windows 3.x, since NT dynamically expands
     * the queue.
     */
    SetMessageQueue(64);

    interp = Tcl_CreateInterp();
#ifdef TCL_MEM_DEBUG
    Tcl_InitMemory(interp);
#endif

    /*
     * First get an upper bound on the size of the argv array by counting the
     * number of whitespace characters in the string.
     */

    for (size = 1, p = lpszCmdLine; *p != '\0'; p++) {
	if (isspace(UCHAR(*p))) {
	    size++;
	}
    }
    size++;			/* Leave space for final NULL pointer. */
    argvlist = (char **) ckalloc((unsigned) (size * sizeof(char *)));
    argv = argvlist;

    /*
     * Split the command line into words, and store pointers to the start of
     * each word into the argv array.  Skips leading whitespace on each word.
     */

    for (i = 0, p = lpszCmdLine; *p != '\0'; i++) {
	while (isspace(UCHAR(*p))) {
	    p++;
	}
	if (*p == '\0') {
	    break;
	}
	argv[i] = p;
	while (*p != '\0' && !isspace(UCHAR(*p))) {
	    p++;
	}
	if (*p != '\0') {
	    *p = '\0';
	    p++;
	}
    }
    argv[i] = NULL;
    argc = i;

    /*
     * Parse command-line arguments.  A leading "-file" argument is
     * ignored (a historical relic from the distant past).  If the
     * next argument doesn't start with a "-" then strip it off and
     * use it as the name of a script file to process.  Also check
     * for other standard arguments, such as "-geometry", anywhere
     * in the argument list.
     */

    GetModuleFileName(NULL, argv0, 255);

    if (argc > 0) {
	length = strlen(argv[0]);
	if ((length >= 2) && (strncmp(argv[0], "-file", length) == 0)) {
	    argc--;
	    argv++;
	}
    }
    if ((argc > 0) && (argv[0][0] != '-')) {
	fileName = argv[0];
	argc--;
	argv++;
    }
    if (Tk_ParseArgv(interp, (Tk_Window) NULL, &argc, argv, argTable,
	    TK_ARGV_DONT_SKIP_FIRST_ARG) != TCL_OK) {
	WishPanic("%s\n", interp->result);
    }
    if (name == NULL) {
	if (fileName != NULL) {
	    p = fileName;
	} else {
	    p = argv0;
	}
	name = strrchr(p, '\\');
	if (name != NULL) {
	    name++;
	} else {
	    name = p;
	}
    }

    /*
     * Make command-line arguments available in the Tcl variables "argc"
     * and "argv".    Also set the "geometry" variable from the geometry
     * specified on the command line.
     */

    args = Tcl_Merge(argc, argv);
    Tcl_SetVar(interp, "argv", args, TCL_GLOBAL_ONLY);
    ckfree(args);
    sprintf(buf, "%d", argc);
    Tcl_SetVar(interp, "argc", buf, TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "argv0", (fileName != NULL) ? fileName : argv0,
	    TCL_GLOBAL_ONLY);
    if (geometry != NULL) {
	Tcl_SetVar(interp, "geometry", geometry, TCL_GLOBAL_ONLY);
    }

    /*
     * If a display was specified, put it into the DISPLAY
     * environment variable so that it will be available for
     * any sub-processes created by us.
     */

    if (display == NULL) {
	display = defaultDisplay;
    }
    Tcl_SetVar2(interp, "env", "DISPLAY", display, TCL_GLOBAL_ONLY);

    /*
     * Initialize the Tk application.  If a -name option was provided,
     * use it;  otherwise, if a file name was provided, use the last
     * element of its path as the name of the application; otherwise
     * use the last element of the program name.  For the application's
     * class, capitalize the first letter of the name.
     */

    if (name == NULL) {
	p = (fileName != NULL) ? fileName : argv0;
	name = strrchr(p, '/');
	if (name != NULL) {
	    name++;
	} else {
	    name = strrchr(p, '\\');
	    if (name != NULL) {
		name++;
	    } else {
		name = p;
	    }
	}
    }
    class = ckalloc((unsigned) (strlen(name) + 1));
    strcpy(class, name);
    class[0] = toupper((unsigned char) class[0]);
    mainWindow = Tk_CreateMainWindow(interp, display, name, class);
    ckfree(class);
    if (mainWindow == NULL) {
	WishPanic("%s\n", interp->result);
    }

    /*
     * Set the "tcl_interactive" variable.
     */

    tty = (fileName == NULL) ? 1 : 0;
    Tcl_SetVar(interp, "tcl_interactive", (tty ? "1" : "0"), TCL_GLOBAL_ONLY);

    /*
     * Invoke application-specific initialization.
     */

    if (Tcl_AppInit(interp) != TCL_OK) {
	WishPanic("Tcl_AppInit failed: %s\n", interp->result);
    }

    /*
     * Invoke the script specified on the command line, if any.
     */

    if (fileName != NULL) {
	code = Tcl_EvalFile(interp, fileName);
	if (code != TCL_OK) {
	    p = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
	    if ((p == NULL) || (*p == '\0')) {
		p = interp->result;
	    }
	    WishPanic("%s\n", p);
	}
	tty = 0;
    } else {
	/*
	 * Commands will come from the console.  Evaluate the .rc file, if one
	 * has been specified.  Create the console if the input device
	 * is a terminal.
	 */

	if (tcl_RcFileName != NULL) {
	    Tcl_DString buffer;
	    char *fullName;
	    FILE *f;
    
	    fullName = Tcl_TildeSubst(interp, tcl_RcFileName, &buffer);
	    if (fullName == NULL) {
		WishPanic("%s\n", interp->result);
	    } else {
		f = fopen(fullName, "r");
		if (f != NULL) {
		    code = Tcl_EvalFile(interp, fullName);
		    if (code != TCL_OK) {
			WishPanic("%s\n", interp->result);
		    }
		    fclose(f);
		}
	    }
	    Tcl_DStringFree(&buffer);
	}
	if (tty) {
	    /*
	     * Create and display the console window.
	     */

	    if (Console_Init(interp) != TCL_OK) {
		return;
	    }
	}
    }
    Tcl_DStringInit(&command);
    Tcl_ResetResult(interp);

    /*
     * Set the geometry of the main window, if requested.  If the "geometry"
     * variable has gone away, this means that the application doesn't want
     * us to set the geometry after all.
     */

    if (geometry != NULL) {
	p = Tcl_GetVar(interp, "geometry", TCL_GLOBAL_ONLY);
	if (p != NULL) {
	    code = Tcl_VarEval(interp, "wm geometry . ", p, (char *) NULL);
	    if (code != TCL_OK) {
		WishPanic("%s\n", interp->result);
	    }
	}
    }

    /*
     * Loop infinitely, waiting for commands to execute.  When there
     * are no windows left, Tk_MainLoop returns and we exit.
     */

    ckfree((char *)argvlist);

    Tk_MainLoop();

    /*
     * Don't exit directly, but rather invoke the Tcl "exit" command.
     * This gives the application the opportunity to redefine "exit"
     * to do additional cleanup.
     */

    Tcl_Eval(interp, "exit");
    exit(1);
}

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
    Tk_Window main;

    main = Tk_MainWindow(interp);

    if (Tcl_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    if (Tk_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
#ifdef TK_TEST
    if (TkTest_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
#endif /* TK_TEST */

    tcl_RcFileName = "~/wishrc.tcl";
    return TCL_OK;
}

/*
 *--------------------------------------------------------------
 *
 * Tk_MainLoop --
 *
 *	Call Tk_DoOneEvent over and over again in an infinite
 *	loop as long as there exist any main windows.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Arbitrary;  depends on handlers for events.
 *
 *--------------------------------------------------------------
 */

void
Tk_MainLoop()
{
    while (Tk_GetNumMainWindows() > 0) {
	Tk_DoOneEvent(0);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * WishPanic --
 *
 *	Display a message and exit.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Exits the program.
 *
 *----------------------------------------------------------------------
 */

void
WishPanic TCL_VARARGS_DEF(char *,arg1)
{
    va_list argList;
    char buf[1024];
    char *format;
    
    format = TCL_VARARGS_START(char *,arg1,argList);
    vsprintf(buf, format, argList);

    MessageBeep(MB_ICONEXCLAMATION);
    MessageBox(NULL, buf, "Fatal Error in Wish",
	    MB_ICONSTOP | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
    exit(1);
}
