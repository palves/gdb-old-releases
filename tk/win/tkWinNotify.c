/* 
 * winNotify.c --
 *
 *	This file provides the platform specific event detection
 *	facilities used by the Tk event routines.  The procedures in
 *	this file comprise the notifier interface and contain Windows
 *	specific file and timer handling code.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include <winsock.h>
#include "tkWinInt.h"

/*
 * The notifier has at most one pending timer.  The next scheduled timer
 * event is recorded in the pendingTimeout structure.  The
 * pendingTimeoutPtr is set to refer to pendingTimeout by Tk_NotifyTimer.
 * Once the timer fires, Tk_DoOneEvent resets the pointer to NULL.
 */

static Tk_Time *pendingTimeoutPtr;
				/* Points to current timer value. */
static Tk_Time pendingTimeout;	/* Time when timer should fire. */
static UINT timerHandle = 0;	/* System handle for pending timer. */

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
				 * combination of TK_DONT_WAIT, TK_X_EVENTS,
				 * TK_FILE_EVENTS, TK_TIMER_EVENTS, and
				 * TK_IDLE_EVENTS. */
{
    struct timeval curTime, timeout, *timeoutPtr;
    Tk_Event event;
    MSG msg;
    int foundEvent = 0;
    int wasIdle = 1;
    int dontLoop;		/* Avoids looping if set to 1. */

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
	 * Ask Tk to service a queued event.  If Tk does not handle any events,
	 * then we should look for new events.
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
	 * Compute the current timeout value.  If idle tasks are pending or if
	 * TK_DONT_WAIT is set, then we should poll.  If a timer is pending,
	 * then we should compute the remaining time.  Otherwise, we block.
	 */

	if ((flags & TK_DONT_WAIT)
		|| ((flags & TK_IDLE_EVENTS) && Tk_IdlePending())) {
	    timeout.tv_sec = 0;
	    timeout.tv_usec = 0;
	    timeoutPtr = &timeout;
	} else if ((pendingTimeoutPtr != NULL) && (flags & TK_TIMER_EVENTS)) {
	    (void) gettimeofday(&curTime, (struct timezone *) NULL);

	    /*
	     * Check to see if the timer has expired.
	     */
	    
	    if ((pendingTimeoutPtr->sec < curTime.tv_sec)
		    || ((pendingTimeoutPtr->sec == curTime.tv_sec)
		    &&  (pendingTimeoutPtr->usec < curTime.tv_usec))) {
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
	    } else {
		timeout.tv_sec = pendingTimeoutPtr->sec - curTime.tv_sec;
		timeout.tv_usec = pendingTimeoutPtr->usec - curTime.tv_usec;
		if (timeout.tv_usec < 0) {
		    timeout.tv_sec -= 1;
		    timeout.tv_usec += 1000000;
		}
	    }
	    timeoutPtr = &timeout;
	} else {
	    timeoutPtr = NULL;
	}

	/*
	 * Cancel a previously scheduled timeout, if there is one.
	 */

	if (timerHandle != 0) {
	    KillTimer(NULL, timerHandle);
	    timerHandle = 0;
	}

	/*
	 * Wait for the next event, with an appropriate timeout.
	 */

	foundEvent = 1;
	if (timeoutPtr != NULL) {
	    UINT ms;
	    ms = timeout.tv_sec * 1000;
	    ms += timeout.tv_usec / 1000;
	    if (ms > 0) {
		timerHandle = SetTimer(NULL, 0, ms, NULL);
		GetMessage(&msg, NULL, 0, 0);
	    } else {
		foundEvent = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
	    }
	} else {
	    GetMessage(&msg, NULL, 0, 0);
	}

	/*
	 * We have been told to quit by Windows so don't do any more
	 * processing.
	 */

	if (foundEvent && msg.message == WM_QUIT) {
	    return 0;
	}

	/*
	 * Check to see if the timer has expired.
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

		wasIdle = 0;
		event.type = TK_TIMER_EVENTS;
		event.timer.time.sec = curTime.tv_sec;
		event.timer.time.usec = curTime.tv_usec;
		Tk_QueueEvent(&event, TK_QUEUE_TAIL);
	    }
	}

	if (foundEvent) {
	    TranslateMessage(&msg);
	    DispatchMessage(&msg);
	    return 1;
	}

	/*
	 * If no other events were generated, then we should service any
	 * pending idle handlers.
	 */

	doIdle:
	if (wasIdle && (flags & TK_IDLE_EVENTS)) {
	    foundEvent = Tk_ServiceIdle();
	}
    } while (!foundEvent && !dontLoop);

    /*
     * No other events can be detected at this point.
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
    Sleep(ms);
}

/*
 *----------------------------------------------------------------------
 *
 * TkWinDoQueuedEvents --
 *
 *	This routine is called from the TopLevelProc routine in order
 *	to force previously queued events and idle handlers to be
 *	processed during a recursive event loop callback.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Invokes event handlers for any pending window events and any
 *	idle handlers.
 *
 *----------------------------------------------------------------------
 */

void
TkWinDoQueuedEvents()
{
    /*
     * Loop until no more events are ready to be serviced.
     */

    while (Tk_ServiceEvent(TK_WINDOW_EVENTS)) { }

    /*
     * Invoke any pending idle handlers.
     */

    Tk_ServiceIdle();
}
