/* 
 * tkMacMenus.c --
 *
 *	These calls set up and manage the menubar for the
 *	Macintosh.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include <Menus.h>
#include <Memory.h>
#include <SegLoad.h>
#include <ToolUtils.h>
#include <Desk.h>

#include "tcl.h"
#include "tk.h"
#include "tkInt.h"
#include "tkMacInt.h"

#define kAppleMenu		256
#define kAppleAboutItem		1
#define kFileMenu		2
#define kEditMenu		3

#define kSourceItem		-1 /* Not Implemented Yet. */
#define kCloseItem		1
#define kQuitItem		3

#define EDIT_CUT		1
#define EDIT_COPY		2
#define EDIT_PASTE		3
#define EDIT_CLEAR		4

static Tcl_Interp *	gInterp;	/* Interpreter for this application. */
static MenuHandle	gAppleM;	/* Handles to menus */
static MenuHandle	gFileM;		/* Handles to menus */
static MenuHandle	gEditM;		/* Handles to menus */

Tk_Window Tk_TopCoordsToWindow _ANSI_ARGS_((Tk_Window tkwin, int rootX,
			int rootY, int *newX, int *newY));
static void FakeEditEvent _ANSI_ARGS_((int flag));


/*
 *----------------------------------------------------------------------
 *
 * TkMacHandleMenuSelect --
 *
 *    Displays the default Tk About box.
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
TkMacHandleMenuSelect(mResult, optionKeyPressed)
    long mResult;
    int optionKeyPressed;
{
    short theItem = LoWord(mResult);
    short theMenu = HiWord(mResult);
    Str255 name;
    Tk_Window tkwin;
    Window window;

    if (mResult == 0) {
	return;
    }

    switch (theMenu) {
	
	case kAppleMenu:
	    switch (theItem) {
		case kAppleAboutItem:
		    {
			Tcl_CmdInfo dummy;
			
			if (optionKeyPressed || gInterp == NULL ||
			    Tcl_GetCommandInfo(gInterp,
				    "tkAboutDialog", &dummy) == 0) {
			    TkAboutDlg();
			} else {
			    Tcl_Eval(gInterp, "tkAboutDialog");
			}
			break;
		    }
		default:
		    GetItem(gAppleM, theItem, name);
		    HiliteMenu(0);
		    OpenDeskAcc(name);
		    return;
	    }
	    break;
	case kFileMenu:
	    switch (theItem) {
		case kCloseItem:
		    /* Send close event */
		    window = TkMacGetXWindow(FrontWindow());
		    tkwin = Tk_IdToWindow(tkDisplayList->display, window);
		    TkGenWMDestroyEvent(tkwin);
		    break;
		case kQuitItem:
		    /* Exit */
		    if (optionKeyPressed || gInterp == NULL) {
			ExitToShell();
		    } else {
			Tcl_Eval(gInterp, "exit");
		    }
		    break;
	    }
	    break;
	case kEditMenu:
	    /*
	     * This implementation just send keysyms
	     * the Tk thinks are associated with function keys that
	     * do Cut, Copy & Paste on a Sun keyboard.
	     */
	    FakeEditEvent(theItem);
	    break;
	default:
	    /*
	     * This will eventually evoke Tcl scripts
	     * TODO: come up with frame work for this...
	     */
	    break;
    }

    /*
     * Finally we unhighlight the menu.
     */
    HiliteMenu(0);
} /* TkMacHandleMenuSelect */

/*
 *----------------------------------------------------------------------
 *
 * TkMacInitMenus --
 *
 *	This procedure initializes the Macintosh menu bar.
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
TkMacInitMenus(interp)
    Tcl_Interp 	*interp;
{
    Handle mbarH;

    gInterp = interp;

    /* 
     * At this point, InitMenus() should have already been called. 
     */

    mbarH = (Handle) GetMenuBar();
    if (mbarH == NULL) {
	panic("memory - menu bar");
    }

    gAppleM = NewMenu(256, "\p\024");
    if (gAppleM == NULL) {
	panic("memory - menus");
    }
    InsertMenu(gAppleM, 0);
    AppendMenu(gAppleM, "\pAbout Tcl & TkÉ");
    AppendMenu(gAppleM, "\p(-");
    AddResMenu(gAppleM, 'DRVR');

    gFileM = NewMenu(kFileMenu, "\pFile");
    if (gFileM == NULL) {
	panic("memory - menus");
    }
    InsertMenu(gFileM, 0);
    AppendMenu(gFileM, "\pClose/W");
    AppendMenu(gFileM, "\p(-");
    AppendMenu(gFileM, "\pQuit/Q");

    gEditM = NewMenu(kEditMenu, "\pEdit");
    if (gEditM == NULL) {
	panic("memory - menus");
    }
    InsertMenu(gEditM, 0);
    AppendMenu(gEditM, "\pCut/X");
    AppendMenu(gEditM, "\pCopy/C");
    AppendMenu(gEditM, "\pPaste/V");

    DrawMenuBar();

    DisposeHandle(mbarH);

    return;
} /* Mac_InitMenus */

/*
 *----------------------------------------------------------------------
 *
 * FakeEditEvent --
 *
 *    Displays the default Tk About box.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static void 
FakeEditEvent(flag)
    int flag;
{
    Tk_Event tkEvent;
    XEvent *event = &tkEvent.window.event;
    WindowPtr frontWin;
    Window window;
    Point where;
    Tk_Window tkwin;

    /* 
     * Find the window 
     */
    frontWin = FrontWindow();
    window = TkMacGetXWindow(frontWin);
    tkwin = Tk_IdToWindow(tkDisplayList->display, window);
    if (tkwin == NULL) {
        return;
    }

    tkEvent.type = TK_WINDOW_EVENTS;
    event->xany.send_event = false;
    event->xkey.same_screen = true;
    event->xkey.subwindow = None;
    event->xkey.time = GenerateTime();

    GetMouse(&where);
    tkwin = Tk_TopCoordsToWindow(tkwin, where.h, where.v, 
	    &event->xkey.x, &event->xkey.y);
    LocalToGlobal(&where);
    event->xkey.x_root = where.h;
    event->xkey.y_root = where.v;

    event->xany.serial = Tk_Display(tkwin)->request;
    event->xkey.window = Tk_WindowId(tkwin);
    event->xkey.display = Tk_Display(tkwin);
    event->xkey.root = XRootWindow(Tk_Display(tkwin), 0);
    
    switch (flag) {
	case EDIT_CUT:
	    event->xkey.keycode = 0x00020D17;
	    event->xkey.state = ControlMask;
	    event->xany.type = KeyPress;
	    Tk_QueueEvent(&tkEvent, TK_QUEUE_TAIL);
	    event->xany.type = KeyRelease;
	    Tk_QueueEvent(&tkEvent, TK_QUEUE_TAIL);
	    break;
	case EDIT_COPY:
	    /*
	     * Meta doesn't work yet - use Control-i as short term hack
	     */
	    event->xkey.state = ControlMask;
	    event->xkey.keycode = 0x00022209;
	    event->xany.type = KeyPress;
	    Tk_QueueEvent(&tkEvent, TK_QUEUE_TAIL);
	    event->xany.type = KeyRelease;
	    Tk_QueueEvent(&tkEvent, TK_QUEUE_TAIL);
	    break;
	case EDIT_PASTE:
	    event->xkey.keycode = 0x00021019;
	    event->xkey.state = ControlMask;
	    event->xany.type = KeyPress;
	    Tk_QueueEvent(&tkEvent, TK_QUEUE_TAIL);
	    event->xany.type = KeyRelease;
	    Tk_QueueEvent(&tkEvent, TK_QUEUE_TAIL);
	    break;
	case EDIT_CLEAR:
	    /*
	     * TODO: figure out a way to do this.
	     */
	    break;
    }
}
