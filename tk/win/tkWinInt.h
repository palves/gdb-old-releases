/*
 * winInt.h --
 *
 *	Declarations of Windows specific shared variables and procedures.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef _WININT
#define _WININT

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <tkPort.h>
#include <tkInt.h>

/*
 * The TkWinDCState is used to save the state of a device context
 * so that it can be restored later.
 */

typedef struct TkWinDCState {
    HPALETTE palette;
} TkWinDCState;


/*
 * The TkWinDrawable is the internal implementation of an X Drawable (either
 * a Window or a Pixmap).  The following constants define the valid Drawable
 * types.
 */

#define TWD_BITMAP	1
#define TWD_WINDOW	2
#define TWD_WM_WINDOW	3

typedef struct {
    int type;
    HWND handle;
    TkWindow *winPtr;
} TkWinWindow;

typedef struct {
    int type;
    HBITMAP handle;
    TkWindow *winPtr;
    int depth;
} TkWinBitmap;
    
typedef union {
    int type;
    TkWinWindow window;
    TkWinBitmap bitmap;
} TkWinDrawable;

/*
 * The following macros are used to retrieve internal values from a Drawable.
 */

#define TkWinGetHWND(w) (((TkWinDrawable *) w)->window.handle)
#define TkWinGetWinPtr(w) (((TkWinDrawable*)w)->window.winPtr)
#define TkWinGetHBITMAP(w) (((TkWinDrawable*)w)->bitmap.handle)

/*
 * The following structure is used to encapsulate palette information.
 */

typedef struct {
    HPALETTE palette;		/* Palette handle used when drawing. */
    UINT size;			/* Number of entries in the palette. */
    int stale;			/* 1 if palette needs to be realized,
				 * otherwise 0.  If the palette is stale,
				 * then an idle handler is scheduled to
				 * realize the palette. */
    Tcl_HashTable refCounts;	/* Hash table of palette entry reference counts
				 * indexed by pixel value. */
} TkWinColormap;

/*
 * The following macro retrieves the Win32 palette from a colormap.
 */

#define TkWinGetPalette(colormap) (((TkWinColormap *) colormap)->palette)

/*
 * Internal procedures used by more than one source file.
 */

extern void		TkWinClipboardRender _ANSI_ARGS_((TkWindow *winPtr,
			    UINT format));
extern void		TkWinDoQueuedEvents _ANSI_ARGS_((void));
extern HINSTANCE 	TkWinGetAppInstance _ANSI_ARGS_((void));
extern unsigned int	TkWinGetModifierState _ANSI_ARGS_((UINT message,
			    WPARAM wParam, LPARAM lParam));
extern HDC		TkWinGetDrawableDC _ANSI_ARGS_((Display *display,
			    Drawable d, TkWinDCState* state));
extern TkWinDrawable *	TkWinGetDrawableFromHandle _ANSI_ARGS_((HWND hwnd));
extern HPALETTE		TkWinGetSystemPalette _ANSI_ARGS_((void));
extern void		TkWinPointerDeadWindow _ANSI_ARGS_((TkWindow *winPtr));
extern void		TkWinPointerEvent _ANSI_ARGS_((Tk_Event *event,
			    TkWindow *winPtr));
extern void		TkWinPointerInit _ANSI_ARGS_((void));
extern void		TkWinReleaseDrawableDC _ANSI_ARGS_((Drawable d,
			    HDC hdc, TkWinDCState* state));
HPALETTE		TkWinSelectPalette _ANSI_ARGS_((HDC dc,
			    Colormap colormap));
extern void		TkWinUpdateCursor _ANSI_ARGS_((TkWindow *winPtr));
extern void		TkWinWmConfigure _ANSI_ARGS_((TkWindow *winPtr,
			    WINDOWPOS *pos));
extern int		TkWinWmInstallColormaps _ANSI_ARGS_((HWND hwnd,
			    int message, int isForemost));
extern void		TkWinWmSetLimits _ANSI_ARGS_((TkWindow *winPtr,
			    MINMAXINFO *info));
extern void 		TkWinXInit _ANSI_ARGS_((HINSTANCE hInstance));

#endif /* _WININT */
