/* 
 * tkMacNotify.c --
 *
 *	This file provides the platform specific event detection
 *	facilities used by the Tk event routines.  The procedures in
 *	this file comprise the notifier interface and contain unix
 *	specific file and timer handling code.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tkPort.h"
#include "tkInt.h"
#include "tkMacInt.h"
#include <signal.h>
#include <LowMem.h>

/*
 * A constant the defines the nominal amount of time (in ticks)
 * between calls to WaitNextEvent.
 */

#define WNE_WAIT_TIME 5

/*
 * The information below is used to provide read, write, and
 * exception masks to select during calls to Tk_DoOneEvent.
 */

static fd_mask ready[3*MASK_SIZE];
				/* Masks passed to select and modified
				 * by kernel to indicate which files are
				 * actually ready. */
static fd_mask check[3*MASK_SIZE];
				/* Current set of masks, built up by
				 * Tk_NotifyFile and Tk_NotifyDisplay,
				 * that reflects what files we should wait
				 * for in the next select. */
static int numFdBits = 0;	/* Number of valid bits in mask
				 * arrays (this value is passed
				 * to select).  This value is only a high
				 * water mark as long as at least one display
				 * is registered.  If there are no displays,
				 * then it is recomputed at the end of each
				 * call to Tk_DoOneEvent. */

/*
 * The notifier has at most one pending timer.  The next scheduled timer
 * event is recorded in the pendingTimeout structure.  The
 * pendingTimeoutPtr is set to refer to pendingTimeout by Tk_NotifyTimer.
 * Once the timer fires, Tk_DoOneEvent resets the pointer to NULL.
 */

static Tk_Time *pendingTimeoutPtr;
				/* Points to current timer value. */
static Tk_Time pendingTimeout;	/* Time when timer should fire. */


/*
 * Prototypes for procedures referenced only in this file:
 */

static void		RefreshFdBits _ANSI_ARGS_((void));

/*
 *----------------------------------------------------------------------
 *
 * Tk_NotifyFile --
 *
 *	Arrange for Tk_QueueEvent to be called the next time a given
 *	I/O channel becomes readable or writable.  This is a one-shot
 *	event.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The notifier will generate a file event when the I/O channel
 *	given by fd next becomes ready in the way indicated by mask.
 *	If fd is already registered then the old mask will be replaced
 *	with the new one.  Once the event is sent, the notifier will
 *	not send any more events about the fd until the next call to
 *	Tk_NotifyFile. 
 *
 *----------------------------------------------------------------------
 */

void
Tk_NotifyFile(fd, mask)
    int fd;			/* Integer identifier for a stream. */
    int mask;			/* OR'ed combination of TK_READABLE,
				 * TK_WRITABLE, and TK_EXCEPTION:
				 * indicates conditions under which a
				 * new event should be queued. */
{
    int index;
    fd_mask bitSelect;
    
    if (fd >= OPEN_MAX) {
	panic("Tk_NotifyFile can't handle file id %d", fd);
    }

    /*
     * Make sure the appropriate check bits are set for the specified file
     * descriptor.  
     */

    index = fd/(NBBY*sizeof(fd_mask));
    bitSelect = 1 << (fd%(NBBY*sizeof(fd_mask)));
    if (mask & TK_READABLE) {
	check[index] |= bitSelect;
    } else {
	check[index] &= ~bitSelect;
    }
    if (mask & TK_WRITABLE) {
	check[index+MASK_SIZE] |= bitSelect;
    } else {
	check[index+MASK_SIZE] &= ~bitSelect;
    }
    if (mask & TK_EXCEPTION) {
	check[index+2*MASK_SIZE] |= bitSelect;
    } else {
	check[index+2*MASK_SIZE] &= ~bitSelect;
    }

    if (numFdBits <= fd) {
	numFdBits = fd+1;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_IgnoreFile --
 *
 *	Cancel an outstanding file event request.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The notifier will stop looking for events on the I/O channel
 *	given by fd. 
 *
 *----------------------------------------------------------------------
 */

void
Tk_IgnoreFile(fd)
    int fd;			/* Integer identifier for a stream. */
{
    int index;
    fd_mask bitSelect;

    /*
     * Turn off check bits for the specified file.
     */

    index = fd/(NBBY*sizeof(fd_mask));
    bitSelect = 1 << (fd%(NBBY*sizeof(fd_mask)));
    check[index] &= ~bitSelect;
    check[index+MASK_SIZE] &= ~bitSelect;
    check[index+2*MASK_SIZE] &= ~bitSelect;

    /*
     * If this fd was at the high water mark and there is a chance
     * that it was the last fd, then recompute the numFdBits.
     */

    if (fd == (numFdBits-1)) {
	RefreshFdBits();
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_NotifyDisplay --
 *
 *	Arrange for events to be generated for a particular display.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	From now on, all window system events directed at display will
 *	be passed to Tk_QueueEvent.
 *
 *----------------------------------------------------------------------
 */

void
Tk_NotifyDisplay(display)
    Display *display;		/* Display for which events should be
				 * generated. */
{
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_IgnoreDisplay --
 *
 *	Arrange for events to no longer be delivered for a particular
 *	display.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	From now on, the notifier will not generate events for a
 *	particular display.
 *
 *----------------------------------------------------------------------
 */

void
Tk_IgnoreDisplay(display)
    Display *display;		/* Display for which events should not
				 * be generated. */
{
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_NotifyTimer --
 *
 *	Arrange for an event to be generated at a particular time in
 *	the future.  This is a one-shot event.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	When the time specified by timeout is reached, a timer event
 *	will be passed to Tk_QueueEvent.  There is only one
 *	outstanding timer, so if the previous timer has not expired,
 *	it will be replaced with the current timeout value.  Only one
 *	timer event will be generated after the timer expires until
 *	the next call to Tk_NotifyTimer.
 *
 *----------------------------------------------------------------------
 */

void
Tk_NotifyTimer(time)
    Tk_Time *time;		/* Absolute time when the notifier should
				 * generate a timer event. */
{
    pendingTimeoutPtr = &pendingTimeout;
    pendingTimeout = *time;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_DoOneEvent --
 *
 *	Process a single event of some sort.  If there's no work to
 *	do, wait for an event to occur, then process it.
 *
 * Results:
 *	The return value is 1 if the procedure actually found an event
 *	to process.  If no processing occurred, then 0 is returned.
 *	The caller should invoke Tk_DoOneEvent repeatedly until it
 *	returns 0 to insure that all events have been processed.
 *
 * Side effects:
 *	May delay execution of process while waiting for an event,
 *	unless TK_DONT_WAIT is set in the flags argument.
 *	Tk_ServiceEvent will be called at most once.  If no queued
 *	events were processed, then the procedure checks for new
 *	events.  If TK_DONT_WAIT is set or Tk has called
 *	Tk_NotifyIdle since the last time Tk_ServiceIdle was called,
 *	then Tk_DoOneEvent will only poll for events, otherwise it
 *	blocks until the next event occurs.  All detected events will
 *	be queued.  If no events are detected, and Tk has called
 *	Tk_NotifyIdle, then Tk_ServiceIdle will be called.
 *
 *----------------------------------------------------------------------
 */

int
Tk_DoOneEvent(flags)
    int flags;			/* Miscellaneous flag values:  may be any
				 * combination of TK_DONT_WAIT,
				 * TK_WINDOW_EVENTS, TK_FILE_EVENTS,
				 * TK_TIMER_EVENTS, and TK_IDLE_EVENTS. */
{
    struct timeval curTime, timeout, *timeoutPtr;
    Tk_Event event;
    fd_mask fdmask, bitSelect;
    int index, numFound, waitFlag;
    int foundEvent = 0;
    int dontLoop;		/* Avoids looping if set to 1. */
    long sleepTime;

    /*
     * No event flags is equivalent to TK_ALL_EVENTS.
     */
    
    if ((flags & TK_ALL_EVENTS) == 0) {
	flags |= TK_ALL_EVENTS;
    }

    dontLoop = (flags & TK_DONT_WAIT);

    /*
     * Look for events until we find something to do, or no other events
     * are possible.
     */

    do {

	/*
	 * The first thing we do is to service any asynchronous event
	 * handlers.
	 */
    
	if (Tcl_AsyncReady()) {
	    (void) Tcl_AsyncInvoke((Tcl_Interp *) NULL, 0);
	    return 1;
	}

	/*
	 * Ask Tk to service a queued event.  If Tk does not handle any
	 * events, then we should look for new events.
	 */

	if (Tk_ServiceEvent(flags)) {
	    return 1;		/* Tk serviced an event so we're done. */
	}

	/*
	 * Skip to the idle handlers if that is all we are doing.
	 */

	if ((flags & TK_ALL_EVENTS) == TK_IDLE_EVENTS) {
	    dontLoop = 1;
	    goto doIdle;
	}
    
	/*
	 * Step 1: poll for mouse moved events
	 */

	foundEvent |= TkGenerateMotionEvents();

	/*
	 * Step 2: poll select
	 */

	/*
	 * Load the select mask from the current check mask.  Always call
	 * select with a timeout of zero.  (Always poll.)
	 */

	memcpy((VOID *) ready, (VOID *) check, 3*MASK_SIZE*sizeof(fd_mask));

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	timeoutPtr = &timeout;
	numFound = select(numFdBits, (SELECT_MASK *) &ready[0],
		(SELECT_MASK *) &ready[MASK_SIZE],
		(SELECT_MASK *) &ready[2*MASK_SIZE], timeoutPtr);

	/*
	 * Some systems don't clear the masks after an error, so
	 * we have to do it here.
	 */

	if (numFound == -1) {
	    memset((VOID *) ready, 0, 3*MASK_SIZE*sizeof(fd_mask));
	}

	/*
	 * Check for file events.  We iterate over the ready fd_masks looking
	 * for a mask with any bits set.  By or'ing the read, write, and
	 * exception masks together, we can make a single pass through the file
	 * descriptors.  If any event occured on a file, we need to queue a
	 * file event with the appropriate file descriptor and event mask, and
	 * then clear all of the check bits for that file.
	 */
    
	if (numFound > 0) {
	    foundEvent = 1;
	    event.type = TK_FILE_EVENTS;
	    index = -1;
	    bitSelect = 0;
	    event.file.fd = 0;
	    fdmask = 0;

	    /*
	     * Each time through the loop we shift bitSelect.  When we
	     * have tested each bit in the current mask, bitSelect becomes
	     * zero, so we know to continue with the next mask in the set.
	     */
	
	    while (numFound > 0) {
		if (!bitSelect) {	/* check for wrap-around */
		    bitSelect = 1;
		    index++;
		    fdmask = ready[index] | ready[index+MASK_SIZE]
			| ready[index+2*MASK_SIZE];
		}
		if (fdmask & bitSelect) {
		    numFound--;
		    event.file.mask = 0;
		    if (ready[index] & bitSelect) {
			event.file.mask |= TK_READABLE;
		    }
		    if (ready[index+MASK_SIZE] & bitSelect) {
			event.file.mask |= TK_WRITABLE;
		    }
		    if (ready[index+2*MASK_SIZE] & bitSelect) {
			event.file.mask |= TK_EXCEPTION;
		    }
		    Tk_QueueEvent(&event, TK_QUEUE_TAIL);
		    check[index] &= ~bitSelect;
		    check[index+MASK_SIZE] &= ~bitSelect;
		    check[index+2*MASK_SIZE] &= ~bitSelect;
		}
		bitSelect <<= 1;
		event.file.fd++;
	    }
	}
		
	/* 
	 * Step 3: Window Events & time share 
	 */
	if (foundEvent || ((flags & TK_IDLE_EVENTS) && Tk_IdlePending())) {
	    sleepTime = 0;
	    waitFlag = TK_DONT_WAIT;
	} else {
	    waitFlag = 0;
	    if (pendingTimeoutPtr == NULL || pendingTimeoutPtr->sec > 1) {
		sleepTime = WNE_WAIT_TIME;
	    } else {
		sleepTime = pendingTimeoutPtr->usec / 16666;
		if (sleepTime > WNE_WAIT_TIME) {
		    sleepTime = WNE_WAIT_TIME;
		} else if (sleepTime == 0) {
		    waitFlag = TK_DONT_WAIT;
		}
	    }
	}
	foundEvent |= TkHandleMacEvents(waitFlag, sleepTime);
    
	/*
	 * Step 4: Check to see if the timer has expired.
	 */

	if ((pendingTimeoutPtr != NULL) && (flags & TK_TIMER_EVENTS)) {
	    (void) gettimeofday(&curTime, (struct timezone *) NULL);
	    if ((pendingTimeoutPtr->sec < curTime.tv_sec)
		    || ((pendingTimeoutPtr->sec == curTime.tv_sec)
			    &&  (pendingTimeoutPtr->usec < curTime.tv_usec))) {
		pendingTimeoutPtr = NULL;

		/*
		 * Generate timer event.
		 */

		foundEvent = 1;
		event.type = TK_TIMER_EVENTS;
		event.timer.time.sec = curTime.tv_sec;
		event.timer.time.usec = curTime.tv_usec;
		Tk_QueueEvent(&event, TK_QUEUE_TAIL);
	    }
	}

	/*
	 * If no other events were generated, then we should service any
	 * pending idle handlers.
	 */

	doIdle:
	if (!foundEvent && (flags & TK_IDLE_EVENTS)) {
	    foundEvent = Tk_ServiceIdle();
	}
    } while (!foundEvent && !dontLoop);

    /*
     * We found nothing at all to do this time around.
     */
    return foundEvent;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_Sleep --
 *
 *	Delay execution for the specified number of milliseconds.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Time passes.
 *
 *----------------------------------------------------------------------
 */

void
Tk_Sleep(ms)
    int ms;			/* Number of milliseconds to sleep. */
{
    long endTime;

    if (ms <= 0) {
	return;
    }
	
    /* 
     * One tick == 16.66 milliseconds 
     */
    Delay( (ms / 16.66), &endTime);
}

/*
 *--------------------------------------------------------------
 *
 * Tk_MainLoop --
 *
 *	Call Tk_DoOneEvent over and over again in an infinite
 *	loop as long as there exist any main windows.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Arbitrary;  depends on handlers for events.
 *
 *--------------------------------------------------------------
 */

void
Tk_MainLoop()
{
    while (Tk_GetNumMainWindows() > 0) {
	Tk_DoOneEvent(0);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * RefreshFdBits --
 *
 *	This function finds the largest currently registered
 *	file descriptor from the check mask.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Updates the numFdBits static variable.
 *
 *----------------------------------------------------------------------
 */

void
RefreshFdBits()
{
    int limit, index;
    fd_mask fdmask;

    /*
     * We start by finding the largest element of the check array that
     * contains at least one set bit.  We start the search with the
     * last known maximum index value.
     */

    limit = (numFdBits / (NBBY*sizeof(fd_mask)));
    numFdBits = 0;
    fdmask = 0;
    for (index = limit; (index >= 0); index--) {
	fdmask = check[index] | check[index+MASK_SIZE]
	    | check[index+2*MASK_SIZE];
	if (fdmask) {
	    break;
	}
    }

    /*
     * Now that we have the biggest mask, we need to find the most
     * significant bit.
     */

    if (fdmask) {
	int offset;
	numFdBits = index*NBBY*sizeof(fd_mask);
	for (offset = 0; ; offset++) {
	    fdmask &= ~(1<<offset);
	    if (!fdmask) {
		break;
	    }
	}
	numFdBits += (offset+1);
    }
}
