/*
 * tkMacInt.h --
 *
 *	Declarations of Windows specific shared variables and procedures.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef _TKMACINT
#define _TKMACINT

#include "tkInt.h"
#include "tkPort.h"

#include <AppleEvents.h>
#include <Windows.h>
#include <QDOffscreen.h>

struct TkWindowPrivate {
    TkWindow *winPtr;     	/* Ptr to tk window or NULL if Pixmap */
    GWorldPtr portPtr;     	/* Either WindowPtr or off screen world */
    int xOff;	       		/* X offset from toplevel window */
    int yOff;		       	/* Y offset from toplevel window */
    RgnHandle clipRgn;		/* Visable region of window */
    RgnHandle aboveClipRgn;	/* Visable region of window & it's children */
    Boolean clipValid;   	/* is the current clip rgn valid */
    int magic;			/* Magic number use to ident Tk toplevels */
    int referenceCount;		/* Don't delete toplevel until children are
				 * gone. */
    struct TkWindowPrivate *toplevel;	/* Pointer to the toplevel
					 * datastruct. */
};
typedef struct TkWindowPrivate MacDrawable;

/*
 * Internal procedures shared among Macintosh Tk modules but not exported
 * to the outside world:
 */

extern GWorldPtr 	TkMacGetDrawablePort _ANSI_ARGS_((Drawable drawable));
extern Window 		TkMacGetXWindow _ANSI_ARGS_((WindowPtr macWinPtr));
extern void		TkMacPointerDeadWindow _ANSI_ARGS_((TkWindow *winPtr));
extern void		InvalClipRgns _ANSI_ARGS_((TkWindow *winPtr));
extern void		tkMacInstallMWConsole _ANSI_ARGS_((
			    Tcl_Interp *interp));
extern int 		TkSetMacColor _ANSI_ARGS_((unsigned long pixel,
			    RGBColor *macColor));
extern void		TkUpdateCursor _ANSI_ARGS_((TkWindow *winPtr));
extern void		TkSuspendClipboard _ANSI_ARGS_((void));
extern void		TkResumeClipboard _ANSI_ARGS_((void));
extern void		TkMacInitAppleEvents _ANSI_ARGS_((Tcl_Interp *interp));
extern void		TkMacDoHLEvent _ANSI_ARGS_((EventRecord *theEvent));
extern void 		TkMacHandleMenuSelect _ANSI_ARGS_((long mResult,
			    int optionKeyPressed));
extern int		TkHandleMacEvents _ANSI_ARGS_((int flags,
			    long sleepTime));
extern RgnHandle 	TkMacVisableClipRgn _ANSI_ARGS_((TkWindow *winPtr));
extern void 		TkAboutDlg _ANSI_ARGS_((void));
extern void		TkGenWMMoveRequestEvent _ANSI_ARGS_((Tk_Window tkwin,
			    int x, int y));
extern void		TkGenWMResizeRequestEvent _ANSI_ARGS_((Tk_Window tkwin,
			    int width, int height));
extern void		TkMacWinBounds _ANSI_ARGS_((TkWindow *winPtr,
			    Rect *bounds));
extern int		HandleWMEvent _ANSI_ARGS_((EventRecord *theEvent));
extern int		TkGenerateMotionEvents _ANSI_ARGS_((void));
extern int		TkWMGrowToplevel _ANSI_ARGS_((WindowPtr whichWindow,
			    Point start));
extern Tk_Window	Tk_TopCoordsToWindow _ANSI_ARGS_((Tk_Window tkwin,
			    int rootX, int rootY, int *newX, int *newY));
extern void		TkMacWMInitBounds _ANSI_ARGS_((TkWindow *winPtr,
			    Rect *geometry));
extern void		TkGenWMDestroyEvent _ANSI_ARGS_((Tk_Window tkwin));
extern void 		TkMacInitMenus _ANSI_ARGS_((Tcl_Interp 	*interp));
extern Time		GenerateTime _ANSI_ARGS_(());
extern int 		TkGetCharPositions _ANSI_ARGS_((
			    XFontStruct *font_struct, char *string,
			    int count, short *buffer));
extern void 		TkMacFontInfo _ANSI_ARGS_((Font fontId, short *family,
			    short *style, short *size));
extern void 		TkFontList _ANSI_ARGS_((Tcl_Interp *interp,
			    Display *display));
extern void 		TkMacUpdateClipRgn _ANSI_ARGS_((TkWindow *winPtr));
extern Window		TkGetTransientMaster _ANSI_ARGS_((TkWindow *winPtr));

#endif /* _TKMACINT */
