/* 
 * tkWinPixmap.c --
 *
 *	This file contains the Xlib emulation functions pertaining to
 *	creating and destroying pixmaps.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tkWinInt.h"


/*
 *----------------------------------------------------------------------
 *
 * XCreatePixmap --
 *
 *	Creates an in memory drawing surface.
 *
 * Results:
 *	Returns a handle to a new pixmap.
 *
 * Side effects:
 *	Allocates a new Win32 bitmap.
 *
 *----------------------------------------------------------------------
 */

Pixmap
XCreatePixmap(display, d, width, height, depth)
    Display* display;
    Drawable d;
    unsigned int width;
    unsigned int height;
    unsigned int depth;
{
    TkWinDrawable *twdPtr;
    
    display->request++;

    twdPtr = (TkWinDrawable*) ckalloc(sizeof(TkWinDrawable));
    if (twdPtr == NULL) {
	return None;
    }
    twdPtr->type = TWD_BITMAP;
    twdPtr->bitmap.depth = depth;
    twdPtr->bitmap.winPtr = TkWinGetWinPtr(d);
    twdPtr->bitmap.handle = CreateBitmap(width, height, 1, depth, NULL);

    if (twdPtr->bitmap.handle == NULL) {
	ckfree((char *) twdPtr);
	return NULL;
    }
    
    return (Pixmap)twdPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * XFreePixmap --
 *
 *	Release the resources associated with a pixmap.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Deletes the bitmap created by XCreatePixmap.
 *
 *----------------------------------------------------------------------
 */

void
XFreePixmap(display, pixmap)
    Display* display;
    Pixmap pixmap;
{
    TkWinDrawable *twdPtr = (TkWinDrawable *) pixmap;

    display->request++;
    if (twdPtr != NULL) {
	DeleteObject(twdPtr->bitmap.handle);
	ckfree((char *)twdPtr);
    }
}
