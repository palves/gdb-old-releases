/* 
 * tkWinWm.c --
 *
 *	This module takes care of the interactions between a Tk-based
 *	application and the window manager.  Among other things, it
 *	implements the "wm" command and passes geometry information
 *	to the window manager.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tkWinInt.h"

/*
 * A data structure of the following type holds information for
 * each window manager protocol (such as WM_DELETE_WINDOW) for
 * which a handler (i.e. a Tcl command) has been defined for a
 * particular top-level window.
 */

typedef struct ProtocolHandler {
    Atom protocol;		/* Identifies the protocol. */
    struct ProtocolHandler *nextPtr;
				/* Next in list of protocol handlers for
				 * the same top-level window, or NULL for
				 * end of list. */
    Tcl_Interp *interp;		/* Interpreter in which to invoke command. */
    char command[4];		/* Tcl command to invoke when a client
				 * message for this protocol arrives. 
				 * The actual size of the structure varies
				 * to accommodate the needs of the actual
				 * command. THIS MUST BE THE LAST FIELD OF
				 * THE STRUCTURE. */
} ProtocolHandler;

#define HANDLER_SIZE(cmdLength) \
    ((unsigned) (sizeof(ProtocolHandler) - 3 + cmdLength))

/*
 * A data structure of the following type holds window-manager-related
 * information for each top-level window in an application.
 */

typedef struct TkWmInfo {
    TkWindow *winPtr;		/* Pointer to main Tk information for
				 * this window. */
    Window reparent;		/* If the window has been reparented, this
				 * gives the ID of the ancestor of the window
				 * that is a child of the root window (may
				 * not be window's immediate parent).  If
				 * the window isn't reparented, this has the
				 * value None. */
    Tk_Uid titleUid;		/* Title to display in window caption.  If
				 * NULL, use name of widget. */
    Tk_Uid iconName;		/* Name to display in icon. */
    Window master;		/* Master window for TRANSIENT_FOR property,
				 * or None. */
    XWMHints hints;		/* Various pieces of information for
				 * window manager. */
    Tk_Uid leaderName;		/* Path name of leader of window group
				 * (corresponds to hints.window_group).
				 * Note:  this field doesn't get updated
				 * if leader is destroyed. */
    Tk_Uid masterWindowName;	/* Path name of window specified as master
				 * in "wm transient" command, or NULL.
				 * Note:  this field doesn't get updated if
				 * masterWindowName is destroyed. */
    Tk_Window icon;		/* Window to use as icon for this window,
				 * or NULL. */
    Tk_Window iconFor;		/* Window for which this window is icon, or
				 * NULL if this isn't an icon for anyone. */
    int withdrawn;		/* Non-zero means window has been withdrawn. */

    /*
     * Information used to construct an XSizeHints structure for
     * the window manager:
     */

    MINMAXINFO minMaxInfo;	/* Resize limits information. */
    int sizeHintsFlags;		/* Flags word for XSizeHints structure.
				 * If the PBaseSize flag is set then the
				 * window is gridded;  otherwise it isn't
				 * gridded. */
    int minWidth, minHeight;	/* Minimum dimensions of window, in
				 * grid units, not pixels. */
    int maxWidth, maxHeight;	/* Maximum dimensions of window, in
				 * grid units, not pixels. */
    Tk_Window gridWin;		/* Identifies the window that controls
				 * gridding for this top-level, or NULL if
				 * the top-level isn't currently gridded. */
    int widthInc, heightInc;	/* Increments for size changes (# pixels
				 * per step). */
    struct {
	int x;	/* numerator */
	int y;  /* denominator */
    } minAspect, maxAspect;	/* Min/max aspect ratios for window. */
    int reqGridWidth, reqGridHeight;
				/* The dimensions of the window (in
				 * grid units) requested through
				 * the geometry manager. */
    int gravity;		/* Desired window gravity. */

    /*
     * Information used to manage the size and location of a window.
     */

    int width, height;		/* Desired dimensions of window, specified
				 * in grid units.  These values are
				 * set by the "wm geometry" command and by
				 * ConfigureNotify events (for when wm
				 * resizes window).  -1 means user hasn't
				 * requested dimensions. */
    int x, y;			/* Desired X and Y coordinates for window.
				 * These values are set by "wm geometry",
				 * plus by ConfigureNotify events (when wm
				 * moves window).  These numbers are
				 * different than the numbers stored in
				 * winPtr->changes because (a) they could be
				 * measured from the right or bottom edge
				 * of the screen (see WM_NEGATIVE_X and
				 * WM_NEGATIVE_Y flags) and (b) if the window
				 * has been reparented then they refer to the
				 * parent rather than the window itself. */
    int parentWidth, parentHeight;
				/* Width and height of reparent, in pixels
				 * *including border*.  If window hasn't been
				 * reparented then these will be the outer
				 * dimensions of the window, including
				 * border. */
    int xInParent, yInParent;	/* Offset of window within reparent,  measured
				 * from upper-left outer corner of parent's
				 * border to upper-left outer corner of child's
				 * border.  If not reparented then these are
				 * zero. */
    int configWidth, configHeight;
				/* Dimensions passed to last request that we
				 * issued to change geometry of window.  Used
				 * to eliminate redundant resize operations. */
    long style;			/* Window style of reparent. */

    /*
     * List of children of the toplevel which have private colormaps.
     */

    TkWindow **cmapList;	/* Array of window with private colormaps. */
    int cmapCount;		/* Number of windows in array. */

    /*
     * Miscellaneous information.
     */

    ProtocolHandler *protPtr;	/* First in list of protocol handlers for
				 * this window (NULL means none). */
    int cmdArgc;		/* Number of elements in cmdArgv below. */
    char **cmdArgv;		/* Array of strings to store in the
				 * WM_COMMAND property.  NULL means nothing
				 * available. */
    char *clientMachine;	/* String to store in WM_CLIENT_MACHINE
				 * property, or NULL. */
    int flags;			/* Miscellaneous flags, defined below. */
    struct TkWmInfo *nextPtr;	/* Next in list of all top-level windows. */
} WmInfo;

/*
 * Flag values for WmInfo structures:
 *
 * WM_NEVER_MAPPED -		non-zero means window has never been
 *				mapped;  need to update all info when
 *				window is first mapped.
 * WM_UPDATE_PENDING -		non-zero means a call to UpdateGeometryInfo
 *				has already been scheduled for this
 *				window;  no need to schedule another one.
 * WM_NEGATIVE_X -		non-zero means x-coordinate is measured in
 *				pixels from right edge of screen, rather
 *				than from left edge.
 * WM_NEGATIVE_Y -		non-zero means y-coordinate is measured in
 *				pixels up from bottom of screen, rather than
 *				down from top.
 * WM_SYNC_PENDING -		set to non-zero while waiting for the window
 *				manager to respond to some state change.
 * WM_ABOUT_TO_MAP -		non-zero means that the window is about to
 *				be mapped by TkWmMapWindow.  This is used
 *				by UpdateGeometryInfo to modify its behavior.
 * WM_MOVE_PENDING -		non-zero means the application has requested
 *				a new position for the window, but it hasn't
 *				been reflected through the window manager
 *				yet.
 * WM_COLORAMPS_EXPLICIT -	non-zero means the colormap windows were
 *				set explicitly via "wm colormapwindows".
 * WM_ADDED_TOPLEVEL_COLORMAP - non-zero means that when "wm colormapwindows"
 *				was called the top-level itself wasn't
 *				specified, so we added it implicitly at
 *				the end of the list.
 */

#define WM_NEVER_MAPPED			1
#define WM_UPDATE_PENDING		2
#define WM_NEGATIVE_X			4
#define WM_NEGATIVE_Y			8
#define WM_UPDATE_SIZE_HINTS		0x10
#define WM_SYNC_PENDING			0x20
#define WM_ABOUT_TO_MAP			0x100
#define WM_MOVE_PENDING			0x200
#define WM_COLORMAPS_EXPLICIT		0x400
#define WM_ADDED_TOPLEVEL_COLORMAP	0x800

/*
 * Default window style for reparented toplevel windows.
 */

#define WM_TOPLEVEL_STYLE (WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|CS_DBLCLKS)
#define WM_OVERRIDE_STYLE (WS_POPUP|WS_CLIPCHILDREN|CS_DBLCLKS)


/*
 * This module keeps a list of all top-level windows, primarily to
 * simplify the job of Tk_CoordsToWindow.
 */

static WmInfo *firstWmPtr = NULL;	/* Points to first top-level window. */


/*
 * The variable below is used to enable or disable tracing in this
 * module.  If tracing is enabled, then information is printed on
 * standard output about interesting interactions with the window
 * manager.
 */

static int wmTracing = 0;

/*
 * The following structure is the official type record for geometry
 * management of top-level windows.
 */

static void		TopLevelReqProc _ANSI_ARGS_((ClientData dummy,
			    Tk_Window tkwin));

static Tk_GeomMgr wmMgrType = {
    "wm",				/* name */
    TopLevelReqProc,			/* requestProc */
    (Tk_GeomLostSlaveProc *) NULL,	/* lostSlaveProc */
};

/*
 * Global system palette.  This value always refers to the currently
 * installed foreground logical palette.
 */

static HPALETTE systemPalette = NULL;

/*
 * Forward declarations for procedures defined in this file:
 */

static void		ConfigureEvent _ANSI_ARGS_((TkWindow *winPtr,
			    XConfigureEvent *eventPtr));
static void		DeiconifyWindow _ANSI_ARGS_((TkWindow *winPtr));
static void		IconifyWindow _ANSI_ARGS_((TkWindow *winPtr));
static int		ParseGeometry _ANSI_ARGS_((Tcl_Interp *interp,
			    char *string, TkWindow *winPtr));
static void		TopLevelEventProc _ANSI_ARGS_((ClientData clientData,
			    XEvent *eventPtr));
static void		TopLevelReqProc _ANSI_ARGS_((ClientData dummy,
			    Tk_Window tkwin));
static void		UpdateGeometryInfo _ANSI_ARGS_((
			    ClientData clientData));
static void		UpdateParentGeometry _ANSI_ARGS_((TkWindow *wmPtr,
			    RECT *rect));

/*
 *----------------------------------------------------------------------
 *
 * TkWinWmSetLimits --
 *
 *	Updates the minimum and maximum window size constraints.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Changes the values of the info pointer to reflect the current
 *	minimum and maximum size values.
 *
 *----------------------------------------------------------------------
 */

void
TkWinWmSetLimits(winPtr, info)
    TkWindow *winPtr;
    MINMAXINFO *info;
{
    register WmInfo *wmPtr;

    if (winPtr == NULL) {
	return;
    }

    wmPtr = winPtr->wmInfoPtr;
    
    /*
     * Copy latest constraint info.
     */
    
    if (wmPtr->gridWin != NULL) {
	info->ptMaxTrackSize.x = wmPtr->parentWidth +
	    (wmPtr->maxWidth - wmPtr->reqGridWidth) * wmPtr->widthInc;
	info->ptMaxTrackSize.y = wmPtr->parentHeight +
	    (wmPtr->maxHeight - wmPtr->reqGridHeight) * wmPtr->heightInc;
	info->ptMinTrackSize.x = wmPtr->parentWidth +
	    (wmPtr->minWidth - wmPtr->reqGridWidth) * wmPtr->widthInc;
	info->ptMinTrackSize.y = wmPtr->parentHeight +
	    (wmPtr->minHeight - wmPtr->reqGridHeight) * wmPtr->heightInc;
    } else {
	info->ptMaxTrackSize.x = wmPtr->maxWidth;
	info->ptMaxTrackSize.y = wmPtr->maxHeight;
	info->ptMinTrackSize.x = wmPtr->minWidth;
	info->ptMinTrackSize.y = wmPtr->minHeight;
    }
}

/*
 *--------------------------------------------------------------
 *
 * TkWmNewWindow --
 *
 *	This procedure is invoked whenever a new top-level
 *	window is created.  Its job is to initialize the WmInfo
 *	structure for the window.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	A WmInfo structure gets allocated and initialized.
 *
 *--------------------------------------------------------------
 */

void
TkWmNewWindow(winPtr)
    TkWindow *winPtr;		/* Newly-created top-level window. */
{
    register WmInfo *wmPtr;
    RECT rc;

    wmPtr = (WmInfo *) ckalloc(sizeof(WmInfo));
    winPtr->wmInfoPtr = wmPtr;
    wmPtr->winPtr = winPtr;
    wmPtr->reparent = None;
    wmPtr->titleUid = NULL;
    wmPtr->iconName = NULL;
    wmPtr->master = None;
    wmPtr->hints.flags = InputHint | StateHint;
    wmPtr->hints.input = True;
    wmPtr->hints.initial_state = NormalState;
    wmPtr->hints.icon_pixmap = None;
    wmPtr->hints.icon_window = None;
    wmPtr->hints.icon_x = wmPtr->hints.icon_y = 0;
    wmPtr->hints.icon_mask = None;
    wmPtr->hints.window_group = None;
    wmPtr->leaderName = NULL;
    wmPtr->masterWindowName = NULL;
    wmPtr->icon = NULL;
    wmPtr->iconFor = NULL;
    wmPtr->withdrawn = 0;
    wmPtr->sizeHintsFlags = 0;
    wmPtr->minWidth = wmPtr->minHeight = 1;
    wmPtr->maxWidth = DisplayWidth(winPtr->display, winPtr->screenNum);
    wmPtr->maxHeight = DisplayHeight(winPtr->display, winPtr->screenNum);
    wmPtr->gridWin = NULL;
    wmPtr->widthInc = wmPtr->heightInc = 1;
    wmPtr->minAspect.x = wmPtr->minAspect.y = 1;
    wmPtr->maxAspect.x = wmPtr->maxAspect.y = 1;
    wmPtr->reqGridWidth = wmPtr->reqGridHeight = -1;
    wmPtr->gravity = NorthWestGravity;
    wmPtr->width = -1;
    wmPtr->height = -1;
    wmPtr->style = WM_TOPLEVEL_STYLE;
    wmPtr->x = winPtr->changes.x;
    wmPtr->y = winPtr->changes.y;
    wmPtr->parentWidth = -1;
    wmPtr->parentHeight = -1;
    wmPtr->yInParent = 0;
    wmPtr->xInParent = 0;
    wmPtr->yInParent = 0;
    wmPtr->cmapList = NULL;
    wmPtr->cmapCount = 0;

    wmPtr->configWidth = -1;
    wmPtr->configHeight = -1;
    wmPtr->protPtr = NULL;
    wmPtr->cmdArgv = NULL;
    wmPtr->clientMachine = NULL;
    wmPtr->flags = WM_NEVER_MAPPED;
    wmPtr->nextPtr = firstWmPtr;
    firstWmPtr = wmPtr;

    /*
     * Tk must monitor structure events for top-level windows, in order
     * to detect size and position changes caused by window managers.
     */

    Tk_CreateEventHandler((Tk_Window) winPtr, StructureNotifyMask,
	    TopLevelEventProc, (ClientData) winPtr);

    /*
     * Arrange for geometry requests to be reflected from the window
     * to the window manager.
     */

    Tk_ManageGeometry((Tk_Window) winPtr, &wmMgrType, (ClientData) 0);
}

/*
 *--------------------------------------------------------------
 *
 * TkWmMapWindow --
 *
 *	This procedure is invoked to map a top-level window.  This
 *	module gets a chance to update all window-manager-related
 *	information in properties before the window manager sees
 *	the map event and checks the properties.  It also gets to
 *	decide whether or not to even map the window after all.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Properties of winPtr may get updated to provide up-to-date
 *	information to the window manager.  The window may also get
 *	mapped, but it may not be if this procedure decides that
 *	isn't appropriate (e.g. because the window is withdrawn).
 *
 *--------------------------------------------------------------
 */

void
TkWmMapWindow(winPtr)
    TkWindow *winPtr;		/* Top-level window that's about to
				 * be mapped. */
{
    Window window;
    register WmInfo *wmPtr = winPtr->wmInfoPtr;
    XTextProperty textProp;
    Tk_Event e;
    XEvent* event;
    HWND child = TkWinGetHWND(winPtr->window);

    e.type = TK_X_EVENTS;
    event = &e.window.event;

    if (wmPtr->flags & WM_NEVER_MAPPED) {
	int x, y;
	RECT rc;
	TkWinDrawable *parentPtr;
	HWND window;

	
	wmPtr->flags &= ~WM_NEVER_MAPPED;

	/*
	 * This is the first time this window has ever been mapped.
	 * Store all the window-manager-related information for the
	 * window.
	 */

	if (wmPtr->titleUid == NULL) {
	    wmPtr->titleUid = winPtr->nameUid;
	}

	/*
	 * We need to create the decorative window and reparent the Tk toplevel
	 * frame.  Override redirect windows get created as undecorated
	 * popups.
	 */
	
	if (!winPtr->atts.override_redirect) {
	    wmPtr->style = WM_TOPLEVEL_STYLE;
	} else {
	    wmPtr->style = WM_OVERRIDE_STYLE;
	}
	UpdateParentGeometry(winPtr, &rc);
	if (wmPtr->sizeHintsFlags & (USPosition | PPosition)) {
	    if (wmPtr->flags & WM_NEGATIVE_X) {
		x = DisplayWidth(winPtr->display, winPtr->screenNum)
		    - wmPtr->x - wmPtr->parentWidth;
	    } else {
		x =  wmPtr->x;
	    }
	    if (wmPtr->flags & WM_NEGATIVE_Y) {
		y = DisplayHeight(winPtr->display, winPtr->screenNum)
		    - wmPtr->y - wmPtr->parentHeight;
	    } else {
		y =  wmPtr->y;
	    }
	} else {
	    x = CW_USEDEFAULT;
	    y = CW_USEDEFAULT;
	}
	parentPtr = (TkWinDrawable *) ckalloc(sizeof(TkWinDrawable));
	parentPtr->type = TWD_WM_WINDOW;
	parentPtr->window.winPtr = winPtr;

	window  = CreateWindow("TopLevel", wmPtr->titleUid, wmPtr->style,
		x, y, wmPtr->parentWidth, wmPtr->parentHeight,
		NULL, NULL, TkWinGetAppInstance(), parentPtr);
	parentPtr->window.handle = window;
	GetWindowRect(window, &rc);

	wmPtr->reparent = (Window)parentPtr;
	SetParent(child, window);

	/*
	 * We have to change the style of the window to make it a child
	 * window.
	 */

	SetWindowLong(child, GWL_STYLE,
		WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	/*
	 * Generate a reparent event.
	 */

	event->type = ReparentNotify;
	winPtr->display->request++;
	event->xreparent.serial = winPtr->display->request;
	event->xreparent.send_event = False;
	event->xreparent.display = winPtr->display;
	event->xreparent.event = winPtr->window;
	event->xreparent.window = winPtr->window;
	event->xreparent.parent = wmPtr->reparent;
	event->xreparent.x = 0;
	event->xreparent.y = 0;
	event->xreparent.override_redirect = winPtr->atts.override_redirect;
	Tk_QueueEvent(&e, TK_QUEUE_TAIL);
    }
    if (wmPtr->hints.initial_state == WithdrawnState) {
	return;
    }
    if (wmPtr->iconFor != NULL) {
	/*
	 * This window is an icon for somebody else.  Make sure that
	 * the geometry is up-to-date, then return without mapping
	 * the window.
	 */

	if (wmPtr->flags & WM_UPDATE_PENDING) {
	    Tk_CancelIdleCall(UpdateGeometryInfo, (ClientData) winPtr);
	}
	UpdateGeometryInfo((ClientData) winPtr);
	return;
    }
    wmPtr->flags |= WM_ABOUT_TO_MAP;
    if (wmPtr->flags & WM_UPDATE_PENDING) {
	Tk_CancelIdleCall(UpdateGeometryInfo, (ClientData) winPtr);
    }
    UpdateGeometryInfo((ClientData) winPtr);
    wmPtr->flags &= ~WM_ABOUT_TO_MAP;

    /*
     * Map the window in either the iconified or normal state.  Note that
     * we only send a map event if the window is in the normal state.
     */

	
    if (wmPtr->hints.initial_state == IconicState) {
	wmPtr->flags |= WM_SYNC_PENDING;
	ShowWindow(TkWinGetHWND(wmPtr->reparent), SW_SHOWMINNOACTIVE);
	wmPtr->flags &= ~WM_SYNC_PENDING;

	/*
	 * Send an unmap event if we are iconifying a currently displayed
	 * window.
	 */

	if (!wmPtr->withdrawn) {
	    XUnmapWindow(winPtr->display, winPtr->window);
	}

    } else {
	XMapWindow(winPtr->display, winPtr->window);
	wmPtr->flags |= WM_SYNC_PENDING;
	if (IsZoomed(TkWinGetHWND(wmPtr->reparent))) {
	    ShowWindow(TkWinGetHWND(wmPtr->reparent), SW_SHOWMAXIMIZED);
	} else {
	    ShowWindow(TkWinGetHWND(wmPtr->reparent), SW_SHOWNORMAL);
	}
	wmPtr->flags &= ~WM_SYNC_PENDING;
    }
}

/*
 *--------------------------------------------------------------
 *
 * TkWmUnmapWindow --
 *
 *	This procedure is invoked to unmap a top-level window.  The
 *	only thing it does special is unmap the decorative frame before
 *	unmapping the toplevel window.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Unmaps the decorative frame and the window.
 *
 *--------------------------------------------------------------
 */

void
TkWmUnmapWindow(winPtr)
    TkWindow *winPtr;		/* Top-level window that's about to
				 * be unmapped. */
{
    Tk_Event e;
    register WmInfo *wmPtr = winPtr->wmInfoPtr;

    wmPtr->flags |= WM_SYNC_PENDING;
    ShowWindow(TkWinGetHWND(wmPtr->reparent), SW_HIDE);
    wmPtr->flags &= ~WM_SYNC_PENDING;
    winPtr->flags &= ~TK_MAPPED;
    XUnmapWindow(winPtr->display, winPtr->window);
}

/*
 *--------------------------------------------------------------
 *
 * TkWmDeadWindow --
 *
 *	This procedure is invoked when a top-level window is
 *	about to be deleted.  It cleans up the wm-related data
 *	structures for the window.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The WmInfo structure for winPtr gets freed up.
 *
 *--------------------------------------------------------------
 */

void
TkWmDeadWindow(winPtr)
    TkWindow *winPtr;		/* Top-level window that's being deleted. */
{
    register WmInfo *wmPtr = winPtr->wmInfoPtr;
    WmInfo *wmPtr2;

    if (wmPtr == NULL) {
	return;
    }

    /*
     * Clean up event related window info.
     */

    if (firstWmPtr == wmPtr) {
	firstWmPtr = wmPtr->nextPtr;
    } else {
	register WmInfo *prevPtr;

	for (prevPtr = firstWmPtr; ; prevPtr = prevPtr->nextPtr) {
	    if (prevPtr == NULL) {
		panic("couldn't unlink window in TkWmDeadWindow");
	    }
	    if (prevPtr->nextPtr == wmPtr) {
		prevPtr->nextPtr = wmPtr->nextPtr;
		break;
	    }
	}
    }
    if (wmPtr->hints.flags & IconPixmapHint) {
	Tk_FreeBitmap(winPtr->display, wmPtr->hints.icon_pixmap);
    }
    if (wmPtr->hints.flags & IconMaskHint) {
	Tk_FreeBitmap(winPtr->display, wmPtr->hints.icon_mask);
    }
    if (wmPtr->icon != NULL) {
	wmPtr2 = ((TkWindow *) wmPtr->icon)->wmInfoPtr;
	wmPtr2->iconFor = NULL;
	wmPtr2->withdrawn = 1;
    }
    if (wmPtr->iconFor != NULL) {
	wmPtr2 = ((TkWindow *) wmPtr->iconFor)->wmInfoPtr;
	wmPtr2->icon = NULL;
	wmPtr2->hints.flags &= ~IconWindowHint;
    }
    while (wmPtr->protPtr != NULL) {
	ProtocolHandler *protPtr;

	protPtr = wmPtr->protPtr;
	wmPtr->protPtr = protPtr->nextPtr;
	Tk_EventuallyFree((ClientData) protPtr, TCL_DYNAMIC);
    }
    if (wmPtr->cmdArgv != NULL) {
	ckfree((char *) wmPtr->cmdArgv);
    }
    if (wmPtr->clientMachine != NULL) {
	ckfree((char *) wmPtr->clientMachine);
    }
    if (wmPtr->flags & WM_UPDATE_PENDING) {
	Tk_CancelIdleCall(UpdateGeometryInfo, (ClientData) winPtr);
    }

    /*
     * Destroy the decorative frame window.  Note that the back pointer
     * to the child window must be cleared before calling DestroyWindow to
     * avoid generating events on a window that is already dead.
     */

    if (wmPtr->reparent != None) {
	((TkWinDrawable *) wmPtr->reparent)->window.winPtr = NULL;
	DestroyWindow(TkWinGetHWND(wmPtr->reparent));
    }
    ckfree((char *) wmPtr);
    winPtr->wmInfoPtr = NULL;
}

/*
 *--------------------------------------------------------------
 *
 * TkWmSetClass --
 *
 *	This procedure is invoked whenever a top-level window's
 *	class is changed.  If the window has been mapped then this
 *	procedure updates the window manager property for the
 *	class.  If the window hasn't been mapped, the update is
 *	deferred until just before the first mapping.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	A window property may get updated.
 *
 *--------------------------------------------------------------
 */

void
TkWmSetClass(winPtr)
    TkWindow *winPtr;		/* Newly-created top-level window. */
{
    return;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_WmCmd --
 *
 *	This procedure is invoked to process the "wm" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tk_WmCmd(clientData, interp, argc, argv)
    ClientData clientData;	/* Main window associated with
				 * interpreter. */
    Tcl_Interp *interp;		/* Current interpreter. */
    int argc;			/* Number of arguments. */
    char **argv;		/* Argument strings. */
{
    Tk_Window tkwin = (Tk_Window) clientData;
    TkWindow *winPtr;
    register WmInfo *wmPtr;
    int c;
    size_t length;

    if (argc < 2) {
	wrongNumArgs:
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " option window ?arg ...?\"", (char *) NULL);
	return TCL_ERROR;
    }
    c = argv[1][0];
    length = strlen(argv[1]);
    if ((c == 't') && (strncmp(argv[1], "tracing", length) == 0)
	    && (length >= 3)) {
	if ((argc != 2) && (argc != 3)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " tracing ?boolean?\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 2) {
	    interp->result = (wmTracing) ? "on" : "off";
	    return TCL_OK;
	}
	return Tcl_GetBoolean(interp, argv[2], &wmTracing);
    }

    if (argc < 3) {
	goto wrongNumArgs;
    }
    winPtr = (TkWindow *) Tk_NameToWindow(interp, argv[2], tkwin);
    if (winPtr == NULL) {
	return TCL_ERROR;
    }
    if (!(winPtr->flags & TK_TOP_LEVEL)) {
	Tcl_AppendResult(interp, "window \"", winPtr->pathName,
		"\" isn't a top-level window", (char *) NULL);
	return TCL_ERROR;
    }
    wmPtr = winPtr->wmInfoPtr;
    if ((c == 'a') && (strncmp(argv[1], "aspect", length) == 0)) {
	int numer1, denom1, numer2, denom2;

	if ((argc != 3) && (argc != 7)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " aspect window ?minNumer minDenom ",
		    "maxNumer maxDenom?\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    if (wmPtr->sizeHintsFlags & PAspect) {
		sprintf(interp->result, "%d %d %d %d", wmPtr->minAspect.x,
			wmPtr->minAspect.y, wmPtr->maxAspect.x,
			wmPtr->maxAspect.y);
	    }
	    return TCL_OK;
	}
	if (*argv[3] == '\0') {
	    wmPtr->sizeHintsFlags &= ~PAspect;
	} else {
	    if ((Tcl_GetInt(interp, argv[3], &numer1) != TCL_OK)
		    || (Tcl_GetInt(interp, argv[4], &denom1) != TCL_OK)
		    || (Tcl_GetInt(interp, argv[5], &numer2) != TCL_OK)
		    || (Tcl_GetInt(interp, argv[6], &denom2) != TCL_OK)) {
		return TCL_ERROR;
	    }
	    if ((numer1 <= 0) || (denom1 <= 0) || (numer2 <= 0) ||
		    (denom2 <= 0)) {
		interp->result = "aspect number can't be <= 0";
		return TCL_ERROR;
	    }
	    wmPtr->minAspect.x = numer1;
	    wmPtr->minAspect.y = denom1;
	    wmPtr->maxAspect.x = numer2;
	    wmPtr->maxAspect.y = denom2;
	    wmPtr->sizeHintsFlags |= PAspect;
	}
	goto updateGeom;
    } else if ((c == 'c') && (strncmp(argv[1], "client", length) == 0)
	    && (length >= 2)) {
	if ((argc != 3) && (argc != 4)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " client window ?name?\"",
		    (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    if (wmPtr->clientMachine != NULL) {
		interp->result = wmPtr->clientMachine;
	    }
	    return TCL_OK;
	}
	if (argv[3][0] == 0) {
	    if (wmPtr->clientMachine != NULL) {
		ckfree((char *) wmPtr->clientMachine);
		wmPtr->clientMachine = NULL;
		if (!(wmPtr->flags & WM_NEVER_MAPPED)) {
		    XDeleteProperty(winPtr->display, winPtr->window,
			    Tk_InternAtom((Tk_Window) winPtr,
			    "WM_CLIENT_MACHINE"));
		}
	    }
	    return TCL_OK;
	}
	if (wmPtr->clientMachine != NULL) {
	    ckfree((char *) wmPtr->clientMachine);
	}
	wmPtr->clientMachine = (char *)
		ckalloc((unsigned) (strlen(argv[3]) + 1));
	strcpy(wmPtr->clientMachine, argv[3]);
	if (!(wmPtr->flags & WM_NEVER_MAPPED)) {
	    XTextProperty textProp;
	    if (XStringListToTextProperty(&wmPtr->clientMachine, 1, &textProp)
		    != 0) {
		XSetWMClientMachine(winPtr->display, winPtr->window,
			&textProp);
		XFree((char *) textProp.value);
	    }
	}
    } else if ((c == 'c') && (strncmp(argv[1], "colormapwindows", length) == 0)
	    && (length >= 3)) {
	TkWindow **cmapList;
	TkWindow *winPtr2;
	int i, windowArgc, gotToplevel;
	char **windowArgv;

	if ((argc != 3) && (argc != 4)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " colormapwindows window ?windowList?\"",
		    (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    Tk_MakeWindowExist((Tk_Window) winPtr);
	    for (i = 0; i < wmPtr->cmapCount; i++) {
		if ((i == (wmPtr->cmapCount-1))
			&& (wmPtr->flags & WM_ADDED_TOPLEVEL_COLORMAP)) {
		    break;
		}
		Tcl_AppendElement(interp, wmPtr->cmapList[i]->pathName);
	    }
	    return TCL_OK;
	}
	if (Tcl_SplitList(interp, argv[3], &windowArgc, &windowArgv)
		!= TCL_OK) {
	    return TCL_ERROR;
	}
	cmapList = (TkWindow **) ckalloc((unsigned)
		((windowArgc+1)*sizeof(TkWindow*)));
	for (i = 0; i < windowArgc; i++) {
	    winPtr2 = (TkWindow *) Tk_NameToWindow(interp, windowArgv[i],
		    tkwin);
	    if (winPtr2 == NULL) {
		ckfree((char *) cmapList);
		ckfree((char *) windowArgv);
		return TCL_ERROR;
	    }
	    if (winPtr2 == winPtr) {
		gotToplevel = 1;
	    }
	    if (winPtr2->window == None) {
		Tk_MakeWindowExist((Tk_Window) winPtr2);
	    }
	    cmapList[i] = winPtr2;
	}
	if (!gotToplevel) {
	    wmPtr->flags |= WM_ADDED_TOPLEVEL_COLORMAP;
	    cmapList[windowArgc] = winPtr;
	    windowArgc++;
	} else {
	    wmPtr->flags &= ~WM_ADDED_TOPLEVEL_COLORMAP;
	}
	wmPtr->flags |= WM_COLORMAPS_EXPLICIT;
	if (wmPtr->cmapList != NULL) {
	    ckfree((char *)wmPtr->cmapList);
	}
	wmPtr->cmapList = cmapList;
	wmPtr->cmapCount = windowArgc;
	ckfree((char *) windowArgv);

	/*
	 * Now we need to force the updated colormaps to be installed.
	 */

	SendMessage(TkWinGetHWND(wmPtr->reparent), WM_QUERYNEWPALETTE,
		(WPARAM) NULL, (LPARAM) NULL);
	return TCL_OK;
    } else if ((c == 'c') && (strncmp(argv[1], "command", length) == 0)
	    && (length >= 3)) {
	int cmdArgc;
	char **cmdArgv;

	if ((argc != 3) && (argc != 4)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " command window ?value?\"",
		    (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    if (wmPtr->cmdArgv != NULL) {
		interp->result = Tcl_Merge(wmPtr->cmdArgc, wmPtr->cmdArgv);
		interp->freeProc = TCL_DYNAMIC;
	    }
	    return TCL_OK;
	}
	if (argv[3][0] == 0) {
	    if (wmPtr->cmdArgv != NULL) {
		ckfree((char *) wmPtr->cmdArgv);
		wmPtr->cmdArgv = NULL;
		if (!(wmPtr->flags & WM_NEVER_MAPPED)) {
		    XDeleteProperty(winPtr->display, winPtr->window,
			    Tk_InternAtom((Tk_Window) winPtr, "WM_COMMAND"));
		}
	    }
	    return TCL_OK;
	}
	if (Tcl_SplitList(interp, argv[3], &cmdArgc, &cmdArgv) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (wmPtr->cmdArgv != NULL) {
	    ckfree((char *) wmPtr->cmdArgv);
	}
	wmPtr->cmdArgc = cmdArgc;
	wmPtr->cmdArgv = cmdArgv;
	if (!(wmPtr->flags & WM_NEVER_MAPPED)) {
	    XSetCommand(winPtr->display, winPtr->window, cmdArgv, cmdArgc);
	}
    } else if ((c == 'd') && (strncmp(argv[1], "deiconify", length) == 0)) {
	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " deiconify window\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (wmPtr->iconFor != NULL) {
	    Tcl_AppendResult(interp, "can't deiconify ", argv[2],
		    ": it is an icon for ", winPtr->pathName, (char *) NULL);
	    return TCL_ERROR;
	}
	DeiconifyWindow(winPtr);
    } else if ((c == 'f') && (strncmp(argv[1], "focusmodel", length) == 0)
	    && (length >= 2)) {
	if ((argc != 3) && (argc != 4)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " focusmodel window ?active|passive?\"",
		    (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    interp->result = wmPtr->hints.input ? "passive" : "active";
	    return TCL_OK;
	}
	c = argv[3][0];
	length = strlen(argv[3]);
	if ((c == 'a') && (strncmp(argv[3], "active", length) == 0)) {
	    wmPtr->hints.input = False;
	} else if ((c == 'p') && (strncmp(argv[3], "passive", length) == 0)) {
	    wmPtr->hints.input = True;
	} else {
	    Tcl_AppendResult(interp, "bad argument \"", argv[3],
		    "\": must be active or passive", (char *) NULL);
	    return TCL_ERROR;
	}
    } else if ((c == 'f') && (strncmp(argv[1], "frame", length) == 0)
	    && (length >= 2)) {
	Window window;

	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " frame window\"", (char *) NULL);
	    return TCL_ERROR;
	}
	window = wmPtr->reparent;
	if (window == None) {
	    window = Tk_WindowId((Tk_Window) winPtr);
	}
	sprintf(interp->result, "0x%x", (unsigned int) window);
    } else if ((c == 'g') && (strncmp(argv[1], "geometry", length) == 0)
	    && (length >= 2)) {
	char xSign, ySign;
	int width, height;

	if ((argc != 3) && (argc != 4)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " geometry window ?newGeometry?\"",
		    (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    xSign = (wmPtr->flags & WM_NEGATIVE_X) ? '-' : '+';
	    ySign = (wmPtr->flags & WM_NEGATIVE_Y) ? '-' : '+';
	    if (wmPtr->gridWin != NULL) {
		width = wmPtr->reqGridWidth + (winPtr->changes.width
			- winPtr->reqWidth)/wmPtr->widthInc;
		height = wmPtr->reqGridHeight + (winPtr->changes.height
			- winPtr->reqHeight)/wmPtr->heightInc;
	    } else {
		width = winPtr->changes.width;
		height = winPtr->changes.height;
	    }
	    sprintf(interp->result, "%dx%d%c%d%c%d", width, height,
		    xSign, wmPtr->x, ySign, wmPtr->y);
	    return TCL_OK;
	}
	if (*argv[3] == '\0') {
	    wmPtr->width = -1;
	    wmPtr->height = -1;
	    goto updateGeom;
	}
	return ParseGeometry(interp, argv[3], winPtr);
    } else if ((c == 'g') && (strncmp(argv[1], "grid", length) == 0)
	    && (length >= 3)) {
	int reqWidth, reqHeight, widthInc, heightInc;

	if ((argc != 3) && (argc != 7)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " grid window ?baseWidth baseHeight ",
		    "widthInc heightInc?\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    if (wmPtr->sizeHintsFlags & PBaseSize) {
		sprintf(interp->result, "%d %d %d %d", wmPtr->reqGridWidth,
			wmPtr->reqGridHeight, wmPtr->widthInc,
			wmPtr->heightInc);
	    }
	    return TCL_OK;
	}
	if (*argv[3] == '\0') {
	    /*
	     * Turn off gridding and reset the width and height
	     * to make sense as ungridded numbers.
	     */

	    wmPtr->sizeHintsFlags &= ~(PBaseSize|PResizeInc);
	    if (wmPtr->width != -1) {
		wmPtr->width = winPtr->reqWidth + (wmPtr->width
			- wmPtr->reqGridWidth)*wmPtr->widthInc;
		wmPtr->height = winPtr->reqHeight + (wmPtr->height
			- wmPtr->reqGridHeight)*wmPtr->heightInc;
	    }
	    wmPtr->widthInc = 1;
	    wmPtr->heightInc = 1;
	} else {
	    if ((Tcl_GetInt(interp, argv[3], &reqWidth) != TCL_OK)
		    || (Tcl_GetInt(interp, argv[4], &reqHeight) != TCL_OK)
		    || (Tcl_GetInt(interp, argv[5], &widthInc) != TCL_OK)
		    || (Tcl_GetInt(interp, argv[6], &heightInc) != TCL_OK)) {
		return TCL_ERROR;
	    }
	    if (reqWidth < 0) {
		interp->result = "baseWidth can't be < 0";
		return TCL_ERROR;
	    }
	    if (reqHeight < 0) {
		interp->result = "baseHeight can't be < 0";
		return TCL_ERROR;
	    }
	    if (widthInc < 0) {
		interp->result = "widthInc can't be < 0";
		return TCL_ERROR;
	    }
	    if (heightInc < 0) {
		interp->result = "heightInc can't be < 0";
		return TCL_ERROR;
	    }
	    Tk_SetGrid((Tk_Window) winPtr, reqWidth, reqHeight, widthInc,
		    heightInc);
	}
	goto updateGeom;
    } else if ((c == 'g') && (strncmp(argv[1], "group", length) == 0)
	    && (length >= 3)) {
	Tk_Window tkwin2;

	if ((argc != 3) && (argc != 4)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " group window ?pathName?\"",
		    (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    if (wmPtr->hints.flags & WindowGroupHint) {
		interp->result = wmPtr->leaderName;
	    }
	    return TCL_OK;
	}
	if (*argv[3] == '\0') {
	    wmPtr->hints.flags &= ~WindowGroupHint;
	    wmPtr->leaderName = NULL;
	} else {
	    tkwin2 = Tk_NameToWindow(interp, argv[3], tkwin);
	    if (tkwin2 == NULL) {
		return TCL_ERROR;
	    }
	    Tk_MakeWindowExist(tkwin2);
	    wmPtr->hints.window_group = Tk_WindowId(tkwin2);
	    wmPtr->hints.flags |= WindowGroupHint;
	    wmPtr->leaderName = Tk_PathName(tkwin2);
	}
    } else if ((c == 'i') && (strncmp(argv[1], "iconbitmap", length) == 0)
	    && (length >= 5)) {
	Pixmap pixmap;

	if ((argc != 3) && (argc != 4)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " iconbitmap window ?bitmap?\"",
		    (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    if (wmPtr->hints.flags & IconPixmapHint) {
		interp->result = Tk_NameOfBitmap(winPtr->display,
			wmPtr->hints.icon_pixmap);
	    }
	    return TCL_OK;
	}
	if (*argv[3] == '\0') {
	    if (wmPtr->hints.icon_pixmap != None) {
		Tk_FreeBitmap(winPtr->display, wmPtr->hints.icon_pixmap);
	    }
	    wmPtr->hints.flags &= ~IconPixmapHint;
	} else {
	    pixmap = Tk_GetBitmap(interp, (Tk_Window) winPtr,
		    Tk_GetUid(argv[3]));
	    if (pixmap == None) {
		return TCL_ERROR;
	    }
	    wmPtr->hints.icon_pixmap = pixmap;
	    wmPtr->hints.flags |= IconPixmapHint;
	}
    } else if ((c == 'i') && (strncmp(argv[1], "iconify", length) == 0)
	    && (length >= 5)) {
	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " iconify window\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (Tk_Attributes((Tk_Window) winPtr)->override_redirect) {
	    Tcl_AppendResult(interp, "can't iconify \"", winPtr->pathName,
		    "\": override-redirect flag is set", (char *) NULL);
	    return TCL_ERROR;
	}
	if (wmPtr->master != None) {
	    Tcl_AppendResult(interp, "can't iconify \"", winPtr->pathName,
		    "\": it is a transient", (char *) NULL);
	    return TCL_ERROR;
	}
	if (wmPtr->iconFor != NULL) {
	    Tcl_AppendResult(interp, "can't iconify ", argv[2],
		    ": it is an icon for ", winPtr->pathName, (char *) NULL);
	    return TCL_ERROR;
	}
	IconifyWindow(winPtr);
    } else if ((c == 'i') && (strncmp(argv[1], "iconmask", length) == 0)
	    && (length >= 5)) {
	Pixmap pixmap;

	if ((argc != 3) && (argc != 4)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " iconmask window ?bitmap?\"",
		    (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    if (wmPtr->hints.flags & IconMaskHint) {
		interp->result = Tk_NameOfBitmap(winPtr->display,
			wmPtr->hints.icon_mask);
	    }
	    return TCL_OK;
	}
	if (*argv[3] == '\0') {
	    if (wmPtr->hints.icon_mask != None) {
		Tk_FreeBitmap(winPtr->display, wmPtr->hints.icon_mask);
	    }
	    wmPtr->hints.flags &= ~IconMaskHint;
	} else {
	    pixmap = Tk_GetBitmap(interp, tkwin, Tk_GetUid(argv[3]));
	    if (pixmap == None) {
		return TCL_ERROR;
	    }
	    wmPtr->hints.icon_mask = pixmap;
	    wmPtr->hints.flags |= IconMaskHint;
	}
    } else if ((c == 'i') && (strncmp(argv[1], "iconname", length) == 0)
	    && (length >= 5)) {
	if (argc > 4) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " iconname window ?newName?\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    interp->result = (wmPtr->iconName != NULL) ? wmPtr->iconName : "";
	    return TCL_OK;
	} else {
	    wmPtr->iconName = Tk_GetUid(argv[3]);
	    if (!(wmPtr->flags & WM_NEVER_MAPPED)) {
		XSetIconName(winPtr->display, winPtr->window, wmPtr->iconName);
	    }
	}
    } else if ((c == 'i') && (strncmp(argv[1], "iconposition", length) == 0)
	    && (length >= 5)) {
	int x, y;

	if ((argc != 3) && (argc != 5)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " iconposition window ?x y?\"",
		    (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    if (wmPtr->hints.flags & IconPositionHint) {
		sprintf(interp->result, "%d %d", wmPtr->hints.icon_x,
			wmPtr->hints.icon_y);
	    }
	    return TCL_OK;
	}
	if (*argv[3] == '\0') {
	    wmPtr->hints.flags &= ~IconPositionHint;
	} else {
	    if ((Tcl_GetInt(interp, argv[3], &x) != TCL_OK)
		    || (Tcl_GetInt(interp, argv[4], &y) != TCL_OK)){
		return TCL_ERROR;
	    }
	    wmPtr->hints.icon_x = x;
	    wmPtr->hints.icon_y = y;
	    wmPtr->hints.flags |= IconPositionHint;
	}
    } else if ((c == 'i') && (strncmp(argv[1], "iconwindow", length) == 0)
	    && (length >= 5)) {
	Tk_Window tkwin2;
	WmInfo *wmPtr2;

	if ((argc != 3) && (argc != 4)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " iconwindow window ?pathName?\"",
		    (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    if (wmPtr->icon != NULL) {
		interp->result = Tk_PathName(wmPtr->icon);
	    }
	    return TCL_OK;
	}
	if (*argv[3] == '\0') {
	    wmPtr->hints.flags &= ~IconWindowHint;
	    if (wmPtr->icon != NULL) {
		wmPtr2 = ((TkWindow *) wmPtr->icon)->wmInfoPtr;
		wmPtr2->iconFor = NULL;
		wmPtr2->withdrawn = 1;
		wmPtr2->hints.initial_state = WithdrawnState;
	    }
	    wmPtr->icon = NULL;
	} else {
	    tkwin2 = Tk_NameToWindow(interp, argv[3], tkwin);
	    if (tkwin2 == NULL) {
		return TCL_ERROR;
	    }
	    if (!Tk_IsTopLevel(tkwin2)) {
		Tcl_AppendResult(interp, "can't use ", argv[3],
			" as icon window: not at top level", (char *) NULL);
		return TCL_ERROR;
	    }
	    wmPtr2 = ((TkWindow *) tkwin2)->wmInfoPtr;
	    if (wmPtr2->iconFor != NULL) {
		Tcl_AppendResult(interp, argv[3], " is already an icon for ",
			Tk_PathName(wmPtr2->iconFor), (char *) NULL);
		return TCL_ERROR;
	    }
	    if (wmPtr->icon != NULL) {
		WmInfo *wmPtr3 = ((TkWindow *) wmPtr->icon)->wmInfoPtr;
		wmPtr3->iconFor = NULL;
		wmPtr3->withdrawn = 1;
	    }
	    Tk_MakeWindowExist(tkwin2);
	    wmPtr->hints.icon_window = Tk_WindowId(tkwin2);
	    wmPtr->hints.flags |= IconWindowHint;
	    wmPtr->icon = tkwin2;
	    wmPtr2->iconFor = (Tk_Window) winPtr;
	    if (!wmPtr2->withdrawn && !(wmPtr2->flags & WM_NEVER_MAPPED)) {
		wmPtr2->withdrawn = 0;
		if (XWithdrawWindow(Tk_Display(tkwin2), Tk_WindowId(tkwin2),
			Tk_ScreenNumber(tkwin2)) == 0) {
		    interp->result =
			    "couldn't send withdraw message to window manager";
		    return TCL_ERROR;
		}
	    }
	}
    } else if ((c == 'm') && (strncmp(argv[1], "maxsize", length) == 0)
	    && (length >= 2)) {
	int width, height;
	if ((argc != 3) && (argc != 5)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " maxsize window ?width height?\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    sprintf(interp->result, "%d %d", wmPtr->maxWidth,
		    wmPtr->maxHeight);
	    return TCL_OK;
	}
	if ((Tcl_GetInt(interp, argv[3], &width) != TCL_OK)
		|| (Tcl_GetInt(interp, argv[4], &height) != TCL_OK)) {
	    return TCL_ERROR;
	}
	wmPtr->maxWidth = width;
	wmPtr->maxHeight = height;
	goto updateGeom;
    } else if ((c == 'm') && (strncmp(argv[1], "minsize", length) == 0)
	    && (length >= 2)) {
	int width, height;
	if ((argc != 3) && (argc != 5)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " minsize window ?width height?\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    sprintf(interp->result, "%d %d", wmPtr->minWidth,
		    wmPtr->minHeight);
	    return TCL_OK;
	}
	if ((Tcl_GetInt(interp, argv[3], &width) != TCL_OK)
		|| (Tcl_GetInt(interp, argv[4], &height) != TCL_OK)) {
	    return TCL_ERROR;
	}
	wmPtr->minWidth = width;
	wmPtr->minHeight = height;
	goto updateGeom;
    } else if ((c == 'o')
	    && (strncmp(argv[1], "overrideredirect", length) == 0)) {
	int boolean;
	XSetWindowAttributes atts;

	if ((argc != 3) && (argc != 4)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " overrideredirect window ?boolean?\"",
		    (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    if (Tk_Attributes((Tk_Window) winPtr)->override_redirect) {
		interp->result = "1";
	    } else {
		interp->result = "0";
	    }
	    return TCL_OK;
	}
	if (Tcl_GetBoolean(interp, argv[3], &boolean) != TCL_OK) {
	    return TCL_ERROR;
	}
	atts.override_redirect = (boolean) ? True : False;
	Tk_ChangeWindowAttributes((Tk_Window) winPtr, CWOverrideRedirect,
		&atts);
    } else if ((c == 'p') && (strncmp(argv[1], "positionfrom", length) == 0)
	    && (length >= 2)) {
	if ((argc != 3) && (argc != 4)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " positionfrom window ?user/program?\"",
		    (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    if (wmPtr->sizeHintsFlags & USPosition) {
		interp->result = "user";
	    } else if (wmPtr->sizeHintsFlags & PPosition) {
		interp->result = "program";
	    }
	    return TCL_OK;
	}
	if (*argv[3] == '\0') {
	    wmPtr->sizeHintsFlags &= ~(USPosition|PPosition);
	} else {
	    c = argv[3][0];
	    length = strlen(argv[3]);
	    if ((c == 'u') && (strncmp(argv[3], "user", length) == 0)) {
		wmPtr->sizeHintsFlags &= ~PPosition;
		wmPtr->sizeHintsFlags |= USPosition;
	    } else if ((c == 'p') && (strncmp(argv[3], "program", length) == 0)) {
		wmPtr->sizeHintsFlags &= ~USPosition;
		wmPtr->sizeHintsFlags |= PPosition;
	    } else {
		Tcl_AppendResult(interp, "bad argument \"", argv[3],
			"\": must be program or user", (char *) NULL);
		return TCL_ERROR;
	    }
	}
	goto updateGeom;
    } else if ((c == 'p') && (strncmp(argv[1], "protocol", length) == 0)
	    && (length >= 2)) {
	register ProtocolHandler *protPtr, *prevPtr;
	Atom protocol;
	int cmdLength;

	if ((argc < 3) || (argc > 5)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " protocol window ?name? ?command?\"",
		    (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    /*
	     * Return a list of all defined protocols for the window.
	     */
	    for (protPtr = wmPtr->protPtr; protPtr != NULL;
		    protPtr = protPtr->nextPtr) {
		Tcl_AppendElement(interp,
			Tk_GetAtomName((Tk_Window) winPtr, protPtr->protocol));
	    }
	    return TCL_OK;
	}
	protocol = Tk_InternAtom((Tk_Window) winPtr, argv[3]);
	if (argc == 4) {
	    /*
	     * Return the command to handle a given protocol.
	     */
	    for (protPtr = wmPtr->protPtr; protPtr != NULL;
		    protPtr = protPtr->nextPtr) {
		if (protPtr->protocol == protocol) {
		    interp->result = protPtr->command;
		    return TCL_OK;
		}
	    }
	    return TCL_OK;
	}

	/*
	 * Delete any current protocol handler, then create a new
	 * one with the specified command, unless the command is
	 * empty.
	 */

	for (protPtr = wmPtr->protPtr, prevPtr = NULL; protPtr != NULL;
		prevPtr = protPtr, protPtr = protPtr->nextPtr) {
	    if (protPtr->protocol == protocol) {
		if (prevPtr == NULL) {
		    wmPtr->protPtr = protPtr->nextPtr;
		} else {
		    prevPtr->nextPtr = protPtr->nextPtr;
		}
		Tk_EventuallyFree((ClientData) protPtr, TCL_DYNAMIC);
		break;
	    }
	}
	cmdLength = strlen(argv[4]);
	if (cmdLength > 0) {
	    protPtr = (ProtocolHandler *) ckalloc(HANDLER_SIZE(cmdLength));
	    protPtr->protocol = protocol;
	    protPtr->nextPtr = wmPtr->protPtr;
	    wmPtr->protPtr = protPtr;
	    protPtr->interp = interp;
	    strcpy(protPtr->command, argv[4]);
	}
    } else if ((c == 's') && (strncmp(argv[1], "sizefrom", length) == 0)
	    && (length >= 2)) {
	if ((argc != 3) && (argc != 4)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " sizefrom window ?user|program?\"",
		    (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    if (wmPtr->sizeHintsFlags & USSize) {
		interp->result = "user";
	    } else if (wmPtr->sizeHintsFlags & PSize) {
		interp->result = "program";
	    }
	    return TCL_OK;
	}
	if (*argv[3] == '\0') {
	    wmPtr->sizeHintsFlags &= ~(USSize|PSize);
	} else {
	    c = argv[3][0];
	    length = strlen(argv[3]);
	    if ((c == 'u') && (strncmp(argv[3], "user", length) == 0)) {
		wmPtr->sizeHintsFlags &= ~PSize;
		wmPtr->sizeHintsFlags |= USSize;
	    } else if ((c == 'p')
		    && (strncmp(argv[3], "program", length) == 0)) {
		wmPtr->sizeHintsFlags &= ~USSize;
		wmPtr->sizeHintsFlags |= PSize;
	    } else {
		Tcl_AppendResult(interp, "bad argument \"", argv[3],
			"\": must be program or user", (char *) NULL);
		return TCL_ERROR;
	    }
	}
	goto updateGeom;
    } else if ((c == 's') && (strncmp(argv[1], "state", length) == 0)
	    && (length >= 2)) {
	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " state window\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (wmPtr->iconFor != NULL) {
	    interp->result = "icon";
	} else if (wmPtr->withdrawn) {
	    interp->result = "withdrawn";
	} else if (Tk_IsMapped((Tk_Window) winPtr)
		|| ((wmPtr->flags & WM_NEVER_MAPPED)
		&& (wmPtr->hints.initial_state == NormalState))) {
	    interp->result = "normal";
	} else {
	    interp->result = "iconic";
	}
    } else if ((c == 't') && (strncmp(argv[1], "title", length) == 0)
	    && (length >= 2)) {
	if (argc > 4) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " title window ?newTitle?\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    interp->result = (wmPtr->titleUid != NULL) ? wmPtr->titleUid
		    : winPtr->nameUid;
	    return TCL_OK;
	} else {
	    wmPtr->titleUid = Tk_GetUid(argv[3]);
	    if (!(wmPtr->flags & WM_NEVER_MAPPED) && wmPtr->reparent != None) {
		SetWindowText(TkWinGetHWND(wmPtr->reparent), wmPtr->titleUid);
	    }
	}
    } else if ((c == 't') && (strncmp(argv[1], "transient", length) == 0)
	    && (length >= 3)) {
	Tk_Window master;

	if ((argc != 3) && (argc != 4)) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " transient window ?master?\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    if (wmPtr->master != None) {
		interp->result = wmPtr->masterWindowName;
	    }
	    return TCL_OK;
	}
	if (argv[3][0] == '\0') {
	    wmPtr->master = None;
	    wmPtr->masterWindowName = NULL;
	} else {
	    master = Tk_NameToWindow(interp, argv[3], tkwin);
	    if (master == NULL) {
		return TCL_ERROR;
	    }
	    Tk_MakeWindowExist(master);
	    wmPtr->master = Tk_WindowId(master);
	    wmPtr->masterWindowName = Tk_PathName(master);
	}
	if (!(wmPtr->flags & WM_NEVER_MAPPED)) {
	    XSetTransientForHint(winPtr->display, winPtr->window,
		    wmPtr->master);
	}
    } else if ((c == 'w') && (strncmp(argv[1], "withdraw", length) == 0)) {
	Window window;
	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # arguments: must be \"",
		    argv[0], " withdraw window\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (wmPtr->iconFor != NULL) {
	    Tcl_AppendResult(interp, "can't withdraw ", argv[2],
		    ": it is an icon for ", Tk_PathName(wmPtr->iconFor),
		    (char *) NULL);
	    return TCL_ERROR;
	}
	wmPtr->hints.initial_state = WithdrawnState;
	wmPtr->withdrawn = 1;
	if (wmPtr->flags & WM_NEVER_MAPPED) {
	    return TCL_OK;
	}

	TkWmUnmapWindow(winPtr);
    } else {
	Tcl_AppendResult(interp, "unknown or ambiguous option \"", argv[1],
		"\": must be aspect, client, command, deiconify, ",
		"focusmodel, frame, geometry, grid, group, iconbitmap, ",
		"iconify, iconmask, iconname, iconposition, ",
		"iconwindow, maxsize, minsize, overrideredirect, ",
		"positionfrom, protocol, sizefrom, state, title, ",
		"tracing, transient, or withdraw",
		(char *) NULL);
	return TCL_ERROR;
    }
    return TCL_OK;

    updateGeom:
    if (!(wmPtr->flags & (WM_UPDATE_PENDING|WM_NEVER_MAPPED))) {
	Tk_DoWhenIdle(UpdateGeometryInfo, (ClientData) winPtr);
	wmPtr->flags |= WM_UPDATE_PENDING;
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_SetGrid --
 *
 *	This procedure is invoked by a widget when it wishes to set a grid
 *	coordinate system that controls the size of a top-level window.
 *	It provides a C interface equivalent to the "wm grid" command and
 *	is usually asscoiated with the -setgrid option.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Grid-related information will be passed to the window manager, so
 *	that the top-level window associated with tkwin will resize on
 *	even grid units.  If some other window already controls gridding
 *	for the top-level window then this procedure call has no effect.
 *
 *----------------------------------------------------------------------
 */

void
Tk_SetGrid(tkwin, reqWidth, reqHeight, widthInc, heightInc)
    Tk_Window tkwin;		/* Token for window.  New window mgr info
				 * will be posted for the top-level window
				 * associated with this window. */
    int reqWidth;		/* Width (in grid units) corresponding to
				 * the requested geometry for tkwin. */
    int reqHeight;		/* Height (in grid units) corresponding to
				 * the requested geometry for tkwin. */
    int widthInc, heightInc;	/* Pixel increments corresponding to a
				 * change of one grid unit. */
{
    TkWindow *winPtr = (TkWindow *) tkwin;
    register WmInfo *wmPtr;

    /*
     * Find the top-level window for tkwin, plus the window manager
     * information.
     */

    while (!(winPtr->flags & TK_TOP_LEVEL)) {
	winPtr = winPtr->parentPtr;
    }
    wmPtr = winPtr->wmInfoPtr;

    if ((wmPtr->gridWin != NULL) && (wmPtr->gridWin != tkwin)) {
	return;
    }

    if ((wmPtr->reqGridWidth == reqWidth)
	    && (wmPtr->reqGridHeight == reqHeight)
	    && (wmPtr->widthInc == widthInc)
	    && (wmPtr->heightInc == heightInc)
	    && ((wmPtr->sizeHintsFlags & (PBaseSize|PResizeInc))
		    == PBaseSize|PResizeInc)) {
	return;
    }

    /*
     * If gridding was previously off, then forget about any window
     * size requests made by the user or via "wm geometry":  these are
     * in pixel units and there's no easy way to translate them to
     * grid units since the new requested size of the top-level window in
     * pixels may not yet have been registered yet (it may filter up
     * the hierarchy in DoWhenIdle handlers).
     */

    if (wmPtr->gridWin == NULL) {
	wmPtr->width = -1;
	wmPtr->height = -1;
    }

    /* 
     * Set the new gridding information, and start the process of passing
     * all of this information to the window manager.
     */

    wmPtr->gridWin = tkwin;
    wmPtr->reqGridWidth = reqWidth;
    wmPtr->reqGridHeight = reqHeight;
    wmPtr->widthInc = widthInc;
    wmPtr->heightInc = heightInc;
    wmPtr->sizeHintsFlags |= PBaseSize|PResizeInc;
    if (!(wmPtr->flags & (WM_UPDATE_PENDING|WM_NEVER_MAPPED))) {
	Tk_DoWhenIdle(UpdateGeometryInfo, (ClientData) winPtr);
	wmPtr->flags |= WM_UPDATE_PENDING;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_UnsetGrid --
 *
 *	This procedure cancels the effect of a previous call
 *	to Tk_SetGrid.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	If tkwin currently controls gridding for its top-level window,
 *	gridding is cancelled for that top-level window;  if some other
 *	window controls gridding then this procedure has no effect.
 *
 *----------------------------------------------------------------------
 */

void
Tk_UnsetGrid(tkwin)
    Tk_Window tkwin;		/* Token for window that is currently
				 * controlling gridding. */
{
    TkWindow *winPtr = (TkWindow *) tkwin;
    register WmInfo *wmPtr;

    /*
     * Find the top-level window for tkwin, plus the window manager
     * information.
     */

    while (!(winPtr->flags & TK_TOP_LEVEL)) {
	winPtr = winPtr->parentPtr;
    }
    wmPtr = winPtr->wmInfoPtr;
    if (tkwin != wmPtr->gridWin) {
	return;
    }

    wmPtr->gridWin = NULL;
    wmPtr->sizeHintsFlags &= ~(PBaseSize|PResizeInc);
    if (wmPtr->width != -1) {
	wmPtr->width = winPtr->reqWidth + (wmPtr->width
		- wmPtr->reqGridWidth)*wmPtr->widthInc;
	wmPtr->height = winPtr->reqHeight + (wmPtr->height
		- wmPtr->reqGridHeight)*wmPtr->heightInc;
    }
    wmPtr->widthInc = 1;
    wmPtr->heightInc = 1;

    if (!(wmPtr->flags & (WM_UPDATE_PENDING|WM_NEVER_MAPPED))) {
	Tk_DoWhenIdle(UpdateGeometryInfo, (ClientData) winPtr);
	wmPtr->flags |= WM_UPDATE_PENDING;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ConfigureEvent --
 *
 *	This procedure is called to handle ConfigureNotify events on
 *	top-level windows.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Information gets updated in the WmInfo structure for the window.
 *
 *----------------------------------------------------------------------
 */

static void
ConfigureEvent(winPtr, configEventPtr)
    TkWindow *winPtr;			/* Top-level window. */
    XConfigureEvent *configEventPtr;	/* Event that just occurred for
					 * winPtr. */
{
    RECT rect;
    register WmInfo *wmPtr = winPtr->wmInfoPtr;

    /* 
     * Update size information from the event.  There are a couple of
     * tricky points here:
     *
     * 1. If the user changed the size externally then set wmPtr->width
     *    and wmPtr->height just as if a "wm geometry" command had been
     *    invoked with the same information.
     * 2. However, if the size is changing in response to a request
     *    coming from us (WM_SYNC_PENDING is set), then don't set wmPtr->width
     *    or wmPtr->height if they were previously -1 (otherwise the
     *    window will stop tracking geometry manager requests).
     */

    if (((winPtr->changes.width != configEventPtr->width)
	    || (winPtr->changes.height != configEventPtr->height))
	    && !(wmPtr->flags & WM_SYNC_PENDING)){
	if (wmPtr->gridWin != NULL) {
	    wmPtr->width = wmPtr->reqGridWidth
		    + (configEventPtr->width
		    - winPtr->reqWidth)/wmPtr->widthInc;
	    if (wmPtr->width < 0) {
		wmPtr->width = 0;
	    }
	    wmPtr->height = wmPtr->reqGridHeight
		    + (configEventPtr->height
		    - winPtr->reqHeight)/wmPtr->heightInc;
	    if (wmPtr->height < 0) {
		wmPtr->height = 0;
	    }
	} else {
	    wmPtr->width = configEventPtr->width;
	    wmPtr->height = configEventPtr->height;
	}
	wmPtr->configWidth = configEventPtr->width;
	wmPtr->configHeight = configEventPtr->height;
    }

    if (!(wmPtr->flags & WM_SYNC_PENDING)) {
	wmPtr->x = configEventPtr->x;
	wmPtr->y = configEventPtr->y;
	wmPtr->flags &= ~(WM_NEGATIVE_X | WM_NEGATIVE_Y);
    }

    winPtr->changes.x = configEventPtr->x;
    winPtr->changes.y = configEventPtr->y;
    winPtr->changes.width = configEventPtr->width;
    winPtr->changes.height = configEventPtr->height;
    winPtr->changes.border_width = configEventPtr->border_width;
    winPtr->changes.sibling = configEventPtr->above;
    winPtr->changes.stack_mode = Above;

    UpdateParentGeometry(winPtr, &rect);
    MoveWindow(TkWinGetHWND(winPtr->window), 0, 0,
	    winPtr->changes.width, winPtr->changes.height, TRUE);
}

/*
 *----------------------------------------------------------------------
 *
 * TopLevelEventProc --
 *
 *	This procedure is invoked when a top-level (or other externally-
 *	managed window) is restructured in any way.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Tk's internal data structures for the window get modified to
 *	reflect the structural change.
 *
 *----------------------------------------------------------------------
 */

static void
TopLevelEventProc(clientData, eventPtr)
    ClientData clientData;		/* Window for which event occurred. */
    XEvent *eventPtr;			/* Event that just happened. */
{
    register TkWindow *winPtr = (TkWindow *) clientData;

    if (eventPtr->type == DestroyNotify) {
	Tk_ErrorHandler handler;

	if (!(winPtr->flags & TK_ALREADY_DEAD)) {
	    /*
	     * A top-level window was deleted externally (e.g., by the window
	     * manager).  This is probably not a good thing, but cleanup as
	     * best we can.  The error handler is needed because
	     * Tk_DestroyWindow will try to destroy the window, but of course
	     * it's already gone.
	     */
    
	    handler = Tk_CreateErrorHandler(winPtr->display, -1, -1, -1,
		    (Tk_ErrorProc *) NULL, (ClientData) NULL);
	    Tk_DestroyWindow((Tk_Window) winPtr);
	    Tk_DeleteErrorHandler(handler);
	}

	if (wmTracing) {
	    printf("TopLevelEventProc: %s deleted\n", winPtr->pathName);
	}
    } else if (eventPtr->type == ConfigureNotify) {
	/*
	 * Ignore the event if the window has never been mapped yet.
	 * Such an event occurs only in weird cases like changing the
	 * internal border width of a top-level window, which results
	 * in a synthetic Configure event.  These events are not relevant
	 * to us, and if we process them confusion may result (e.g. we
	 * may conclude erroneously that the user repositioned or resized
	 * the window).
	 */

	if (!(winPtr->wmInfoPtr->flags & WM_NEVER_MAPPED)) {
	    ConfigureEvent(winPtr, &eventPtr->xconfigure);
	}
    } else if (eventPtr->type == MapNotify) {
	winPtr->flags |= TK_MAPPED;
	if (wmTracing) {
	    printf("TopLevelEventProc: %s mapped\n", winPtr->pathName);
	}
    } else if (eventPtr->type == UnmapNotify) {
	winPtr->flags &= ~TK_MAPPED;
	if (wmTracing) {
	    printf("TopLevelEventProc: %s unmapped\n", winPtr->pathName);
	}
    }
}

/*
 *----------------------------------------------------------------------
 *
 * TopLevelReqProc --
 *
 *	This procedure is invoked by the geometry manager whenever
 *	the requested size for a top-level window is changed.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Arrange for the window to be resized to satisfy the request
 *	(this happens as a when-idle action).
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
static void
TopLevelReqProc(dummy, tkwin)
    ClientData dummy;			/* Not used. */
    Tk_Window tkwin;			/* Information about window. */
{
    TkWindow *winPtr = (TkWindow *) tkwin;
    WmInfo *wmPtr;

    wmPtr = winPtr->wmInfoPtr;
    if (!(wmPtr->flags & (WM_UPDATE_PENDING|WM_NEVER_MAPPED))) {
	Tk_DoWhenIdle(UpdateGeometryInfo, (ClientData) winPtr);
	wmPtr->flags |= WM_UPDATE_PENDING;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * UpdateGeometryInfo --
 *
 *	This procedure is invoked when a top-level window is first
 *	mapped, and also as a when-idle procedure, to bring the
 *	geometry and/or position of a top-level window back into
 *	line with what has been requested by the user and/or widgets.
 *	This procedure doesn't return until the window manager has
 *	responded to the geometry change.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The window's size and location may change, unless the WM prevents
 *	that from happening.
 *
 *----------------------------------------------------------------------
 */

static void
UpdateGeometryInfo(clientData)
    ClientData clientData;		/* Pointer to the window's record. */
{
    register TkWindow *winPtr = (TkWindow *) clientData;
    register WmInfo *wmPtr = winPtr->wmInfoPtr;
    int x, y, width, height;
    RECT rc;

    wmPtr->flags &= ~WM_UPDATE_PENDING;

    /*
     * Compute the new size for the top-level window.  See the
     * user documentation for details on this, but the size
     * requested depends on (a) the size requested internally
     * by the window's widgets, (b) the size requested by the
     * user in a "wm geometry" command or via wm-based interactive
     * resizing (if any), and (c) whether or not the window is
     * gridded.  Don't permit sizes <= 0 because this upsets
     * the X server.
     */

    if (wmPtr->width == -1) {
	width = winPtr->reqWidth;
	height = winPtr->reqHeight;
    } else if (wmPtr->gridWin != NULL) {
	width = winPtr->reqWidth
		+ (wmPtr->width - wmPtr->reqGridWidth)*wmPtr->widthInc;
	height = winPtr->reqHeight
		+ (wmPtr->height - wmPtr->reqGridHeight)*wmPtr->heightInc;
    } else {
	width = wmPtr->width;
	height = wmPtr->height;
    }
    if (width <= 0) {
	width = 1;
    }
    if (height <= 0) {
	height = 1;
    }

    /*
     * Compute the new position for the upper-left pixel of the window's
     * decorative frame.  This is tricky, because we need to include the
     * border widths supplied by a reparented parent in this calculation,
     * but can't use the parent's current overall size since that may
     * change as a result of this code.
     */

    if (wmPtr->flags & WM_NEGATIVE_X) {
	x = DisplayWidth(winPtr->display, winPtr->screenNum) - wmPtr->x
		- (width + (wmPtr->parentWidth - winPtr->changes.width));
    } else {
	x =  wmPtr->x;
    }
    if (wmPtr->flags & WM_NEGATIVE_Y) {
	y = DisplayHeight(winPtr->display, winPtr->screenNum) - wmPtr->y
		- (height + (wmPtr->parentHeight - winPtr->changes.height));
    } else {
	y =  wmPtr->y;
    }

    /*
     * Reconfigure the window if it isn't already configured correctly.
     * A few tricky points:
     *
     * 1. Sometimes the window manager will give us a different size
     *    than we asked for (e.g. mwm has a minimum size for windows), so
     *    base the size check on what we *asked for* last time, not what we
     *    got.
     * 2. Don't move window unless a new position has been requested for
     *	  it.  This is because of "features" in some window managers (e.g.
     *    twm, as of 4/24/91) where they don't interpret coordinates
     *    according to ICCCM.  Moving a window to its current location may
     *    cause it to shift position on the screen.
     */

    if ((wmPtr->flags & WM_MOVE_PENDING)
	    || (width != wmPtr->configWidth)
	    || (height != wmPtr->configHeight)) {
	wmPtr->configWidth = width;
	wmPtr->configHeight = height;
	if (wmTracing) {
	    printf("UpdateGeometryInfo moving to %d %d, resizing to %d x %d,\n",
		    x, y, width, height);
	}
	winPtr->changes.x = x;
	winPtr->changes.y = y;
	winPtr->changes.width = width;
	winPtr->changes.height = height;
	UpdateParentGeometry(winPtr, &rc);
	    
	wmPtr->flags |= WM_SYNC_PENDING;
	MoveWindow(TkWinGetHWND(wmPtr->reparent), x, y,
		wmPtr->parentWidth, wmPtr->parentHeight, TRUE);
	wmPtr->flags &= ~WM_SYNC_PENDING;
	MoveWindow(TkWinGetHWND(winPtr->window), 0, 0,
		winPtr->changes.width, winPtr->changes.height, TRUE);
    } else {
	return;
    }
}

/*
 *--------------------------------------------------------------
 *
 * ParseGeometry --
 *
 *	This procedure parses a geometry string and updates
 *	information used to control the geometry of a top-level
 *	window.
 *
 * Results:
 *	A standard Tcl return value, plus an error message in
 *	interp->result if an error occurs.
 *
 * Side effects:
 *	The size and/or location of winPtr may change.
 *
 *--------------------------------------------------------------
 */

static int
ParseGeometry(interp, string, winPtr)
    Tcl_Interp *interp;		/* Used for error reporting. */
    char *string;		/* String containing new geometry.  Has the
				 * standard form "=wxh+x+y". */
    TkWindow *winPtr;		/* Pointer to top-level window whose
				 * geometry is to be changed. */
{
    register WmInfo *wmPtr = winPtr->wmInfoPtr;
    int x, y, width, height, flags;
    char *end;
    register char *p = string;

    /*
     * The leading "=" is optional.
     */

    if (*p == '=') {
	p++;
    }

    /*
     * Parse the width and height, if they are present.  Don't
     * actually update any of the fields of wmPtr until we've
     * successfully parsed the entire geometry string.
     */

    width = wmPtr->width;
    height = wmPtr->height;
    x = wmPtr->x;
    y = wmPtr->y;
    flags = wmPtr->flags;
    if (isdigit(UCHAR(*p))) {
	width = strtoul(p, &end, 10);
	p = end;
	if (*p != 'x') {
	    goto error;
	}
	p++;
	if (!isdigit(UCHAR(*p))) {
	    goto error;
	}
	height = strtoul(p, &end, 10);
	p = end;
    }

    /*
     * Parse the X and Y coordinates, if they are present.
     */

    if (*p != '\0') {
	flags &= ~(WM_NEGATIVE_X | WM_NEGATIVE_Y);
	if (*p == '-') {
	    flags |= WM_NEGATIVE_X;
	} else if (*p != '+') {
	    goto error;
	}
	x = strtol(p+1, &end, 10);
	p = end;
	if (*p == '-') {
	    flags |= WM_NEGATIVE_Y;
	} else if (*p != '+') {
	    goto error;
	}
	y = strtol(p+1, &end, 10);
	if (*end != '\0') {
	    goto error;
	}

	/*
	 * Assume that the geometry information came from the user,
	 * unless an explicit source has been specified.  Otherwise
	 * most window managers assume that the size hints were
	 * program-specified and they ignore them.
	 */

	if ((wmPtr->sizeHintsFlags & (USPosition|PPosition)) == 0) {
	    wmPtr->sizeHintsFlags |= USPosition;
	}
    }

    /*
     * Everything was parsed OK.  Update the fields of *wmPtr and
     * arrange for the appropriate information to be percolated out
     * to the window manager at the next idle moment.
     */

    wmPtr->width = width;
    wmPtr->height = height;
    if ((x != wmPtr->x) || (y != wmPtr->y)
	    || ((flags & (WM_NEGATIVE_X|WM_NEGATIVE_Y))
	    != (wmPtr->flags & (WM_NEGATIVE_X|WM_NEGATIVE_Y)))) {
	wmPtr->x = x;
	wmPtr->y = y;
	flags |= WM_MOVE_PENDING;
    }
    wmPtr->flags = flags;

    if (!(wmPtr->flags & (WM_UPDATE_PENDING|WM_NEVER_MAPPED))) {
	Tk_DoWhenIdle(UpdateGeometryInfo, (ClientData) winPtr);
	wmPtr->flags |= WM_UPDATE_PENDING;
    }
    return TCL_OK;

    error:
    Tcl_AppendResult(interp, "bad geometry specifier \"",
	    string, "\"", (char *) NULL);
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_GetRootCoords --
 *
 *	Given a token for a window, this procedure traces through the
 *	window's lineage to find the (virtual) root-window coordinates
 *	corresponding to point (0,0) in the window.
 *
 * Results:
 *	The locations pointed to by xPtr and yPtr are filled in with
 *	the root coordinates of the (0,0) point in tkwin.  If a virtual
 *	root window is in effect for the window, then the coordinates
 *	in the virtual root are returned.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void
Tk_GetRootCoords(tkwin, xPtr, yPtr)
    Tk_Window tkwin;		/* Token for window. */
    int *xPtr;			/* Where to store x-displacement of (0,0). */
    int *yPtr;			/* Where to store y-displacement of (0,0). */
{
    int x, y;
    register TkWindow *winPtr = (TkWindow *) tkwin;

    /*
     * Search back through this window's parents all the way to a
     * top-level window, combining the offsets of each window within
     * its parent.
     */

    x = y = 0;
    while (1) {
	x += winPtr->changes.x + winPtr->changes.border_width;
	y += winPtr->changes.y + winPtr->changes.border_width;
	if (winPtr->flags & TK_TOP_LEVEL) {
	    x += winPtr->wmInfoPtr->xInParent;
	    y += winPtr->wmInfoPtr->yInParent;
	    break;
	}
	winPtr = winPtr->parentPtr;
    }
    *xPtr = x;
    *yPtr = y;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_CoordsToWindow --
 *
 *	Given the (virtual) root coordinates of a point, this procedure
 *	returns the token for the top-most window covering that point,
 *	if there exists such a window in this application.
 *
 * Results:
 *	The return result is either a token for the window corresponding
 *	to rootX and rootY, or else NULL to indicate that there is no such
 *	window.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

Tk_Window
Tk_CoordsToWindow(rootX, rootY, tkwin)
    int rootX, rootY;		/* Coordinates of point in root window.  If
				 * a virtual-root window manager is in use,
				 * these coordinates refer to the virtual
				 * root, not the real root. */
    Tk_Window tkwin;		/* Token for any window in application;
				 * used to identify the display. */
{
    POINT pos;
    HWND hwnd;
    TkWinDrawable *twdPtr;
    TkWindow *winPtr;

    pos.x = rootX;
    pos.y = rootY;
    hwnd = WindowFromPoint(pos);

    twdPtr = TkWinGetDrawableFromHandle(hwnd);
    if (twdPtr && (twdPtr->type == TWD_WINDOW)) {
	winPtr = TkWinGetWinPtr(twdPtr);
	if (winPtr->mainPtr == ((TkWindow *) tkwin)->mainPtr) {
	    return (Tk_Window) winPtr;
	}
    }
    return NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_GetVRootGeometry --
 *
 *	This procedure returns information about the virtual root
 *	window corresponding to a particular Tk window.
 *
 * Results:
 *	The values at xPtr, yPtr, widthPtr, and heightPtr are set
 *	with the offset and dimensions of the root window corresponding
 *	to tkwin.  If tkwin is being managed by a virtual root window
 *	manager these values correspond to the virtual root window being
 *	used for tkwin;  otherwise the offsets will be 0 and the
 *	dimensions will be those of the screen.
 *
 * Side effects:
 *	Vroot window information is refreshed if it is out of date.
 *
 *----------------------------------------------------------------------
 */

void
Tk_GetVRootGeometry(tkwin, xPtr, yPtr, widthPtr, heightPtr)
    Tk_Window tkwin;		/* Window whose virtual root is to be
				 * queried. */
    int *xPtr, *yPtr;		/* Store x and y offsets of virtual root
				 * here. */
    int *widthPtr, *heightPtr;	/* Store dimensions of virtual root here. */
{
    WmInfo *wmPtr;
    TkWindow *winPtr = (TkWindow *) tkwin;

    *xPtr = 0;
    *yPtr = 0;
    *widthPtr = DisplayWidth(winPtr->display, winPtr->screenNum);
    *heightPtr = DisplayHeight(winPtr->display, winPtr->screenNum);
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_MoveToplevelWindow --
 *
 *	This procedure is called instead of Tk_MoveWindow to adjust
 *	the x-y location of a top-level window.  It delays the actual
 *	move to a later time and keeps window-manager information
 *	up-to-date with the move
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The window is eventually moved so that its upper-left corner
 *	(actually, the upper-left corner of the window's decorative
 *	frame, if there is one) is at (x,y).
 *
 *----------------------------------------------------------------------
 */

void
Tk_MoveToplevelWindow(tkwin, x, y)
    Tk_Window tkwin;		/* Window to move. */
    int x, y;			/* New location for window (within
				 * parent). */
{
    TkWindow *winPtr = (TkWindow *) tkwin;
    register WmInfo *wmPtr = winPtr->wmInfoPtr;

    if (!(winPtr->flags & TK_TOP_LEVEL)) {
	panic("Tk_MoveToplevelWindow called with non-toplevel window");
    }
    wmPtr->x = x;
    wmPtr->y = y;
    wmPtr->flags |= WM_MOVE_PENDING;
    wmPtr->flags &= ~(WM_NEGATIVE_X|WM_NEGATIVE_Y);
    if ((wmPtr->sizeHintsFlags & (USPosition|PPosition)) == 0) {
	wmPtr->sizeHintsFlags |= USPosition;
    }

    /*
     * If the window has already been mapped, must bring its geometry
     * up-to-date immediately, otherwise an event might arrive from the
     * server that would overwrite wmPtr->x and wmPtr->y and lose the
     * new position.
     */

    if (!(wmPtr->flags & WM_NEVER_MAPPED)) {
	if (wmPtr->flags & WM_UPDATE_PENDING) {
	    Tk_CancelIdleCall(UpdateGeometryInfo, (ClientData) winPtr);
	}
	UpdateGeometryInfo((ClientData) winPtr);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * TkWmProtocolEventProc --
 *
 *	This procedure is called by the Tk_HandleEvent whenever a
 *	ClientMessage event arrives whose type is "WM_PROTOCOLS".
 *	This procedure handles the message from the window manager
 *	in an appropriate fashion.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Depends on what sort of handler, if any, was set up for the
 *	protocol.
 *
 *----------------------------------------------------------------------
 */

void
TkWmProtocolEventProc(winPtr, eventPtr)
    TkWindow *winPtr;		/* Window to which the event was sent. */
    XEvent *eventPtr;		/* X event. */
{
    WmInfo *wmPtr;
    register ProtocolHandler *protPtr;
    Atom protocol;
    int result;

    wmPtr = winPtr->wmInfoPtr;
    if (wmPtr == NULL) {
	return;
    }
    protocol = (Atom) eventPtr->xclient.data.l[0];
    for (protPtr = wmPtr->protPtr; protPtr != NULL;
	    protPtr = protPtr->nextPtr) {
	if (protocol == protPtr->protocol) {
	    Tk_Preserve((ClientData) protPtr);
	    result = Tcl_GlobalEval(protPtr->interp, protPtr->command);
	    if (result != TCL_OK) {
		Tcl_AddErrorInfo(protPtr->interp, "\n    (command for \"");
		Tcl_AddErrorInfo(protPtr->interp,
			Tk_GetAtomName((Tk_Window) winPtr, protocol));
		Tcl_AddErrorInfo(protPtr->interp,
			"\" window manager protocol)");
		Tk_BackgroundError(protPtr->interp);
	    }
	    Tk_Release((ClientData) protPtr);
	    return;
	}
    }

    /*
     * No handler was present for this protocol.  If this is a
     * WM_DELETE_WINDOW message then just destroy the window.
     */

    if (protocol == Tk_InternAtom((Tk_Window) winPtr, "WM_DELETE_WINDOW")) {
	Tk_DestroyWindow((Tk_Window) winPtr);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * TkWmRestackToplevel --
 *
 *	This procedure restacks a top-level window.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	WinPtr gets restacked  as specified by aboveBelow and otherPtr.
 *	This procedure doesn't return until the restack has taken
 *	effect and the ConfigureNotify event for it has been received.
 *
 *----------------------------------------------------------------------
 */

void
TkWmRestackToplevel(winPtr, aboveBelow, otherPtr)
    TkWindow *winPtr;		/* Window to restack. */
    int aboveBelow;		/* Gives relative position for restacking;
				 * must be Above or Below. */
    TkWindow *otherPtr;		/* Window relative to which to restack;
				 * if NULL, then winPtr gets restacked
				 * above or below *all* siblings. */
{
    XWindowChanges changes;
    XWindowAttributes atts;
    unsigned int mask;
    Window window, dummy1, dummy2;
    Window *children;
    unsigned int numChildren;
    int i;
    XEvent event;
    int diff;
    Tk_ErrorHandler handler;

    changes.stack_mode = aboveBelow;
    mask = CWStackMode;
    if (winPtr->window == None) {
	Tk_MakeWindowExist((Tk_Window) winPtr);
    }
    window = (winPtr->wmInfoPtr->reparent != None)
	    ? winPtr->wmInfoPtr->reparent : winPtr->window;
    if (otherPtr != NULL) {
	if (otherPtr->window == None) {
	    Tk_MakeWindowExist((Tk_Window) otherPtr);
	}
	changes.sibling = (otherPtr->wmInfoPtr->reparent != None)
		? otherPtr->wmInfoPtr->reparent : otherPtr->window;
	mask = CWStackMode|CWSibling;
    }

    /*
     * Reconfigure the window and wait for the reconfiguration to be
     * complete.  If we don't wait, then the window may not restack
     * for a while and the application might observe it before it has
     * restacked.  Waiting for the reconfiguration is tricky if winPtr
     * has been reparented, since the window getting the event isn't
     * one that Tk owns.
     */

    XConfigureWindow(winPtr->display, window, mask, &changes);
}

/*
 *----------------------------------------------------------------------
 *
 * TkWmAddToColormapWindows --
 *
 *	This procedure is called to add a given window to the
 *	WM_COLORMAP_WINDOWS property for its top-level, if it
 *	isn't already there.  It is invoked by the Tk code that
 *	creates a new colormap, in order to make sure that colormap
 *	information is propagated to the window manager by default.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	WinPtr's window gets added to the WM_COLORMAP_WINDOWS
 *	property of its nearest top-level ancestor, unless the
 *	colormaps have been set explicitly with the
 *	"wm colormapwindows" command.
 *
 *----------------------------------------------------------------------
 */

void
TkWmAddToColormapWindows(winPtr)
    TkWindow *winPtr;		/* Window with a non-default colormap.
				 * Should not be a top-level window. */
{
    TkWindow *topPtr;
    TkWindow **oldPtr, **newPtr;
    int count, i;

    if (winPtr->window == None) {
	return;
    }

    for (topPtr = winPtr->parentPtr; ; topPtr = topPtr->parentPtr) {
	if (topPtr == NULL) {
	    /*
	     * Window is being deleted.  Skip the whole operation.
	     */

	    return;
	}
	if (topPtr->flags & TK_TOP_LEVEL) {
	    break;
	}
    }
    if (topPtr->wmInfoPtr->flags & WM_COLORMAPS_EXPLICIT) {
	return;
    }

    /*
     * Make sure that the window isn't already in the list.
     */

    count = topPtr->wmInfoPtr->cmapCount;
    oldPtr = topPtr->wmInfoPtr->cmapList;

    for (i = 0; i < count; i++) {
	if (oldPtr[i] == winPtr) {
	    return;
	}
    }

    /*
     * Make a new bigger array and use it to reset the property.
     * Automatically add the toplevel itself as the last element
     * of the list.
     */

    newPtr = (TkWindow **) ckalloc((unsigned) ((count+2)*sizeof(TkWindow*)));
    if (count > 0) {
	memcpy(newPtr, oldPtr, count * sizeof(TkWindow*));
    }
    newPtr[count] = winPtr;
    newPtr[count+1] = topPtr;
    if (oldPtr != NULL) {
	ckfree((char *) oldPtr);
    }

    topPtr->wmInfoPtr->cmapList = newPtr;
    topPtr->wmInfoPtr->cmapCount = count+2;
}

/*
 *----------------------------------------------------------------------
 *
 * UpdateParentGeometry --
 *
 *	Recalculate the shape of the reparent window.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Updates the parentWidth and parentHeight fields of the wmPtr,
 *	and returns the bounding box in rect.
 *
 *----------------------------------------------------------------------
 */

static void
UpdateParentGeometry(winPtr, rect)
    TkWindow *winPtr;
    RECT *rect;
{
    WmInfo *wmPtr = winPtr->wmInfoPtr;

    /*
     * Compute the bounding rectangle of the decorative frame.
     */

    rect->left = rect->top = 0;
    rect->right = winPtr->changes.width;
    rect->bottom = winPtr->changes.height;
    AdjustWindowRect(rect, wmPtr->style, 0);
    wmPtr->parentWidth = rect->right - rect->left;
    wmPtr->parentHeight = rect->bottom - rect->top;
    wmPtr->xInParent = - rect->left;
    wmPtr->yInParent = - rect->top;
}

/*
 *----------------------------------------------------------------------
 *
 * TkWinWmConfigure --
 *
 *	Generate a ConfigureNotify event based on the current position
 *	information.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Queues a new event.
 *
 *----------------------------------------------------------------------
 */

void
TkWinWmConfigure(winPtr, pos)
    TkWindow *winPtr;
    WINDOWPOS *pos;
{
    Tk_Event e;
    WmInfo *wmPtr;

    if (winPtr == NULL) {
	return;
    }

    wmPtr = winPtr->wmInfoPtr;

    if (IsIconic(TkWinGetHWND(wmPtr->reparent))) {
	if (wmPtr->hints.initial_state == NormalState) {
	    IconifyWindow(winPtr);
	}
    } else {
	if (wmPtr->hints.initial_state == IconicState) {
	    DeiconifyWindow(winPtr);
	}
	e.type = TK_X_EVENTS;
	e.window.event.type = ConfigureNotify;
	e.window.event.xconfigure.serial = winPtr->display->request;
	e.window.event.xconfigure.send_event = False;
	e.window.event.xconfigure.display = winPtr->display;
	e.window.event.xconfigure.event = winPtr->window;
	e.window.event.xconfigure.window = winPtr->window;
	e.window.event.xconfigure.border_width =
	    winPtr->changes.border_width;
	e.window.event.xconfigure.override_redirect =
	    winPtr->atts.override_redirect;

	if (pos->flags & SWP_NOMOVE) {
	    e.window.event.xconfigure.x = winPtr->changes.x;
	    e.window.event.xconfigure.y = winPtr->changes.y;
	} else {
	    e.window.event.xconfigure.x = pos->x;
	    e.window.event.xconfigure.y = pos->y;
	}
	if (pos->flags & SWP_NOSIZE) {
	    e.window.event.xconfigure.width = winPtr->changes.width;
	    e.window.event.xconfigure.height = winPtr->changes.height;
	} else {
	    e.window.event.xconfigure.width = pos->cx -
		(wmPtr->parentWidth - winPtr->changes.width);
	    e.window.event.xconfigure.height = pos->cy -
		(wmPtr->parentHeight - winPtr->changes.height);
	}

	/*
	 * Ignore Z order changes for now.
	 */

	if (winPtr->changes.stack_mode == Above) {
	    e.window.event.xconfigure.above = winPtr->changes.sibling;
	} else {
	    e.window.event.xconfigure.above = None;
	}
	Tk_QueueEvent(&e, TK_QUEUE_TAIL);
    }
    TkWinDoQueuedEvents();
}

/*
 *----------------------------------------------------------------------
 *
 * IconifyWindow --
 *
 *	Put a toplevel window into the iconified state.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Iconifies the window, possibly mapping it for the first time.
 *
 *----------------------------------------------------------------------
 */

void
IconifyWindow(winPtr)
    TkWindow *winPtr;		/* Window to be iconified. */
{
    WmInfo *wmPtr = winPtr->wmInfoPtr;
    wmPtr->hints.initial_state = IconicState;
    if (!(wmPtr->flags & WM_NEVER_MAPPED)) {
	if (wmPtr->withdrawn) {
	    Tk_MapWindow((Tk_Window) winPtr);
	    wmPtr->withdrawn = 0;
	} else {
	    wmPtr->flags |= WM_SYNC_PENDING;
	    CloseWindow(TkWinGetHWND(wmPtr->reparent));
	    wmPtr->flags &= ~WM_SYNC_PENDING;
	    winPtr->flags &= ~TK_MAPPED;
	    XUnmapWindow(winPtr->display, winPtr->window);
	}
    }
}

/*
 *----------------------------------------------------------------------
 *
 * DeiconifyWindow --
 *
 *	Put a toplevel window into the deiconified state.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Deiconifies the window, possibly mapping it for the first time.
 *
 *----------------------------------------------------------------------
 */

void
DeiconifyWindow(winPtr)
    TkWindow *winPtr;		/* Window to be deiconified. */
{
    WmInfo *wmPtr = winPtr->wmInfoPtr;
    wmPtr->hints.initial_state = NormalState;
    wmPtr->withdrawn = 0;
    if (!(wmPtr->flags & WM_NEVER_MAPPED)) {
	Tk_MapWindow((Tk_Window) winPtr);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * TkWinWmInstallColormaps --
 *
 *	Installs the colormaps associated with the toplevel which is
 *	currently active.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	May change the system palette and generate damage.
 *
 *----------------------------------------------------------------------
 */

int
TkWinWmInstallColormaps(hwnd, message, isForemost)
    HWND hwnd;			/* Toplevel wrapper window whose colormaps
				 * should be installed. */
    int message;		/* Either WM_PALETTECHANGED or
				 * WM_QUERYNEWPALETTE */
    int isForemost;		/* 1 if window is foremost, else 0 */
{
    int i, rval;
    HDC dc;
    HPALETTE oldPalette, primaryPalette;
    TkWinDrawable *twdPtr =
	(TkWinDrawable *) GetWindowLong(hwnd, GWL_USERDATA);
    TkWindow *winPtr = TkWinGetWinPtr(twdPtr);
    WmInfo *wmPtr;
	    
    if (winPtr == NULL) {
	return 0;
    }

    wmPtr = winPtr->wmInfoPtr;
    dc = GetDC(hwnd);
    oldPalette = NULL;

    /*
     * There are three cases when this function will be called:
     * Case 1:  a WM_PALETTECHANGED message was received and the
     *		specified window is the foremost window
     * Case 2:	a WM_PALETTECHANGED message was received and the 
     *		specified window is a background window
     * Case 3:	a WM_QUERYNEWPALETTE message was received, indicating 
     *		that the specified window is about to become the
     *		foremost window
     */

    if ((message == WM_PALETTECHANGED) && isForemost) {

	/*
	 * Case 1: this message is generated as a result of case 3, so
	 * we don't need to install the primary palette again.
	 */

	if (wmPtr->cmapCount > 1) {
	    winPtr = wmPtr->cmapList[1];
	    oldPalette = SelectPalette(dc,
		    TkWinGetPalette(winPtr->atts.colormap), TRUE);
	    if (RealizePalette(dc)) {
		UpdateColors(dc); 
	    }
	    for (i = 2; i < wmPtr->cmapCount; i++) {
		winPtr = wmPtr->cmapList[i];
		SelectPalette(dc, TkWinGetPalette(winPtr->atts.colormap),
			TRUE);
		if (RealizePalette(dc)) {
		    UpdateColors(dc);
		}
	    }
	}

    } else {
	/*
	 * Cases 2 & 3:
	 * Retrieve the primary palette from the head of the colormap list
	 * or from the toplevel if there is no list.
	 */

	if (wmPtr->cmapCount > 0) {
	    winPtr = wmPtr->cmapList[0];
	}
	primaryPalette = TkWinGetPalette(winPtr->atts.colormap);

	if (message == WM_PALETTECHANGED) {
	    /*
	     * Case 2:
	     * Install the primary and secondary palettes in the background.
	     */

	    systemPalette = NULL;
	    oldPalette = SelectPalette(dc, primaryPalette, TRUE);
	    if (RealizePalette(dc)) {
		UpdateColors(dc); 
	    }
	    for (i = 1; i < wmPtr->cmapCount; i++) {
		winPtr = wmPtr->cmapList[i];
		SelectPalette(dc, TkWinGetPalette(winPtr->atts.colormap),
			TRUE);
		if (RealizePalette(dc)) {
		    UpdateColors(dc); 
		}
	    }

	} else {
	    /*
	     * Case 3:
	     * Install the primary palette.  If the system palette
	     * was updated, then Windows will generate a WM_PALETTECHANGED
	     * message.  Otherwise, we have to synthesize one in order
	     * to ensure that the secondary palettes are installed.
	     */

	    systemPalette = primaryPalette;
	    oldPalette = SelectPalette(dc, primaryPalette, FALSE);
	    rval = RealizePalette(dc);
	    if (rval) {
		InvalidateRect(NULL, NULL, FALSE);
	    } else if (wmPtr->cmapCount > 1) {
		SelectPalette(dc, oldPalette, TRUE);
		RealizePalette(dc);
		ReleaseDC(hwnd, dc);
		SendMessage(hwnd, WM_PALETTECHANGED, (WPARAM)hwnd,
			(LPARAM)NULL);
		return 0;
	    }
	}
    }

    /*
     * Restore the default palette.
     */

    SelectPalette(dc, oldPalette, TRUE);
    RealizePalette(dc);
    ReleaseDC(hwnd, dc);
    return TRUE;
}

/*
 *----------------------------------------------------------------------
 *
 * TkWinGetSystemPalette --
 *
 *	Retrieves the currently installed foreground palette.
 *
 * Results:
 *	Returns the global foreground palette, if there is one.
 *	Otherwise, returns NULL.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

HPALETTE
TkWinGetSystemPalette()
{
    return systemPalette;
}

