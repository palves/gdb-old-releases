/* 
 * tkMacWindowMgr.c --
 *
 *	Implements common window manager functions for the Macintosh.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include <Events.h>
#include <Dialogs.h>
#include <EPPC.h>
#include <Windows.h>
#include <ToolUtils.h>
#include <DiskInit.h>
#include <Desk.h>

#include "tkInt.h"
#include "tkPort.h"
#include "tkMacInt.h"

#define TK_DEFAULT_ABOUT 128

void TkGenWMMoveRequestEvent(Tk_Window tkwin, int x, int y);

/*
 * Prototypes for procedures referenced only in this file:
 */
static int		HandleMouseDown _ANSI_ARGS_((EventRecord *theEvent));

/*
 *----------------------------------------------------------------------
 *
 * HandleWMEvent --
 *
 *	Determine if event is system related, if so handle & return.
 *
 * Results:
 *	Return true if event was handle, false otherwise.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
HandleWMEvent(theEvent)
    EventRecord *theEvent;
{
    WindowPtr frontWindow;
    int handled;
	
    frontWindow = FrontWindow();
	
    switch (theEvent->what) {
	case mouseUp:
	case mouseDown:
	    handled = HandleMouseDown(theEvent);
	    return handled;
	case diskEvt:
	    /* Disk insertion. */
	    if (HiWord(theEvent->message) != noErr) {
		Point pt;
			
		DILoad();
		pt.v = pt.h = 120;		/* parameter ignored in sys 7 */
		DIBadMount(pt, theEvent->message);
		DIUnload();
	    }
	    return true;
	case kHighLevelEvent:
	    TkMacDoHLEvent(theEvent);
	    return true;
	case updateEvt:
	case activateEvt:
	    return false;
	case keyDown:
	case autoKey:
	    /*
	     * Handle menu-key events here.
	     */
	    if ((theEvent->modifiers & cmdKey) == cmdKey) {
		char c = theEvent->message & charCodeMask;
		TkMacHandleMenuSelect(MenuKey(c), false);
		return true;
	    }
	default:
	    return false;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * HandleMouseDown --
 *
 *	Handle mouse down events that pertain to window manager functions.
 *
 * Results:
 *	Return true if event was handle, false otherwise.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
HandleMouseDown(theEvent)
    EventRecord *theEvent;
{
    WindowPtr 	whichWindow, front;
    Boolean 	wasDraged = false;
    Window 		window;
    Tk_Window   tkwin;

    front = FrontWindow();

    /* 
     * The window manager only needs to know about mouse down events.
     */
    if (theEvent->what == mouseUp) {
	return false;
    }
	
    switch (FindWindow(theEvent->where, &whichWindow)) {
	case inSysWindow:
	    SystemClick( theEvent, whichWindow);
	    return true;
	case inDrag:
	    if (whichWindow != front) {
		if (theEvent->modifiers & cmdKey) {
		    DragWindow(whichWindow, theEvent->where,
			    &qd.screenBits.bounds);
		} else {
		    SelectWindow(whichWindow);
		    SetPort(whichWindow);
		}
	    } else {
		wasDraged = true;
		DragWindow(whichWindow, theEvent->where,
			&qd.screenBits.bounds);
	    }
			
	    if (wasDraged) {
		Point where;
				
		window = TkMacGetXWindow(whichWindow);
		tkwin = Tk_IdToWindow(tkDisplayList->display, window);
		SetPort(whichWindow);
		where.h = where.v = 0;
		LocalToGlobal(&where);
		TkGenWMMoveRequestEvent(tkwin, where.h, where.v);
	    }
	    return true;
	case inGrow:
	case inContent:
	    if (whichWindow != front ) {
		SelectWindow(whichWindow);
		SetPort(whichWindow);
		return true;
	    } else {
		/*
		 * Generally the content region is the domain of Tk
		 * sub-windows.  However, one exception is the grow
		 * region.  A button down in this area will be handled
		 * by the window manager.  Note: this means that Tk 
		 * will NEVER get button down events in this area!
		 */

		return TkWMGrowToplevel(whichWindow, theEvent->where);
	    }
	case inGoAway:
	    if (TrackGoAway( whichWindow, theEvent->where)) {
		Window window;
		Tk_Window tkwin;

		window = TkMacGetXWindow(whichWindow);
		tkwin = Tk_IdToWindow(tkDisplayList->display, window);
		if (tkwin == NULL) {
		    return true;
		}
		TkGenWMDestroyEvent(tkwin);
		return true;
	    }
	    return true;
	case inMenuBar:
	    {
		KeyMap theKeys;

		GetKeys(theKeys);
		TkMacHandleMenuSelect(MenuSelect(theEvent->where),
			theKeys[1] & 4);
		return true;
	    }
	default:
	    return false;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * TkAboutDlg --
 *
 *	Displays the default Tk About box.  This code uses Macintosh
 *	resources to define the content of the About Box.
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
TkAboutDlg()
{
    DialogPtr aboutDlog;
    short itemHit = -9;
	
    aboutDlog = GetNewDialog(128, NULL, (void*)(-1));
	
    if (!aboutDlog) {
	return;
    }
	
    SelectWindow((WindowPtr) aboutDlog);
	
    while (itemHit != 1) {
	ModalDialog( NULL, &itemHit);
    }
    DisposDialog(aboutDlog);
    aboutDlog = NULL;
	
    SelectWindow(FrontWindow());

    return;
}

