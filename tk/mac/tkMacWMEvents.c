/* 
 * tkMacWMEvents.c --
 *
 *	This file contains a set of routines that take Mac events & 
 *      translates them into corasponding X events.  It also handles
 *	events that are to be passed to the Tk "window manager".
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include <Events.h>
#include <Windows.h>
#include <Resources.h>
#include <ToolUtils.h>

#include "tk.h"
#include "tkInt.h"
#include "tkMacInt.h"
#include "tkPort.h"

/*
 * Declarations of static variables used in this file.
 */

static int	 gCaptured = 0;		 /* 1 if mouse events outside of Tk 
				 	  * windows will be reported, else 0 */
static Tk_Window gGrabWinPtr = NULL;     /* Window that defines the top of the
				          * grab tree in a global grab. */
static Tk_Window gKeyboardWinPtr = NULL; /* Current keyboard grab window. */
static Point 	 gLastPointerPos;	 /* Last known position of mouse. */
static Tk_Window gLastWinPtr = NULL;     /* The last window the mouse was in */
static Tk_Window gRestrictWinPtr = NULL; /* Window to which all mouse
				          * events will be reported. */

/*
 * Forward declarations of procedures used in this file.
 */

static unsigned int ButtonKeyState _ANSI_ARGS_((void));
static int 	CheckEventsAvail _ANSI_ARGS_((void));
static void	CursorInit _ANSI_ARGS_((void));
static int	GenerateKeyEvent _ANSI_ARGS_((EventRecord *macEvent,
			Tk_Event *event));
static int	GenerateButtonEvent _ANSI_ARGS_((EventRecord *macEvent,
			Tk_Event *event));
static int	GenerateUpdateEvent _ANSI_ARGS_((EventRecord *macEvent));
static int	GenerateFocusEvent _ANSI_ARGS_((EventRecord *macEvent,
			Tk_Event *event));
static int 	GenerateEnterLeave _ANSI_ARGS_((Tk_Window tkwin,
			long x, int y));
static void 	GenerateUpdates _ANSI_ARGS_((RgnHandle updateRgn,
			TkWindow *winPtr));
static void 	UpdateCursor _ANSI_ARGS_((TkWindow *winPtr,
			WindowPtr whichwindow, Point whereLocal));

/*
 *----------------------------------------------------------------------
 *
 * GenerateUpdateEvent --
 *
 *	Given a Macintosh update event this function generates all the
 *	X update events needed by Tk.
 *
 * Results:
 *	True if event(s) are generated - false otherwise.  
 *
 * Side effects:
 *	Additional events may be place on the Tk event queue.
 *
 *----------------------------------------------------------------------
 */

static int
GenerateUpdateEvent(macEvent)
    EventRecord *macEvent;	/* Incoming Mac event */
{
    WindowPtr macWindow = (WindowPtr)macEvent->message;
    Window window;
    register TkWindow *winPtr;
	
    window = TkMacGetXWindow(macWindow);
    winPtr = (TkWindow *) Tk_IdToWindow(tkDisplayList->display, window);

    /*
     * After the call to BeginUpdate the visable region (visRgn) of the 
     * window is equal to the intersection of the real visable region and
     * the update region for this event.  We use this region in all of our
     * calculations.
     */

    if (winPtr != NULL) {
	BeginUpdate(macWindow);
	GenerateUpdates(macWindow->visRgn, winPtr);
	EndUpdate(macWindow);
	return true;
    }
	
    return false;
}

/*
 *----------------------------------------------------------------------
 *
 * GenerateUpdates --
 *
 *	Given a Macintosh update region and a Tk window this function
 *	geneates a X damage event for the window if it is within the
 *	update region.  The function will then recursivly have each
 *	damaged window generate damage events for its child windows.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Additional events may be place on the Tk event queue.
 *
 *----------------------------------------------------------------------
 */

static void
GenerateUpdates(updateRgn, winPtr)
    RgnHandle updateRgn;
    TkWindow *winPtr;
{
    TkWindow *childPtr;
    Tk_Event tkEvent;
    Rect bounds;
    RgnHandle rgn;

    TkMacWinBounds(winPtr, &bounds);
	
    if (bounds.top > (*updateRgn)->rgnBBox.bottom ||
	    (*updateRgn)->rgnBBox.top > bounds.bottom ||
	    bounds.left > (*updateRgn)->rgnBBox.right ||
	    (*updateRgn)->rgnBBox.left > bounds.right ||
	    !RectInRgn(&bounds, updateRgn)) {
	return;
    }

    tkEvent.type = TK_WINDOW_EVENTS;
    tkEvent.window.event.xany.serial = Tk_Display(winPtr)->request;
    tkEvent.window.event.xany.send_event = false;
    tkEvent.window.event.xany.window = Tk_WindowId(winPtr);
    tkEvent.window.event.xany.display = Tk_Display(winPtr);
	
    tkEvent.window.event.type = Expose;

    /* 
     * Compute the bounding box of the area that the damage occured in.
     */

    rgn = NewRgn();
    /*
     * CopyRgn(TkMacVisableClipRgn(winPtr), rgn);
     * TODO: this call doesn't work doing resizes!!!
     */
    RectRgn(rgn, &bounds);
    SectRgn(rgn, updateRgn, rgn);
    OffsetRgn(rgn, -bounds.left, -bounds.top);
    tkEvent.window.event.xexpose.x = (**rgn).rgnBBox.left;
    tkEvent.window.event.xexpose.y = (**rgn).rgnBBox.top;
    tkEvent.window.event.xexpose.width = (**rgn).rgnBBox.right -
	(**rgn).rgnBBox.left;
    tkEvent.window.event.xexpose.height = (**rgn).rgnBBox.bottom -
	(**rgn).rgnBBox.top;
    tkEvent.window.event.xexpose.count = 0;
    DisposeRgn(rgn);
    
    Tk_QueueEvent(&tkEvent, TK_QUEUE_TAIL);

    for (childPtr = winPtr->childList; childPtr != NULL;
				       childPtr = childPtr->nextPtr) {
	if (!Tk_IsMapped(childPtr) || Tk_IsTopLevel(childPtr)) {
	    continue;
	}

	GenerateUpdates(updateRgn, childPtr);
    }

    return;
}

/*
 *----------------------------------------------------------------------
 *
 * GenerateButtonEvent --
 *
 *	Given a Macintosh MouseDown or MouseUp event this procedure
 *	generates the appropiate X button events.
 *
 * Results:
 *	True if event(s) are generated - false otherwise.
 *
 * Side effects:
 *	Additional events may be place on the Tk event queue.
 *
 *----------------------------------------------------------------------
 */

static int
GenerateButtonEvent(macEvent, tkEvent)
    EventRecord *macEvent; 	/* Incoming Mac event */
    Tk_Event *tkEvent;
{
    WindowPtr whichWin, frontWin;
    Point where;
    Tk_Window tkwin;
    Window window;
    XEvent *event = &tkEvent->window.event;

    /* 
     * ButtonDown events will always occur in the front
     * window.  ButtonUp events, however, may occur anywhere
     * on the screen.  ButtonUp events should only be sent
     * to Tk if in the front window or during an implicit grab.
     */
    where = macEvent->where;
    FindWindow(where, &whichWin);
    frontWin = FrontWindow();
			
    if ((frontWin == NULL) || (frontWin != whichWin && gGrabWinPtr == NULL)) {
	return false;
    }

    event->xany.send_event = False;
    event->xbutton.x_root = where.h;
    event->xbutton.y_root = where.v;
    event->xbutton.subwindow = None;
    event->xbutton.same_screen = true;

    GlobalToLocal(&where);
    window = TkMacGetXWindow(whichWin);
    tkwin = Tk_IdToWindow(tkDisplayList->display, window);
    tkwin = Tk_TopCoordsToWindow(tkwin, where.h, where.v, 
	    &event->xbutton.x, &event->xbutton.y);
    if (gGrabWinPtr == NULL) {
	if (tkwin == NULL) {
	    /*
	     * This is possible is some weird cases - usally
	     * when switching apps.  I think it is safe just
	     * to ignore the event in this case.
	     */
	    return 0;
	}
    } else {
	Rect bounds;

	if (TkPositionInTree((TkWindow *) tkwin, (TkWindow *) gGrabWinPtr) !=
		TK_GRAB_IN_TREE) {
	    tkwin = gGrabWinPtr;
	}
	
	TkMacWinBounds((TkWindow *) tkwin, &bounds);		

	event->xbutton.x = where.h - bounds.left;
	event->xbutton.y = where.v - bounds.top;
    }

    event->xany.serial = Tk_Display(tkwin)->request;
    event->xany.display = Tk_Display(tkwin);
    event->xbutton.root = XRootWindow(Tk_Display(tkwin), 0);
    event->xbutton.window = Tk_WindowId(tkwin);
    event->xbutton.state = ButtonKeyState();
    event->xbutton.button = Button1;

    /*
     * Button events will also start or end an implicit grab.
     */
    if (macEvent->what == mouseDown) {
	event->xany.type = ButtonPress;
	/*
	 * Set mouse capture and the restrict window if we are
	 * currently unrestricted.
	 */

	if (!gRestrictWinPtr) {
	    if (!gGrabWinPtr) {
		gRestrictWinPtr = tkwin;
		gCaptured = 1;
	    } else {

		/*
		 * Make sure the new restrict window is inside the
		 * current grab tree.
		 */

		if (TkPositionInTree((TkWindow *) tkwin, (TkWindow *)
			gGrabWinPtr) > 0) {
		    gRestrictWinPtr = tkwin;
		} else {
		    gRestrictWinPtr = gGrabWinPtr;
		}
		gCaptured = 1;
	    }
	}
    } else {
	event->xany.type = ButtonRelease;
	
	/*
	 * Release the mouse capture when the last button is
	 * released and we aren't in a global grab.
	 */

	if (gGrabWinPtr == NULL) {
	    gCaptured = 0;
	}
		
	/*
	 * If we are releasing a restrict window, then we need
	 * to send the button event followed by mouse motion from
	 * the restrict window the the current mouse position.
	 */

	if (gRestrictWinPtr) {
	    if (Tk_WindowId(gRestrictWinPtr) != event->xbutton.window) {
		TkChangeEventWindow(event, (TkWindow *) gRestrictWinPtr);
	    }
	    gLastWinPtr = gRestrictWinPtr;
	    gRestrictWinPtr = NULL;
	}
	
    }

    Tk_QueueEvent(tkEvent, TK_QUEUE_TAIL);
    return true;
}

/*
 *----------------------------------------------------------------------
 *
 * GenerateFocusEvent --
 *
 *	Given a  Macintosh activate events this function generates X 
 *	FocusIn and FocusOut events.  Events are only generated for 
 *	the toplevel windows - Tk handles focus management within
 *	toplevels.
 *
 * Results:
 *	True if event(s) are generated - false otherwise.
 *
 * Side effects:
 *	Additional events may be place on the Tk event queue.
 *
 *----------------------------------------------------------------------
 */

static int
GenerateFocusEvent(macEvent, tkEvent)
    EventRecord *macEvent;	/* Incoming Mac event */
    Tk_Event *tkEvent;		
{
    XEvent *event = &tkEvent->window.event;
    Tk_Window tkwin;
    Window window;

    window = TkMacGetXWindow((WindowPtr) macEvent->message);
    tkwin = Tk_IdToWindow(tkDisplayList->display, window);
    if (tkwin == NULL) {
	return false;
    }

    /* TODO: also generate events on suspend and resume */
    if (macEvent->modifiers & activeFlag) {
	event->xany.type = FocusIn;
    } else {
	event->xany.type = FocusOut;
    }

    event->xany.serial = tkDisplayList->display->request;
    event->xany.send_event = False;
    event->xfocus.display = tkDisplayList->display;
    event->xfocus.window = window;
    event->xfocus.mode = NotifyNormal;
    event->xfocus.detail = NotifyDetailNone;

    Tk_QueueEvent(tkEvent, TK_QUEUE_TAIL);
    return true;
}

/*
 *----------------------------------------------------------------------
 *
 * GenerateKeyEvent --
 *
 *	Given Macintosh keyUp, keyDown & autoKey events this function
 *	generates the appropiate X key events.
 *
 * Results:
 *	True if event(s) are generated - false otherwise.
 *
 * Side effects:
 *	Additional events may be place on the Tk event queue.
 *
 *----------------------------------------------------------------------
 */

static int
GenerateKeyEvent(macEvent, tkEvent)
    EventRecord *macEvent;	/* Incoming Mac event */
    Tk_Event *tkEvent;
{
    WindowPtr frontWin;
    Window window;
    Point where;
    Tk_Window tkwin;
    XEvent *event = &tkEvent->window.event;

    frontWin = FrontWindow();

    window = TkMacGetXWindow(frontWin);
    tkwin = Tk_IdToWindow(tkDisplayList->display, window);
    if (tkwin == NULL) {
	return false;
    }
    where.v = macEvent->where.v;
    where.h = macEvent->where.h;

    event->xany.send_event = False;
    event->xkey.same_screen = true;
    event->xkey.subwindow = None;
    event->xkey.time = GenerateTime();
    event->xkey.x_root = where.h;
    event->xkey.y_root = where.v;

    GlobalToLocal(&where);
    tkwin = Tk_TopCoordsToWindow(tkwin, where.h, where.v, 
	    &event->xkey.x, &event->xkey.y);
    event->xkey.keycode = macEvent->message;

    event->xany.serial = Tk_Display(tkwin)->request;
    event->xkey.window = Tk_WindowId(tkwin);
    event->xkey.display = Tk_Display(tkwin);
    event->xkey.root = XRootWindow(Tk_Display(tkwin), 0);
    event->xkey.state = ButtonKeyState();

    if (macEvent->what == keyDown) {
	event->xany.type = KeyPress;
	Tk_QueueEvent(tkEvent, TK_QUEUE_TAIL);
    } else if (macEvent->what == keyUp) {
	event->xany.type = KeyRelease;
	Tk_QueueEvent(tkEvent, TK_QUEUE_TAIL);
    } else {
	/*
	 * Autokey events send multiple XKey events.
	 *
	 * Note: the last KeyRelease will always be missed with
	 * this scheme.  However, most Tk scripts don't look for
	 * KeyUp events so we should be OK.
	 */
	event->xany.type = KeyRelease;
	Tk_QueueEvent(tkEvent, TK_QUEUE_TAIL);
	event->xany.type = KeyPress;
	Tk_QueueEvent(tkEvent, TK_QUEUE_TAIL);
    }
    return true;
}

/*
 *----------------------------------------------------------------------
 *
 * TkGenerateMotionEvents --
 *
 *	This function polls the mouse position and generates X Motion,
 *	Enter & Leave events.  The cursor is also updated in this
 *	procedure.
 *
 * Results:
 *	True if event(s) are generated - false otherwise.  
 *
 * Side effects:
 *	Additional events may be place on the Tk event queue.
 *	The cursor may be changed.
 *
 *----------------------------------------------------------------------
 */

int
TkGenerateMotionEvents()
{
    Tk_Event tkEvent;
    Tk_Window tkwin, rootwin;
    Window window;
    WindowPtr whichwindow, frontWin;
    Point whereLocal, whereGlobal;
    Boolean genMouseMoved, inContentRgn;
    XEvent *event = &tkEvent.window.event;
    short part;
    int local_x, local_y;
 
    tkEvent.type = TK_WINDOW_EVENTS;
    frontWin = FrontWindow();
    if (frontWin == NULL) {
	return false;
    }
    SetPort(frontWin);
   
    GetMouse(&whereLocal);
    whereGlobal = whereLocal;
    LocalToGlobal(&whereGlobal);
	
    part = FindWindow(whereGlobal, &whichwindow);
    inContentRgn = (part == inContent || part == inGrow);

    /*
     * Only generate events if the mouse has moved
     */
    if (EqualPt(gLastPointerPos, whereGlobal)) {
	return false;
    } else {
	genMouseMoved = true;
	gLastPointerPos = whereGlobal;
    }
	
    if (!inContentRgn) {
	tkwin = NULL;
    } else {
	window = TkMacGetXWindow(whichwindow);
	rootwin = Tk_IdToWindow(tkDisplayList->display, window);
	if (rootwin == NULL) {
	    tkwin = NULL;
	} else {
	    SetPort(whichwindow);
	    whereLocal = whereGlobal;
	    GlobalToLocal(&whereLocal);
	    tkwin = Tk_TopCoordsToWindow(rootwin, whereLocal.h, whereLocal.v, 
		    &local_x, &local_y);
	}
    }

    UpdateCursor((TkWindow *) tkwin, whichwindow, whereLocal);

    /*
     * Generate enter and leave events
     */
    GenerateEnterLeave(tkwin, whereGlobal.h, whereGlobal.v);

    /* 
     * Mouse moved events generated only when mouse is in the
     * content of the front window and not untill the next 
     * allowable time.
     */
    if (genMouseMoved) {
	if (gRestrictWinPtr) {
	    tkwin = gRestrictWinPtr;
	} else if (gGrabWinPtr && !tkwin) {
	    tkwin = gGrabWinPtr;
	}
    
	if (gCaptured) {
	    Rect bounds;

	    TkMacWinBounds((TkWindow *) tkwin, &bounds);		

	    local_x = whereLocal.h - bounds.left;
	    local_y = whereLocal.v - bounds.top;
	} else {
	    if (part != inContent) {
		return false;
	    }
	}

	event->xany.type = MotionNotify;
	event->xany.serial = Tk_Display(tkwin)->request;
	event->xany.send_event = False;
	event->xany.display = Tk_Display(tkwin);
	event->xmotion.window = Tk_WindowId(tkwin);
	event->xmotion.root = XRootWindow(Tk_Display(tkwin), 0);
	event->xmotion.state = ButtonKeyState();
	event->xmotion.subwindow = None;
	event->xmotion.time = GenerateTime();
	event->xmotion.x_root = whereGlobal.h;
	event->xmotion.y_root = whereGlobal.v;
	event->xmotion.x = local_x;
	event->xmotion.y = local_y;
	event->xmotion.same_screen = true;
	event->xmotion.is_hint = NotifyNormal;
	Tk_QueueEvent(&tkEvent, TK_QUEUE_TAIL);
    }

    return true;
}

/*
 *----------------------------------------------------------------------
 *
 * ButtonKeyState --
 *
 *	Returns the current state of the button & modifier keys.
 *
 * Results:
 *	A bitwise inclusive OR of a subset of the following:
 *	Button1Mask, ShiftMask, LockMask, ControlMask, Mod?Mask,
 *	Mod?Mask.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static unsigned int
ButtonKeyState()
{
    unsigned int state = 0;
    KeyMap theKeys;

    if (Button()) {
	state |= Button1Mask;
    }

    GetKeys(theKeys);

    if (theKeys[1] & 2) {
	state |= LockMask;
    }

    if (theKeys[1] & 1) {
	state |= ShiftMask;
    }

    if (theKeys[1] & 8) {
	state |= ControlMask;
    }

    if (theKeys[1] & 32768) {
	state |= Mod1Mask;		/* command key */
    }

    if (theKeys[1] & 4) {
	state |= Mod2Mask;		/* option key */
    }

    return state;
}

/*
 *----------------------------------------------------------------------
 *
 * UpdateCursor --
 *
 *	This function updates the cursor based on the current window the
 *  	the cursor is over.  It also generates the resize cursor for the
 *  	resize region in the lower right hand corner of the window.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The cursor may change shape.
 *
 *----------------------------------------------------------------------
 */

static void
UpdateCursor(winPtr, whichwindow, whereLocal)
    TkWindow *winPtr;
    WindowPtr whichwindow;
    Point whereLocal;
{
    WindowPtr frontWin;
    CursHandle cursor;

    /*
     * The cursor will change during an implicit grab only under
     * a few special cases - such as bindings.
     */
    if (gRestrictWinPtr != NULL) {
	winPtr = (TkWindow *) gRestrictWinPtr;
	TkUpdateCursor(winPtr);
	return;
    }

    /*
     * The cursor should be the arrow if outside the active window.
     */
    frontWin = FrontWindow();
    if (frontWin != whichwindow) {	
	TkUpdateCursor(NULL);
	return;
    }

    /*
     * One special case is the grow region.  Because a Tk window may
     * not have allocated space for the grow region the grow region
     * floats above the rest of the Tk window.  This is showen by
     * changing the cursor over the grow region.
     *
     * TODO: check if resizing is disabled or a grow region was specified
     */
    if (gGrabWinPtr == NULL) {
	if (whereLocal.h > (whichwindow->portRect.right - 16) &&
		whereLocal.v > (whichwindow->portRect.bottom - 16)) {
	    cursor = (CursHandle) GetNamedResource('CURS', "\presize");
	    SetCursor(*cursor);
	    return;
	}
    }

    /*
     * Set the cursor to the value set for the given window.  If
     * the value is None - set to the arrow cursor
     */
    TkUpdateCursor(winPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * GenerateEnterLeave --
 *
 *	Given a Tk window and the current mouse position this
 *	function will generate the appropiate X Enter and Leave
 *	events.
 *
 * Results:
 *	True if event(s) are generated - false otherwise.  
 *
 * Side effects:
 *	Additional events may be place on the Tk event queue.
 *
 *----------------------------------------------------------------------
 */

int
GenerateEnterLeave(tkwin, x, y)
    Tk_Window tkwin;		/* current Tk window (or NULL)  */
    long x;			/* current mouse position in */
    long y;			/* root coordinates */
{
    int crossed = 0;		/* 1 if mouse crossed a window boundary */
    Window window;

    /*
     * The following is a sanity check.  It is possible for the
     * last window to have been deleted and no longer valid.
     */
    window = Tk_WindowId(gLastWinPtr);
    if (gLastWinPtr != Tk_IdToWindow(tkDisplayList->display, window)) {
	gLastWinPtr = NULL;
    }

    if (tkwin != gLastWinPtr) {
	Tk_Window targetPtr;

	if (gRestrictWinPtr) {
	    if ((gRestrictWinPtr == gLastWinPtr)
		    || (gRestrictWinPtr == tkwin)) {
		targetPtr = gRestrictWinPtr;
	    } else {
		targetPtr = NULL;
	    }
	} else if ((gLastWinPtr == NULL)
		|| (Tk_WindowId(gLastWinPtr) == None)) {
	    targetPtr = tkwin;
	} else {
	    targetPtr = gLastWinPtr;
	}

	if ((targetPtr != NULL) && (Tk_WindowId(targetPtr) != None)) {
	    Tk_Event e;

	    /*
	     * Generate appropriate Enter/Leave events.
	     */

	    e.type = TK_WINDOW_EVENTS;
	    e.window.event.xcrossing.serial = Tk_Display(targetPtr)->request;
	    e.window.event.xcrossing.send_event = 0;
	    e.window.event.xcrossing.display = Tk_Display(targetPtr);
	    e.window.event.xcrossing.root = RootWindow(Tk_Display(targetPtr),
		    Tk_ScreenNumber(targetPtr));
	    e.window.event.xcrossing.time = GenerateTime();
	    e.window.event.xcrossing.x_root = x;
	    e.window.event.xcrossing.y_root = y;
	    e.window.event.xcrossing.state = ButtonKeyState();
	    e.window.event.xcrossing.mode = NotifyNormal;
	    e.window.event.xcrossing.focus = false;
	    TkInOutEvents(&e, (TkWindow *) gLastWinPtr, (TkWindow *) tkwin,
		    LeaveNotify, EnterNotify, TK_QUEUE_TAIL);
	    crossed = 1;
	}
	gLastWinPtr = tkwin;
    }

    return crossed;
}

/*
 *----------------------------------------------------------------------
 *
 * XGrabPointer --
 *
 *	Capture the mouse so event are reported outside of toplevels.
 *	Note that this is a very limited implementation that only
 *	supports GrabModeAsync and owner_events True.
 *
 * Results:
 *	Always returns GrabSuccess.
 *
 * Side effects:
 *	Turns on mouse capture, sets the global grab pointer, and
 *	clears any window restrictions.
 *
 *----------------------------------------------------------------------
 */

int
XGrabPointer(display, grab_window, owner_events, event_mask, pointer_mode,
	keyboard_mode, confine_to, cursor, time)
    Display* display;
    Window grab_window;
    Bool owner_events;
    unsigned int event_mask;
    int pointer_mode;
    int keyboard_mode;
    Window confine_to;
    Cursor cursor;
    Time time;
{
    gCaptured = 1;
    gGrabWinPtr = Tk_IdToWindow(display, grab_window);
    gRestrictWinPtr = NULL;
    if (TkPositionInTree((TkWindow *) gLastWinPtr, (TkWindow *) gGrabWinPtr) !=
	    TK_GRAB_IN_TREE) {
	TkUpdateCursor((TkWindow *) gGrabWinPtr);
    }
    return GrabSuccess;
}

/*
 *----------------------------------------------------------------------
 *
 * XUngrabPointer --
 *
 *	Release the current grab.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Releases the mouse capture.
 *
 *----------------------------------------------------------------------
 */

void
XUngrabPointer(display, time)
    Display* display;
    Time time;
{
    gCaptured = 0;
    gGrabWinPtr = NULL;
    gRestrictWinPtr = NULL;
    TkUpdateCursor((TkWindow *) gLastWinPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * XGrabKeyboard --
 *
 *	Simulates a keyboard grab by setting the focus.
 *
 * Results:
 *	Always returns GrabSuccess.
 *
 * Side effects:
 *	Sets the keyboard focus to the specified window.
 *
 *----------------------------------------------------------------------
 */

int
XGrabKeyboard(display, grab_window, owner_events, pointer_mode,
	keyboard_mode, time)
    Display* display;
    Window grab_window;
    Bool owner_events;
    int pointer_mode;
    int keyboard_mode;
    Time time;
{
    gKeyboardWinPtr = Tk_IdToWindow(display, grab_window);
    return GrabSuccess;
}

/*
 *----------------------------------------------------------------------
 *
 * XUngrabKeyboard --
 *
 *	Releases the simulated keyboard grab.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Sets the keyboard focus back to the value before the grab.
 *
 *----------------------------------------------------------------------
 */

void
XUngrabKeyboard(display, time)
    Display* display;
    Time time;
{
    gKeyboardWinPtr = NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * XQueryPointer --
 *
 *	Check the current state of the mouse.  This is not a complete
 *	implementation of this function.  It only computes the root
 *	coordinates and the current mask.
 *
 * Results:
 *	Sets root_x_return, root_y_return, and mask_return.  Returns
 *	true on success.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

Bool
XQueryPointer(display, w, root_return, child_return, root_x_return,
	root_y_return, win_x_return, win_y_return, mask_return)
    Display* display;
    Window w;
    Window* root_return;
    Window* child_return;
    int* root_x_return;
    int* root_y_return;
    int* win_x_return;
    int* win_y_return;
    unsigned int* mask_return;
{
    Point where;

    GetMouse(&where);
    LocalToGlobal(&where);
    *root_x_return = where.h;
    *root_y_return = where.v;
    *mask_return = ButtonKeyState();    
    return True;
}

/*
 *----------------------------------------------------------------------
 *
 * GenerateTime --
 *
 *	Returns the total number of ticks from startup  This function
 *	is used to generate the time of generated X events.
 *
 * Results:
 *	Returns the current time (ticks from startup).
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

Time
GenerateTime()
{
    return (Time) TickCount();
}

/*
 *----------------------------------------------------------------------
 *
 * TkMacPointerDeadWindow --
 *
 *	Clean up pointer module state when a window is destroyed.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	May change the grab module settings.
 *
 *----------------------------------------------------------------------
 */

void
TkMacPointerDeadWindow(winPtr)
    TkWindow *winPtr;
{
    if ((Tk_Window) winPtr == gLastWinPtr) {
	gLastWinPtr = NULL;
    }
    if ((Tk_Window) winPtr == gGrabWinPtr) {
	gGrabWinPtr = NULL;
    }
    if ((Tk_Window) winPtr == gRestrictWinPtr) {
	gRestrictWinPtr = NULL;
    }
    if (!(gRestrictWinPtr || gGrabWinPtr)) {
	gCaptured = 0;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * TkMacHandleMacEvents --
 *
 *	This function checks for events from the Macintosh event queue.
 *	It also is the point at which the Tk application must provide
 *	cooprative multitasking with other Macintosh applications.  Mac
 *	events are then translated into the appropiate X events and
 *	placed on the Tk event queue.
 *
 * Results:
 *	Returns 1 if event found, 0 otherwise.
 *
 * Side effects:
 *	May change the grab module settings.
 *
 *----------------------------------------------------------------------
 */

int
TkHandleMacEvents(flags, sleepTime)
    int flags;
    long sleepTime;
{
    Tk_Event tkEvent;
    EventRecord theEvent;
    int eventFound = false;
    int eventToProcess = false;
    static long nextOSEventTime = 0;

    /*
     * If the TK_DONT_WAIT flag is set then we first check to see
     * events are available and simple return if none will be found.
     * As a preformance improvement we never wait if the mouse is
     * being held down.  (Will help with responsivness.)
     */
     
    if ((flags & TK_DONT_WAIT) || Button()) {
	if (CheckEventsAvail() == false) {
	    return false;
	} else {
	    sleepTime = 0;	
	}
    }

    tkEvent.type = TK_WINDOW_EVENTS;
    do {
	eventToProcess = WaitNextEvent(everyEvent, &theEvent, sleepTime, NULL);
	if (!eventToProcess) {
	    break;
	}			

	if (HandleWMEvent(&theEvent)) {
	    /* TODO: what is this for... */
	    if (theEvent.what != activateEvt && theEvent.what != osEvt) {
		continue;
	    }
	}
		
	switch (theEvent.what) {
	    case updateEvt:
		if (GenerateUpdateEvent(&theEvent)) {
		    eventFound = true;
		}
		break;
	    case mouseDown:
	    case mouseUp:
		if (GenerateButtonEvent(&theEvent, &tkEvent)) {
		    eventFound = true;
		}
		break;
	    case autoKey:
	    case keyDown:
	    case keyUp:
		eventFound |= GenerateKeyEvent(&theEvent, &tkEvent);
		break;
	    case activateEvt:
		eventFound |= GenerateFocusEvent(&theEvent, &tkEvent);
		break;
	    case osEvt:
		/*
		 * Do clipboard conversion.
		 */
		if ((theEvent.message & resumeFlag) &&
			(theEvent.message & convertClipboardFlag)) {
		    TkResumeClipboard();
		} else {
		    TkSuspendClipboard();
		}
		break;
	}
    } while (CheckEventsAvail() == true);
    
    return eventFound;
}

/*
 *----------------------------------------------------------------------
 *
 * CheckEventsAvail --
 *
 *	Checks to see if events are available on the Macintosh queue.
 *	This function looks for both queued events (eg. key & button)
 *	and generated events (update).
 *
 * Results:
 *	True is events exist, false otherwise.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
CheckEventsAvail()
{
    QHdrPtr evPtr;
    WindowPeek macWinPtr;
    
    evPtr = GetEvQHdr();
    if (evPtr->qHead != NULL) {
	return true;
    }
    
    macWinPtr = (WindowPeek) FrontWindow();
    while (macWinPtr != NULL) {
	if (!EmptyRgn(macWinPtr->updateRgn)) {
	    return true;
	}
	macWinPtr = macWinPtr->nextWindow;
    }
    return false;
}
