/*
 * tclWinConsole.c --
 *
 *	Windows console window class definition.
 *
 * Copyright (c) 1994 Software Research Associates, Inc.
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tclInt.h"
#include "tclPort.h"
#include "tclWinConsole.h"
#include <windowsx.h>
#include <string.h>

/*
 * Predefined control identifiers.
 */
#define IDC_EDIT	1

/*
 * Initial screen size.
 */

#define INIT_SCREEN_CX	80
#define INIT_SCREEN_CY	25

static HWND hwndEdit;		/* Handle for edit control. */
static char szAppName[] = "Tclsh";
static int cxFrame, cyFrame, cyCaption, cxVScroll;
static Tcl_DString command; 	/* Used to buffer incomplete commands. */

char cmdBuf[256];		/* Line buffer for commands */

WNDPROC oldEditProc;		/* Pointer to system Edit control procedure */

/*
 * Forward references for procedures defined later in this file:
 */

static void 		DisplayString _ANSI_ARGS_((char *str, int newline));
static LRESULT APIENTRY TerminalProc _ANSI_ARGS_((HWND, UINT, WPARAM, LPARAM));
static LRESULT APIENTRY EditProc _ANSI_ARGS_((HWND, UINT, WPARAM, LPARAM));
static int		TerminalPutsCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));


/*
 *----------------------------------------------------------------------
 *
 * RegisterTerminalClass --
 *
 *	Creates the application class for the console window.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The global window class "Tclsh" is created.
 *
 *----------------------------------------------------------------------
 */

BOOL
RegisterTerminalClass(hInstance)
    HINSTANCE hInstance;
{		    
    WNDCLASS wndclass;

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = TerminalProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = sizeof(Tcl_Interp*);
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = "Terminal";

    return RegisterClass(&wndclass);
}

/*
 *----------------------------------------------------------------------
 *
 * CreateTerminal --
 *
 *	Creates an instance of the Tclsh console window.
 *
 * Results:
 *	A Window handle for the newly created instance.
 *
 * Side effects:
 *	Creates a new window instance with a pointer to the
 *	interpreter stored in the window instance data.
 *
 *----------------------------------------------------------------------
 */

HWND
CreateTerminal(hInstance, interp)
    HINSTANCE hInstance;
    Tcl_Interp *interp;
{
    HDC hdc;
    TEXTMETRIC tm;
    HWND hwnd;

    Tcl_DStringInit(&command);
    hdc = GetDC((HWND)NULL);
    SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));
    GetTextMetrics(hdc, &tm);
    ReleaseDC((HWND) NULL, hdc);

    cxFrame = GetSystemMetrics(SM_CXFRAME);
    cyFrame = GetSystemMetrics(SM_CYFRAME);
    cyCaption = GetSystemMetrics(SM_CYCAPTION);
    cxVScroll = GetSystemMetrics(SM_CXVSCROLL);

    hwnd = CreateWindow("Terminal", "Tclsh Console", WS_OVERLAPPEDWINDOW,
	    CW_USEDEFAULT, CW_USEDEFAULT,
	    (tm.tmAveCharWidth * INIT_SCREEN_CX) + (cxFrame * 2) + cxVScroll,
	    (tm.tmHeight * INIT_SCREEN_CY) + cyCaption + (cyFrame * 2), 
	    NULL, NULL, hInstance, NULL);
    if (hwnd != NULL) {
	SetWindowLong(hwnd, GWL_USERDATA, (LONG)interp);
	Tcl_CreateCommand(interp, "puts", TerminalPutsCmd, NULL, NULL);
    }
    return hwnd;
}

/*
 *----------------------------------------------------------------------
 *
 * TerminalProc --
 *
 *	Window procedure for the Tclsh "Terminal" class.
 *
 * Results:
 *	The usual Window procedure values.
 *
 * Side effects:
 *	On window creation, it creates an edit child window.  Most
 *	of the messages are forwarded to the child.
 *
 *----------------------------------------------------------------------
 */

static LRESULT CALLBACK
TerminalProc(hwnd, message, wParam, lParam)
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
{
    switch(message) {
    case WM_CREATE:
	hwndEdit = CreateWindow("edit", NULL,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | ES_LEFT |
		ES_MULTILINE | ES_AUTOVSCROLL,
		0, 0, 0, 0, hwnd, (HMENU) IDC_EDIT,
		GetWindowInstance(hwnd), NULL);
	oldEditProc = SubclassWindow(hwndEdit, EditProc);
	SendMessage (hwndEdit, WM_CHAR, '%', 1L);
	SendMessage (hwndEdit, WM_CHAR, ' ', 1L);
	return 0;

    case WM_COMMAND:
	if (GET_WM_COMMAND_ID(wParam,lParam) == IDC_EDIT) {
	    switch (GET_WM_COMMAND_CMD(wParam, lParam)) {
	    case LBN_DBLCLK: {
		int line, length, offset;
		char *cmd;

		/*
		 * Get line containing cursor.
		 */

		line = Edit_LineFromChar(hwndEdit,
			Edit_LineIndex(hwndEdit, -1));
		length = Edit_GetLine(hwndEdit, line, cmdBuf,
			sizeof(cmdBuf)-1);
		cmdBuf[length] = '\n';
		cmdBuf[length+1] = '\0';
		offset = cmdBuf[0] == '%' ? 1 : 0;
		cmd = Tcl_DStringAppend(&command, cmdBuf + offset, -1);
		DisplayString("", 1);
		if (Tcl_CommandComplete(cmd)) {
		    Tcl_Interp* interp = (Tcl_Interp*) GetWindowLong(hwnd,
			    GWL_USERDATA);
		    Tcl_RecordAndEval(interp, cmd, 0);
		    Tcl_DStringFree(&command);
		    if (interp->result != NULL && *interp->result != '\0') {
			DisplayString(interp->result, 1);
		    }
		    DisplayString("% ", 0);
		}
		break;
	    }
	    case EN_ERRSPACE:
		MessageBox(hwnd, "Edit control out of space.",
			szAppName, MB_OK | MB_ICONSTOP);
		break;
	    }
	}
	return 0;

    case WM_SETFOCUS:
	SetFocus(hwndEdit);
	return 0;
        
    case WM_SIZE:
	MoveWindow(hwndEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        return 0;

    case WM_CLOSE:
	if (MessageBox(hwnd, "Do you really want to exit?",
		szAppName, MB_YESNO | MB_ICONQUESTION) == IDYES) {
	    Tcl_Interp* interp = (Tcl_Interp*) GetWindowLong(hwnd,
		    GWL_USERDATA);
	    Tcl_Eval(interp, "exit");
	    exit(1);
	}
	return 0 ;

    }   	    	
    return DefWindowProc(hwnd, message, wParam, lParam);
}

/*
 *----------------------------------------------------------------------
 *
 * EditProc --
 *
 *	Edit subclass window procedure.
 *
 * Results:
 *	The usual Window procedure values.
 *
 * Side effects:
 *	Allows user to edit commands.  Sends a double click event to
 *	the main window when the user presses enter.
 *
 *----------------------------------------------------------------------
 */

static LRESULT CALLBACK
EditProc(hwnd, message, wParam, lParam)
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
{
    if(message == WM_CHAR && (wParam == '\r' || wParam == '\n')){
	SendMessage (GetParent(hwnd), WM_COMMAND,
		MAKEWPARAM(IDC_EDIT, LBN_DBLCLK), (LPARAM)hwnd) ;
	return 0 ;
    } else {
	return CallWindowProc(oldEditProc, hwnd, message, wParam, lParam);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * TerminalPutsCmd --
 *
 *	Replacement for Tcl "puts" command that writes output to the
 *	terminal instead of stdout/stderr.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	Same as Tcl_PutsCmd, except that it puts the output string
 *	into the terminal if the specified file is stdout.
 *
 *----------------------------------------------------------------------
 */

int
TerminalPutsCmd(clientData, interp, argc, argv)
    ClientData clientData;		/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    FILE *f;
    int i, newline;
    char *fileId;
    BOOL isStdout = FALSE;
    
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

    /* This is a hack to account for the fact that the stderr FILE stream
       pointer is not the same as the stderr handle registered in the TCL
       file arrays */
    if(!strcmp(fileId, "stdout")) {
	isStdout = TRUE;
    }
    
    if (Tcl_GetOpenFile(interp, fileId, 1, 1, &f) != TCL_OK) {
	return TCL_ERROR;
    }

    if (fileno(f) == fileno(stdout) || fileno(f) == fileno(stderr) ||
	isStdout) {
	DisplayString(argv[i], newline);
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
 * DisplayString --
 *
 *	Insert a string into the console with an optional trailing
 *	newline.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Updates the edit control.
 *
 *----------------------------------------------------------------------
 */

void
DisplayString(str, newline)
    char *str;
    int newline;
{
    char *p;
    const char *tmp;
    HGLOBAL clipmem;
    int size, lineCnt;

    tmp = str;
    for(lineCnt = 0; *tmp; tmp++) {
	if(*tmp == '\n') {
	    lineCnt++;
	}
    }
    size  = lstrlen(str) + lineCnt + 3;
    clipmem = GlobalAlloc(GHND, size);
    p = GlobalLock(clipmem);
    if(p != NULL) {
	while(*str != NULL) {
	    if(*str == '\n')
		*p++ = '\r';
	    *p++ = *str++ ;
	}
	if (newline) {
	    *p++ = '\r';
	    *p++ = '\n';
	}
	*p = '\0';
	GlobalUnlock(clipmem);
	OpenClipboard(hwndEdit);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, clipmem);
	CloseClipboard();
	SendMessage(hwndEdit, WM_PASTE, 0, 0L);
    }
}
