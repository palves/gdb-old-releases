/* 
 * tkWinWindow.c --
 *
 *	Xlib emulation routines for Windows related to creating,
 *	displaying and destroying windows.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tkWinInt.h"

/*
 * Forward declarations for procedures defined in this file:
 */

static void 		StackWindow _ANSI_ARGS_((Window w, Window sibling,
			    int stack_mode));

/*
 *----------------------------------------------------------------------
 *
 * TkMakeWindow --
 *
 *	Creates a Windows window object based on the current attributes
 *	of the specified TkWindow.
 *
 * Results:
 *	Returns a pointer to a new TkWinDrawable cast to a Window.
 *
 * Side effects:
 *	Creates a new window.
 *
 *----------------------------------------------------------------------
 */

Window
TkMakeWindow(winPtr, parent)
    TkWindow *winPtr;
    Window parent;
{
    HWND parentWin;
    TkWinDrawable *twdPtr;
    int style;
    

    twdPtr = (TkWinDrawable*) ckalloc(sizeof(TkWinDrawable));
    if (twdPtr == NULL) {
	return None;
    }

    twdPtr->type = TWD_WINDOW;
    twdPtr->window.winPtr = winPtr;

    if (parent != None) {
	parentWin = TkWinGetHWND(parent);
	style = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    } else {
	parentWin = NULL;
	style = WS_POPUP | WS_CLIPCHILDREN;
    }

    twdPtr->window.handle = CreateWindow("Child", "", style,
	    winPtr->changes.x, winPtr->changes.y,
	    winPtr->changes.width, winPtr->changes.height,
	    parentWin, NULL, TkWinGetAppInstance(), twdPtr);

    if (twdPtr->window.handle == NULL) {
	ckfree((char *) twdPtr);
	twdPtr = NULL;
    }

    return (Window)twdPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * XDestroyWindow --
 *
 *	Destroys the given window.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Sends the WM_DESTROY message to the window and then destroys
 *	it the Win32 resources associated with the window.
 *
 *----------------------------------------------------------------------
 */

void
XDestroyWindow(display, w)
    Display* display;
    Window w;
{
    display->request++;

    DestroyWindow(TkWinGetHWND(w));
}

/*
 *----------------------------------------------------------------------
 *
 * XMapWindow --
 *
 *	Cause the given window to become visible.
 *
 * Results:
 *	None
 *
 * Side effects:
 *	Causes the window state to change, and generates a MapNotify
 *	event.
 *
 *----------------------------------------------------------------------
 */

void
XMapWindow(display, w)
    Display* display;
    Window w;
{
    Tk_Event e;
    TkWindow *winPtr = TkWinGetWinPtr(w);

    display->request++;

    ShowWindow(TkWinGetHWND(w), SW_SHOWNORMAL);

    e.type = TK_X_EVENTS;
    e.window.event.type = MapNotify;
    e.window.event.xmap.serial = display->request;
    e.window.event.xmap.send_event = False;
    e.window.event.xmap.display = display;
    e.window.event.xmap.event = winPtr->window;
    e.window.event.xmap.window = winPtr->window;
    e.window.event.xmap.override_redirect = winPtr->atts.override_redirect;
    Tk_QueueEvent(&e, TK_QUEUE_TAIL);
}

/*
 *----------------------------------------------------------------------
 *
 * XUnmapWindow --
 *
 *	Cause the given window to become invisible.
 *
 * Results:
 *	None
 *
 * Side effects:
 *	Causes the window state to change, and generates an UnmapNotify
 *	event.
 *
 *----------------------------------------------------------------------
 */

void
XUnmapWindow(display, w)
    Display* display;
    Window w;
{
    Tk_Event e;
    TkWindow *winPtr = TkWinGetWinPtr(w);

    display->request++;

    ShowWindow(TkWinGetHWND(w), SW_HIDE);

    e.type = TK_X_EVENTS;
    e.window.event.type = UnmapNotify;
    e.window.event.xunmap.serial = display->request;
    e.window.event.xunmap.send_event = False;
    e.window.event.xunmap.display = display;
    e.window.event.xunmap.event = winPtr->window;
    e.window.event.xunmap.window = winPtr->window;
    e.window.event.xunmap.from_configure = False;
    Tk_QueueEvent(&e, TK_QUEUE_TAIL);
}

/*
 *----------------------------------------------------------------------
 *
 * XMoveResizeWindow --
 *
 *	Move and resize a window relative to its parent.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Repositions and resizes the specified window.
 *
 *----------------------------------------------------------------------
 */

void
XMoveResizeWindow(display, w, x, y, width, height)
    Display* display;
    Window w;
    int x;			/* Position relative to parent. */
    int y;
    unsigned int width;
    unsigned int height;
{
    display->request++;
    MoveWindow(TkWinGetHWND(w), x, y, width, height, TRUE);
}

/*
 *----------------------------------------------------------------------
 *
 * XMoveWindow --
 *
 *	Move a window relative to its parent.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Repositions the specified window.
 *
 *----------------------------------------------------------------------
 */

void
XMoveWindow(display, w, x, y)
    Display* display;
    Window w;
    int x;
    int y;
{
    TkWindow *winPtr = TkWinGetWinPtr(w);

    display->request++;

    MoveWindow(TkWinGetHWND(w), x, y, winPtr->changes.width,
	    winPtr->changes.height, TRUE);
}

/*
 *----------------------------------------------------------------------
 *
 * XResizeWindow --
 *
 *	Resize a window.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Resizes the specified window.
 *
 *----------------------------------------------------------------------
 */

void
XResizeWindow(display, w, width, height)
    Display* display;
    Window w;
    unsigned int width;
    unsigned int height;
{
    TkWindow *winPtr = TkWinGetWinPtr(w);

    display->request++;

    MoveWindow(TkWinGetHWND(w), winPtr->changes.x, winPtr->changes.y, width,
	    height, TRUE);
}

/*
 *----------------------------------------------------------------------
 *
 * XRaiseWindow --
 *
 *	Change the stacking order of a window.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Changes the stacking order of the specified window.
 *
 *----------------------------------------------------------------------
 */

void
XRaiseWindow(display, w)
    Display* display;
    Window w;
{
    HWND window = TkWinGetHWND(w);

    display->request++;
    SetWindowPos(window, HWND_TOPMOST, 0, 0, 0, 0,
	    SWP_NOMOVE | SWP_NOSIZE);
}

/*
 *----------------------------------------------------------------------
 *
 * XConfigureWindow --
 *
 *	Change the size, position, stacking, or border of the specified
 *	window.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Changes the attributes of the specified window.  Note that we
 *	ignore the passed in values and use the values stored in the
 *	TkWindow data structure.
 *
 *----------------------------------------------------------------------
 */

void
XConfigureWindow(display, w, value_mask, values)
    Display* display;
    Window w;
    unsigned int value_mask;
    XWindowChanges* values;
{
    TkWindow *winPtr = TkWinGetWinPtr(w);
    HWND window = TkWinGetHWND(w);
    HWND insertAfter;

    display->request++;

    /*
     * Change the shape and/or position of the window.
     */

    if (value_mask & (CWX|CWY|CWWidth|CWHeight)) {
	MoveWindow(window, winPtr->changes.x, winPtr->changes.y,
		winPtr->changes.width, winPtr->changes.height, TRUE);
    }

    /*
     * Change the stacking order of the window.
     */

    if (value_mask & CWStackMode) {
	if ((value_mask & CWSibling) && (values->sibling != None)) {
	    HWND sibling = TkWinGetHWND(values->sibling);

	    /*
	     * Windows doesn't support the Above mode, so we insert the
	     * window just below the sibling and then swap them.
	     */

	    if (values->stack_mode == Above) {
		SetWindowPos(window, sibling, 0, 0, 0, 0,
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
		insertAfter = window;
		window = sibling;
	    } else {
		insertAfter = sibling;
	    }
	} else {
	    insertAfter = (values->stack_mode == Above) ? HWND_TOP
		: HWND_BOTTOM;
	}
		
	SetWindowPos(window, insertAfter, 0, 0, 0, 0,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    } 
}

/*
 *----------------------------------------------------------------------
 *
 * XClearWindow --
 *
 *	Clears the entire window to the current background color.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Erases the current contents of the window.
 *
 *----------------------------------------------------------------------
 */

void
XClearWindow(display, w)
    Display* display;
    Window w;
{
    RECT rc;
    HBRUSH brush;
    HPALETTE oldPalette, palette;
    TkWindow *winPtr;
    HWND hwnd = TkWinGetHWND(w);
    HDC dc = GetDC(hwnd);

    palette = TkWinGetPalette(display->screens[0].cmap);
    oldPalette = SelectPalette(dc, palette, FALSE);

    display->request++;

    winPtr = TkWinGetWinPtr(w);
    brush = CreateSolidBrush(winPtr->atts.background_pixel);
    GetWindowRect(hwnd, &rc);
    rc.right = rc.right - rc.left;
    rc.bottom = rc.bottom - rc.top;
    rc.left = rc.top = 0;
    FillRect(dc, &rc, brush);

    DeleteObject(brush);
    SelectPalette(dc, oldPalette, TRUE);
    ReleaseDC(hwnd, dc);
}
