/* 
 * tkConsole.c --
 *
 *	This file implements a Tcl console for systems that may not
 *	otherwise have access to a console.  It uses the Text widget
 *	and provides special access via a console command.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tkPort.h"
#include "tk.h"

/*
 * A data structure of the following type holds information for each console
 * which a handler (i.e. a Tcl command) has been defined for a particular
 * top-level window.
 */

typedef struct ConsoleInfo {
    Tcl_Interp *consoleInterp;	/* Interpreter for the console. */
    Tk_Window consoleWindow;	/* The root window for the console. */
    Tcl_Interp *interp;		/* Interpreter to send console commands. */
} ConsoleInfo;

/*
 * Forward declarations for procedures defined later in this file:
 */

static int	ConsoleCmd _ANSI_ARGS_((ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv));
static void 	ConsoleDeleteProc _ANSI_ARGS_((ClientData clientData));
static void	ConsoleEventProc _ANSI_ARGS_((ClientData clientData,
		    XEvent *eventPtr));
static int	ConsolePutsCmd _ANSI_ARGS_((ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv));
static int	InterpreterCmd _ANSI_ARGS_((ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv));

/*
 *----------------------------------------------------------------------
 *
 * Console_Init --
 *
 *	Initialize the console.  This code actually creates a new
 *	application and associated interpreter.  This effectivly hides
 *	the implementation from the main application.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	A new console it created.
 *
 *----------------------------------------------------------------------
 */

int 
Console_Init(interp)
    Tcl_Interp *interp;			/* Interpreter to use for prompting. */
{
    Tcl_Interp *consoleInterp;
    Tk_Window consoleWindow;
    ConsoleInfo *info;
    Tk_Window mainWindow = Tk_MainWindow(interp);
#ifdef MAC_TCL
    static char initCmd[] = "source -rsrc {Console}";
#else
    static char initCmd[] = "source $tk_library/console.tcl";
#endif
    
    consoleInterp = Tcl_CreateInterp();
    if (consoleInterp == NULL) {
	goto error;
    }
    
    consoleWindow = Tk_CreateMainWindow(consoleInterp, NULL, "Console",
	    "Console");
    if (consoleWindow == NULL) {
	goto error;
    }

    if (Tcl_Init(consoleInterp) == TCL_ERROR) {
	goto error;
    }
    if (Tk_Init(consoleInterp) == TCL_ERROR) {
	goto error;
    }

    /* 
     * Add console commands to the interp 
     */
    info = (ConsoleInfo *) ckalloc(sizeof(ConsoleInfo));
    info->interp = interp;
    info->consoleInterp = consoleInterp;
    info->consoleWindow = consoleWindow;
    Tcl_CreateCommand(interp, "console", ConsoleCmd, (ClientData) info,
	    (Tcl_CmdDeleteProc *) ConsoleDeleteProc);
    Tcl_CreateCommand(interp, "puts", ConsolePutsCmd,  (ClientData) info,
	    (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(consoleInterp, "puts", ConsolePutsCmd,
	    (ClientData) info, (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(consoleInterp, "interp", InterpreterCmd,
	    (ClientData) info, (Tcl_CmdDeleteProc *) NULL);

    Tk_CreateEventHandler(mainWindow, StructureNotifyMask, ConsoleEventProc,
	    (ClientData) info);

    if (Tcl_Eval(consoleInterp, initCmd) == TCL_ERROR) {
	/* goto error; -- no problem for now... */
	printf("Eval error: %s", consoleInterp->result);
    }
    return TCL_OK;
    
    error:
    if (consoleWindow != NULL) {
	Tk_DestroyWindow(consoleWindow);
    }
    if (consoleInterp != NULL) {
    	Tcl_DeleteInterp(consoleInterp);
    }
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleCmd --
 *
 *	The console command implements a Tcl interface to the various console
 *	options.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
ConsoleCmd(clientData, interp, argc, argv)
    ClientData clientData;		/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    ConsoleInfo *info = (ConsoleInfo *) clientData;
    char c;
    int length;

    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" option ?arg arg ...?\"", (char *) NULL);
	return TCL_ERROR;
    }
    
    c = argv[1][0];
    length = strlen(argv[1]);
    if ((c == 't') && (strncmp(argv[1], "title", length)) == 0) {
	Tcl_DString dString;
	char *wmCmd = "wm title . {";
	
	Tcl_DStringInit(&dString);
	Tcl_DStringAppend(&dString, wmCmd, strlen(wmCmd));
	Tcl_DStringAppend(&dString, argv[2], strlen(argv[2]));
	Tcl_DStringAppend(&dString, "}", strlen("}"));
	Tcl_Eval(info->consoleInterp, dString.string);
	Tcl_DStringFree(&dString);
	return TCL_OK;
    } else if ((c == 'h') && (strncmp(argv[1], "hide", length)) == 0) {
	Tcl_Eval(info->consoleInterp, "wm withdraw .");
    } else if ((c == 's') && (strncmp(argv[1], "show", length)) == 0) {
	Tcl_Eval(info->consoleInterp, "wm deiconify .");
    } else {
	Tcl_AppendResult(interp, "bad option \"", argv[1],
		"\": should be hide, show, or title",
		(char *) NULL);
	return TCL_ERROR;
    }

    return TCL_OK;
} /* ConsoleCmd */

/*
 *----------------------------------------------------------------------
 *
 * InterpreterCmd --
 *
 *	This command allows the console interp to communicate with the
 *	main interpreter.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
InterpreterCmd(clientData, interp, argc, argv)
    ClientData clientData;		/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    ConsoleInfo *info = (ConsoleInfo *) clientData;
    char c;
    int length, code;

    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" option ?arg arg ...?\"", (char *) NULL);
	return TCL_ERROR;
    }
    
    c = argv[1][0];
    length = strlen(argv[1]);
    if ((c == 'e') && (strncmp(argv[1], "eval", length)) == 0) {
   	code = Tcl_GlobalEval(info->interp, argv[2]);
    	Tcl_AppendResult(interp, info->interp->result, (char *) NULL);
	return code;
    } else if ((c == 'r') && (strncmp(argv[1], "record", length)) == 0) {
   	code = Tcl_RecordAndEval(info->interp, argv[2], TCL_EVAL_GLOBAL);
    	Tcl_AppendResult(interp, info->interp->result, (char *) NULL);
	return TCL_OK;
    } else {
	Tcl_AppendResult(interp, "bad option \"", argv[1],
		"\": should be eval or record",
		(char *) NULL);
	return TCL_ERROR;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleDeleteProc --
 *
 *	If the console command is deleted we destroy the console window
 *	and all associated data structures.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	A new console it created.
 *
 *----------------------------------------------------------------------
 */

void 
ConsoleDeleteProc(clientData) 
    ClientData clientData;
{
    ConsoleInfo *info = (ConsoleInfo *) clientData;

    Tk_DestroyWindow(info->consoleWindow);
    Tcl_DeleteInterp(info->consoleInterp);
    info->consoleWindow = NULL;
    info->consoleInterp = NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsolePutsCmd --
 *
 *	Replacement for Tcl "puts" command that writes output to the
 *	console instead of stdout/stderr.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	Same as Tcl_PutsCmd, except that it puts the output string
 *	into the console if the specified file is stdout.
 *
 *----------------------------------------------------------------------
 */

int
ConsolePutsCmd(clientData, interp, argc, argv)
    ClientData clientData;		/* ConsoleInfo pointer. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    ConsoleInfo *info = (ConsoleInfo *) clientData;
    FILE *f;
    int i, newline;
    char *fileId;

    i = 1;
    newline = 1;
    if ((argc >= 2) && (strcmp(argv[1], "-nonewline") == 0)) {
	newline = 0;
	i++;
    }
    if ((i < (argc-3)) || (i >= argc)) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" ?-nonewline? ?fileId? string\"", (char *) NULL);
	return TCL_ERROR;
    }

    /*
     * The code below provides backwards compatibility with an old
     * form of the command that is no longer recommended or documented.
     */

    if (i == (argc-3)) {
	if (strncmp(argv[i+2], "nonewline", strlen(argv[i+2])) != 0) {
	    Tcl_AppendResult(interp, "bad argument \"", argv[i+2],
		    "\": should be \"nonewline\"", (char *) NULL);
	    return TCL_ERROR;
	}
	newline = 0;
    }
    if (i == (argc-1)) {
	fileId = "stdout";
    } else {
	fileId = argv[i];
	i++;
    }

    if (Tcl_GetOpenFile(interp, fileId, 1, 1, &f) != TCL_OK) {
	return TCL_ERROR;
    }

    if (fileno(f) == fileno(stdout) || fileno(f) == fileno(stderr)) {
	static char *newargv[3] = {"tkConsoleOutput", NULL, NULL};
	static char stdoutStr[] = "stdout";
	static char stderrStr[]= "stderr";
	char *result;

	if (newline) {
	    int len = strlen(argv[i]);
	    newargv[2] = ckalloc(len+2);
	    memcpy(newargv[2], argv[i], len);
	    newargv[2][len] = '\n';
	    newargv[2][len+1] = 0;
	} else {
	    newargv[2] = argv[i];
	}
	if (fileno(f) == fileno(stdout)) {
	    newargv[1] = stdoutStr;
	} else {
	    newargv[1] = stderrStr;
	}
	result = Tcl_Merge(3, newargv);
	Tcl_Eval(info->consoleInterp, result);
	ckfree(result);
	if (newline) {
	    ckfree(newargv[2]);
	}
    } else {
	clearerr(f);
	fputs(argv[i], f);
	if (newline) {
	    fputc('\n', f);
	}
	if (ferror(f)) {
	    Tcl_AppendResult(interp, "error writing \"", fileId,
		    "\": ", Tcl_PosixError(interp), (char *) NULL);
	    return TCL_ERROR;
	}
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ConsoleEventProc --
 *
 *	This event procedure is registered on the main window of the
 *	slave interpreter.  If the user or a running script causes the
 *	main window to be destroyed, then we need to inform the console
 *	interpreter by invoking "tkConsoleExit".
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Invokes the "tkConsoleExit" procedure in the console interp.
 *
 *----------------------------------------------------------------------
 */

static void
ConsoleEventProc(clientData, eventPtr)
    ClientData clientData;
    XEvent *eventPtr;
{
    ConsoleInfo *info = (ConsoleInfo *) clientData;
    if (eventPtr->type == DestroyNotify) {
	Tcl_Eval(info->consoleInterp, "tkConsoleExit");
    }
}

/*
 *----------------------------------------------------------------------
 *
 * TkConsolePrint --
 *
 *	Prints to the give text to the console.  Given the main interp
 *	this functions find the appropiate console interp and forwards
 *	the text to be added to that console.
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
TkConsolePrint(interp, buffer, size)
    Tcl_Interp *interp;		/* Main interpreter. */
    char *buffer;		/* Text buffer. */
    long size;			/* Size of text buffer. */
{
    Tcl_DString command, output;
    Tcl_CmdInfo cmdInfo;
    char *cmd = "tkConsoleOutput stdout ";
    ConsoleInfo *info;
    int result;

    if (interp == NULL) {
	return;
    }
    
    result = Tcl_GetCommandInfo(interp, "console", &cmdInfo);
    if (result == 0) {
	return;
    }
    info = (ConsoleInfo *) cmdInfo.clientData;
    
    Tcl_DStringInit(&output);
    Tcl_DStringAppend(&output, buffer, size);

    Tcl_DStringInit(&command);
    Tcl_DStringAppend(&command, cmd, strlen(cmd));
    Tcl_DStringAppendElement(&command, output.string);

    Tcl_Eval(info->consoleInterp, command.string);
    
    Tcl_DStringFree(&command);
    Tcl_DStringFree(&output);
}
