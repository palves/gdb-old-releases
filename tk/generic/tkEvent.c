/* 
 * tkEvent.c --
 *
 *	This file provides basic event-managing facilities, whereby
 *	procedure callbacks may be attached to certain events.  It
 *	also contains the command procedures for the commands "after"
 *	and "fileevent", plus abridged versions of "tkwait" and
 *	"update", for use with Tk_EventInit.
 *
 * Copyright (c) 1990-1994 The Regents of the University of California.
 * Copyright (c) 1994-1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#ifdef _Windows
#include <winsock.h>
#endif /* _Windows */

#include "tkPort.h"
#include "tkInt.h"
#include <errno.h>

/*
 * Global pointer to procedure which handles window events.  If
 * tkHandleEventProc is NULL, then Tk_ServiceEvent will discard any window
 * events.  Otherwise Tk_ServiceEvent will invoke the procedure referenced
 * by tkHandleEventProc.  This global is set by Tk_CreateMainWindow when Tk
 * is initialized.  This pointer exists so tkEvent.c does not have any
 * direct references to the X dependent portions of Tk and can be used by
 * itself in an application.
 */

void (*tkHandleEventProc) _ANSI_ARGS_((XEvent *eventPtr)) = NULL;

/*
 * For each event that is pending, there is one record of the following
 * type, chained together in a queue.  In addition to the head and tail
 * pointers, the queue also maintains a current "mark".  The mark is used
 * to insert events in FIFO order before any currently scheduled events.
 * The grab module uses this feature to insert a block of enter/leave
 * events at the head of the queue.
 */

typedef struct EventList {
    Tk_Event event;
    struct EventList* nextPtr;
} EventList;

static EventList *firstEventPtr = NULL;
				/* Pointer to the head of the queue. */
static EventList *lastEventPtr = NULL;
				/* Pointer to the tail of the queue. */
static EventList *markerEventPtr = NULL;
				/* Pointer to the current marker event. */

/*
 * A mouse motion event may be delayed, in order to merge it with
 * subsequent mouse motion events.  If a mouse motion event has been
 * delayed, then haveDelayedMotionEvent will be 1 and
 * delayedMotionEvent will contain a copy of the event, otherwise
 * haveDelayedMotionEvent will be 0.
 */

static Tk_Event delayedMotionEvent;
static int haveDelayedMotionEvent;

/*
 * For each timer callback that's pending, there is one record
 * of the following type, chained together in a list sorted by
 * time (earliest event first).
 */

typedef struct TimerEvent {
    Tk_Time time;		/* When timer is to fire. */
    void (*proc)  _ANSI_ARGS_((ClientData clientData));
				/* Procedure to call. */
    ClientData clientData;	/* Argument to pass to proc. */
    Tk_TimerToken token;	/* Identifies event so it can be
				 * deleted. */
    struct TimerEvent *nextPtr;	/* Next event in queue, or NULL for
				 * end of queue. */
} TimerEvent;

static TimerEvent *firstTimerHandlerPtr;
				/* First event in queue. */

/*
 * For each file registered in a call to Tk_CreateFileHandler,
 * and for each display that's currently active, there is one
 * record of the following type.  All of these records are
 * chained together into a single list.
 */

typedef struct FileHandler {
    int fd;			/* POSIX file descriptor for file. */
    int mask;			/* Mask of desired events: TK_READABLE, etc. */
    Tk_FileProc *proc;		/* Procedure to call, in the style of
				 * Tk_CreateFileHandler.  This is NULL
				 * if the handler was created by
				 * Tk_CreateFileHandler2. */
    Tk_FileProc2 *proc2;	/* Procedure to call, in the style of
				 * Tk_CreateFileHandler2.  NULL means that
				 * the handler was created by
				 * Tk_CreateFileHandler. */
    ClientData clientData;	/* Argument to pass to proc. */
    struct FileHandler *nextPtr;/* Next in list of all files we
				 * care about (NULL for end of
				 * list). */
} FileHandler;

static FileHandler *firstFileHandlerPtr;
				/* List of all file events. */

/*
 * One of the following structures corresponds to each active call to
 * a Tk_CreateFileHandler2 handler procedure.  These are needed to detect
 * when handlers are deleted during a callback, so that Tk_ServiceEvent
 * doesn't use the FileHandler structure anymore.
 */

typedef struct ActiveFile {
    FileHandler *filePtr;	/* Address of the FileHandler
				 * structure whose proc2 procedure
				 * is active.  Set to NULL if the
				 * handler is deleted. */
    struct ActiveFile *nextPtr;	/* Next in list of all active
				 * handlers, or NULL for end of list. */
} ActiveFile;

static ActiveFile *firstActiveFilePtr = NULL;
					/* First in list of active handlers,
					 * or NULL. */

/*
 * There is one of the following structures for each of the
 * handlers declared in a call to Tk_DoWhenIdle.  All of the
 * currently-active handlers are linked together into a list.
 */

typedef struct IdleHandler {
    void (*proc)  _ANSI_ARGS_((ClientData clientData));
				/* Procedure to call. */
    ClientData clientData;	/* Value to pass to proc. */
    int generation;		/* Used to distinguish older handlers from
				 * recently-created ones. */
    struct IdleHandler *nextPtr;/* Next in list of active handlers. */
} IdleHandler;

static IdleHandler *idleList = NULL;
				/* First in list of all idle handlers. */
static IdleHandler *lastIdlePtr = NULL;
				/* Last in list (or NULL for empty list). */
static int idleGeneration = 0;	/* Used to fill in the "generation" fields
				 * of IdleHandler structures.  Increments
				 * each time Tk_DoOneEvent starts calling
				 * idle handlers, so that all old handlers
				 * can be called without calling any of the
				 * new ones created by old ones. */

/*
 * One of the following structures exists for each file with a handler
 * created by the "fileevent" command.  Several of the fields are
 * two-element arrays, in which the first element is used for read
 * events and the second for write events.
 */

typedef struct FileEvent {
    FILE *f;				/* Stdio handle for file. */
    Tcl_Interp *interps[2];		/* Interpreters in which to execute
					 * scripts.  NULL means no handler
					 * for event. */
    char *scripts[2];			/* Scripts to evaluate in response to
					 * events (malloc'ed).  NULL means no
					 * handler for event. */
    struct FileEvent *nextPtr;		/* Next in list of all file events
					 * currently defined. */
} FileEvent;

static FileEvent *firstFileEventPtr = NULL;
					/* First in list of all existing
					 * file events. */

/*
 * The data structure below is used by the "after" command to remember
 * the command to be executed later.
 */

typedef struct AfterInfo {
    Tcl_Interp *interp;		/* Interpreter in which to execute command. */
    char *command;		/* Command to execute.  Malloc'ed, so must
				 * be freed when structure is deallocated. */
    int id;			/* Integer identifier for command;  used to
				 * cancel it. */
    Tk_TimerToken token;	/* Used to cancel the "after" command.  NULL
				 * means that the command is run as an
				 * idle handler rather than as a timer
				 * handler. */
    struct AfterInfo *nextPtr;	/* Next in list of all "after" commands for
				 * the application. */
} AfterInfo;

static AfterInfo *firstAfterPtr = NULL;
				/* First in list of all pending "after"
				 * commands. */

/*
 * The data structure below is used to report background errors.  One
 * such structure is allocated for each error;  it holds information
 * about the interpreter and the error until tkerror can be invoked
 * later as an idle handler.
 */

typedef struct BgError {
    Tcl_Interp *interp;		/* Interpreter in which error occurred.  NULL
				 * means this error report has been cancelled
				 * (a previous report generated a break). */
    char *errorMsg;		/* The error message (interp->result when
				 * the error occurred).  Malloc-ed. */
    char *errorInfo;		/* Value of the errorInfo variable
				 * (malloc-ed). */
    char *errorCode;		/* Value of the errorCode variable
				 * (malloc-ed). */
    struct BgError *nextPtr;	/* Next in list of all pending error
				 * reports. */
} BgError;

static BgError *firstBgPtr = NULL;
				/* First in list of all background errors
				 * waiting to be processed (NULL if none). */
static BgError *lastBgPtr = NULL;
				/* First in list of all background errors
				 * waiting to be processed (NULL if none). */

/*
 * If someone has called Tk_RestrictEvents, the information below
 * keeps track of it.
 */

static Tk_RestrictProc *restrictProc;
				/* Procedure to call.  NULL means no
				 * restrictProc is currently in effect. */
static ClientData restrictArg;	/* Argument to pass to restrictProc. */

/*
 * Prototypes for procedures referenced only in this file:
 */

static void		AfterProc _ANSI_ARGS_((ClientData clientData));
static void		DeleteFileEvent _ANSI_ARGS_((FILE *f));
static int		FileEventProc _ANSI_ARGS_((ClientData clientData,
			    int mask, int flags));
static void		FreeAfterPtr _ANSI_ARGS_((AfterInfo *afterPtr));
static AfterInfo *	GetAfterEvent _ANSI_ARGS_((Tcl_Interp *interp,
			    char *string));
static void		HandleBgErrors _ANSI_ARGS_((ClientData clientData));
static void		InsertEvent _ANSI_ARGS_ ((Tk_Event *event,
			    Tk_QueuePosition position));
static void		MotionIdleHandler _ANSI_ARGS_((
    			    ClientData clientData));
static int		TkwaitCmd2 _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static int		UpdateCmd2 _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static char *		WaitVariableProc2 _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, char *name1, char *name2,
			    int flags));

/*
 *----------------------------------------------------------------------
 *
 * InsertEvent --
 *
 *	Inserts an event into the event queue in one of three
 *	positions: the head, the tail, or before a floating marker.
 *	Events inserted before the marker will be processed in
 *	first-in-first-out order, but before any events inserted at
 *	the tail of the queue.  Events inserted at the head of the
 *	queue will be processed in last-in-first-out order.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Allocates a new EventList entry, copies the event, and inserts
 *	the entry into the queue.
 *
 *----------------------------------------------------------------------
 */

static void
InsertEvent(event, position)
    Tk_Event* event;		/* Event to add to queue. */
    Tk_QueuePosition position;	/* One of TK_QUEUE_TAIL, TK_QUEUE_HEAD,
				 * TK_QUEUE_MARK. */
{
    EventList *entry;
    entry = (EventList *) ckalloc(sizeof(EventList));
    entry->event = *event;

    if (position == TK_QUEUE_TAIL) {

	/*
	 * Append the event on the end of the queue.
	 */

	entry->nextPtr = NULL;
	if (firstEventPtr == NULL) {
	    firstEventPtr = entry;
	} else {
	    lastEventPtr->nextPtr = entry;
	}
	lastEventPtr = entry;

    } else if (position == TK_QUEUE_HEAD) {

	/*
	 * Push the event on the head of the queue.
	 */

	entry->nextPtr = firstEventPtr;
	if (firstEventPtr == NULL) {
	    lastEventPtr = entry;
	}	    
	firstEventPtr = entry;

    } else if (position == TK_QUEUE_MARK) {

	/*
	 * Insert the event after the current marker event and advance
	 * the marker to the new event.
	 */

	if (markerEventPtr == NULL) {
	    entry->nextPtr = firstEventPtr;
	    firstEventPtr = entry;
	} else {
	    entry->nextPtr = markerEventPtr->nextPtr;
	    markerEventPtr->nextPtr = entry;
	}
	markerEventPtr = entry;
	if (entry->nextPtr == NULL) {
	    lastEventPtr = entry;
	}
    }
}

/*
 *--------------------------------------------------------------
 *
 * Tk_CreateFileHandler --
 *
 *	Arrange for a given procedure to be invoked whenever
 *	a given file becomes readable or writable.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	From now on, whenever the I/O channel given by fd becomes
 *	ready in the way indicated by mask, proc will be invoked.
 *	See the manual entry for details on the calling sequence
 *	to proc.  If fd is already registered then the old mask
 *	and proc and clientData values will be replaced with
 *	new ones.
 *
 *--------------------------------------------------------------
 */

void
Tk_CreateFileHandler(fd, mask, proc, clientData)
    int fd;			/* Integer identifier for stream. */
    int mask;			/* OR'ed combination of TK_READABLE,
				 * TK_WRITABLE, and TK_EXCEPTION:
				 * indicates conditions under which
				 * proc should be called.  TK_IS_DISPLAY
				 * indicates that this is a display and that
				 * clientData is the (Display *) for it,
				 * and that events should be handled
				 * automatically.*/
    Tk_FileProc *proc;		/* Procedure to call for each
				 * selected event. */
    ClientData clientData;	/* Arbitrary data to pass to proc. */
{
    register FileHandler *filePtr;
    EventList *entry, *prevPtr;

    if (fd >= FD_SETSIZE) {
	panic("Tk_CreatefileHandler can't handle file id %d", fd);
    }

    /*
     * Make sure the file isn't already registered.  Create a
     * new record in the normal case where there's no existing
     * record.
     */

    for (filePtr = firstFileHandlerPtr; filePtr != NULL;
	    filePtr = filePtr->nextPtr) {
	if (filePtr->fd == fd) {
	    break;
	}
    }
    if (filePtr == NULL) {
	filePtr = (FileHandler *) ckalloc(sizeof(FileHandler));
	filePtr->fd = fd;
	filePtr->nextPtr = firstFileHandlerPtr;
	firstFileHandlerPtr = filePtr;
    }

    /*
     * Remove any stale file events.
     */
    entry = firstEventPtr;
    prevPtr = NULL;
    while (entry != NULL) {
	if ((entry->event.type == TK_FILE_EVENTS)
		&& (entry->event.file.fd == fd)) {
	    if (prevPtr != NULL) {
		prevPtr->nextPtr = entry->nextPtr;
		ckfree((char*) entry);
		entry = prevPtr->nextPtr;
	    } else {
		firstEventPtr = entry->nextPtr;
		ckfree((char*) entry);
		entry = firstEventPtr;
	    }
	    if (entry == NULL) {
		lastEventPtr = prevPtr;
	    }
	} else {
	    prevPtr = entry;
	    entry = entry->nextPtr;
	}
    }
		

    /*
     * The remainder of the initialization below is done
     * regardless of whether or not this is a new record
     * or a modification of an old one.
     */

    filePtr->mask = mask;
    filePtr->proc = proc;
    filePtr->proc2 = NULL;
    filePtr->clientData = clientData;
    Tk_NotifyFile(fd, mask);
}

/*
 *--------------------------------------------------------------
 *
 * Tk_CreateFileHandler2 --
 *
 *	Arrange for a given procedure to be invoked during the
 *	event loop to handle a particular file.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	In each pass through Tk_ServiceEvent, proc will be invoked to
 *	decide whether fd is "ready" and take appropriate action if
 *	it is.  See the manual entry for details on the calling
 *	sequence to proc.  If a handler for fd has already been
 *	registered then it is superseded by the new one.
 *
 *--------------------------------------------------------------
 */

void
Tk_CreateFileHandler2(fd, proc, clientData)
    int fd;			/* Integer identifier for stream. */
    Tk_FileProc2 *proc;		/* Procedure to call from the event
				 * dispatcher. */
    ClientData clientData;	/* Arbitrary data to pass to proc. */
{
    register FileHandler *filePtr;

    /*
     * Let Tk_CreateFileHandler do all of the work of setting up
     * the handler, then just modify things a bit after it returns.
     */

    Tk_CreateFileHandler(fd, 0, (Tk_FileProc *) NULL, clientData);
    for (filePtr = firstFileHandlerPtr; filePtr->fd != fd;
	    filePtr = filePtr->nextPtr) {
	/* Empty loop body. */
    }
    filePtr->proc = NULL;
    filePtr->proc2 = proc;
}

/*
 *--------------------------------------------------------------
 *
 * Tk_DeleteFileHandler --
 *
 *	Cancel a previously-arranged callback arrangement for
 *	a file.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	If a callback was previously registered on fd, remove it.
 *
 *--------------------------------------------------------------
 */

void
Tk_DeleteFileHandler(fd)
    int fd;			/* Stream id for which to remove
				 * callback procedure. */
{
    FileHandler *filePtr, *prevPtr;
    ActiveFile *activePtr;

    /*
     * Find the entry for the given file (and return if there
     * isn't one).
     */

    for (prevPtr = NULL, filePtr = firstFileHandlerPtr; ;
	    prevPtr = filePtr, filePtr = filePtr->nextPtr) {
	if (filePtr == NULL) {
	    return;
	}
	if (filePtr->fd == fd) {
	    break;
	}
    }

    /*
     * See if a callback is active for the handler.  If so, mark
     * the callback record to indicate that the handler has been
     * deleted.
     */

    for (activePtr = firstActiveFilePtr; activePtr != NULL;
	    activePtr = activePtr->nextPtr) {
	if (activePtr->filePtr == filePtr) {
	    activePtr->filePtr = NULL;
	    break;
	}
    }

    /*
     * Clean up information in the callback record.
     */

    if (prevPtr == NULL) {
	firstFileHandlerPtr = filePtr->nextPtr;
    } else {
	prevPtr->nextPtr = filePtr->nextPtr;
    }

    Tk_IgnoreFile(filePtr->fd);

    ckfree((char *) filePtr);
}

/*
 *--------------------------------------------------------------
 *
 * Tk_CreateTimerHandler --
 *
 *	Arrange for a given procedure to be invoked at a particular
 *	time in the future.
 *
 * Results:
 *	The return value is a token for the timer event, which
 *	may be used to delete the event before it fires.
 *
 * Side effects:
 *	When milliseconds have elapsed, proc will be invoked
 *	exactly once.
 *
 *--------------------------------------------------------------
 */

Tk_TimerToken
Tk_CreateTimerHandler(milliseconds, proc, clientData)
    int milliseconds;		/* How many milliseconds to wait
				 * before invoking proc. */
    Tk_TimerProc *proc;		/* Procedure to invoke. */
    ClientData clientData;	/* Arbitrary data to pass to proc. */
{
    register TimerEvent *timerPtr, *tPtr2, *prevPtr;
    struct timeval curTime;
    static int id = 0;

    timerPtr = (TimerEvent *) ckalloc(sizeof(TimerEvent));

    /*
     * Compute when the event should fire.
     */

    (void) gettimeofday(&curTime, (struct timezone *) NULL);
    curTime.tv_sec += milliseconds/1000;
    curTime.tv_usec += (milliseconds%1000)*1000;
    if (curTime.tv_usec >= 1000000) {
	curTime.tv_usec -= 1000000;
	curTime.tv_sec += 1;
    }
    timerPtr->time.sec = curTime.tv_sec;
    timerPtr->time.usec = curTime.tv_usec;
    
    /*
     * Fill in other fields for the event.
     */

    timerPtr->proc = proc;
    timerPtr->clientData = clientData;
    id++;
    timerPtr->token = (Tk_TimerToken) id;

    /*
     * Add the event to the queue in the correct position
     * (ordered by event firing time).
     */

    for (tPtr2 = firstTimerHandlerPtr, prevPtr = NULL; tPtr2 != NULL;
	    prevPtr = tPtr2, tPtr2 = tPtr2->nextPtr) {
	if ((tPtr2->time.sec > timerPtr->time.sec)
		|| ((tPtr2->time.sec == timerPtr->time.sec)
		&& (tPtr2->time.usec > timerPtr->time.usec))) {
	    break;
	}
    }

    /*
     * If the timer is the earliest event, then we need to give a new
     * timeout to the notifier.
     */

    if (prevPtr == NULL) {
	timerPtr->nextPtr = firstTimerHandlerPtr;
	firstTimerHandlerPtr = timerPtr;
	Tk_NotifyTimer(&timerPtr->time);
    } else {
	timerPtr->nextPtr = prevPtr->nextPtr;
	prevPtr->nextPtr = timerPtr;
    }
    return timerPtr->token;
}

/*
 *--------------------------------------------------------------
 *
 * Tk_DeleteTimerHandler --
 *
 *	Delete a previously-registered timer handler.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Destroy the timer callback identified by TimerToken,
 *	so that its associated procedure will not be called.
 *	If the callback has already fired, or if the given
 *	token doesn't exist, then nothing happens.
 *
 *--------------------------------------------------------------
 */

void
Tk_DeleteTimerHandler(token)
    Tk_TimerToken token;	/* Result previously returned by
				 * Tk_DeleteTimerHandler. */
{
    register TimerEvent *timerPtr, *prevPtr;

    for (timerPtr = firstTimerHandlerPtr, prevPtr = NULL; timerPtr != NULL;
	    prevPtr = timerPtr, timerPtr = timerPtr->nextPtr) {
	if (timerPtr->token != token) {
	    continue;
	}
	if (prevPtr == NULL) {
	    firstTimerHandlerPtr = timerPtr->nextPtr;

	    /*
	     * We only need to update the notifier timeout if the
	     * deleted timer was the youngest.  If there are no other
	     * timers, then we will let the timer expire and ignore
	     * the event.
	     */

	    if (firstTimerHandlerPtr != NULL) {
		Tk_NotifyTimer(&firstTimerHandlerPtr->time);
	    }
	} else {
	    prevPtr->nextPtr = timerPtr->nextPtr;
	}
	ckfree((char *) timerPtr);
	return;
    }
}

/*
 *--------------------------------------------------------------
 *
 * Tk_DoWhenIdle --
 *
 *	Arrange for proc to be invoked the next time the
 *	system is idle (i.e., just before the next time
 *	that Tk_DoOneEvent would have to wait for something
 *	to happen).
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Proc will eventually be called, with clientData
 *	as argument.  See the manual entry for details.
 *
 *--------------------------------------------------------------
 */

void
Tk_DoWhenIdle(proc, clientData)
    Tk_IdleProc *proc;		/* Procedure to invoke. */
    ClientData clientData;	/* Arbitrary value to pass to proc. */
{
    register IdleHandler *idlePtr;

    idlePtr = (IdleHandler *) ckalloc(sizeof(IdleHandler));
    idlePtr->proc = proc;
    idlePtr->clientData = clientData;
    idlePtr->generation = idleGeneration;
    idlePtr->nextPtr = NULL;
    if (lastIdlePtr == NULL) {
	idleList = idlePtr;
    } else {
	lastIdlePtr->nextPtr = idlePtr;
    }
    lastIdlePtr = idlePtr;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_CancelIdleCall --
 *
 *	If there are any when-idle calls requested to a given procedure
 *	with given clientData, cancel all of them.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	If the proc/clientData combination were on the when-idle list,
 *	they are removed so that they will never be called.
 *
 *----------------------------------------------------------------------
 */

void
Tk_CancelIdleCall(proc, clientData)
    Tk_IdleProc *proc;		/* Procedure that was previously registered. */
    ClientData clientData;	/* Arbitrary value to pass to proc. */
{
    register IdleHandler *idlePtr, *prevPtr;
    IdleHandler *nextPtr;

    for (prevPtr = NULL, idlePtr = idleList; idlePtr != NULL;
	    prevPtr = idlePtr, idlePtr = idlePtr->nextPtr) {
	while ((idlePtr->proc == proc)
		&& (idlePtr->clientData == clientData)) {
	    nextPtr = idlePtr->nextPtr;
	    ckfree((char *) idlePtr);
	    idlePtr = nextPtr;
	    if (prevPtr == NULL) {
		idleList = idlePtr;
	    } else {
		prevPtr->nextPtr = idlePtr;
	    }
	    if (idlePtr == NULL) {
		lastIdlePtr = prevPtr;
		return;
	    }
	}
    }
}

/*
 *----------------------------------------------------------------------
 *
 * MotionIdleHandler --
 *
 *	This idle handler is registered by Tk_QueueEvent in order to
 *	defer the processing of motion events so multiple consecutive
 *	motion events can be collapsed.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Inserts a motion event into the event queue.
 *
 *----------------------------------------------------------------------
 */

static void
MotionIdleHandler(clientData)
    ClientData clientData;
{
    /*
     * Now that we are idle, we need to process any delayed motion events.
     * Note that we are only queueing the event, not processing it.
     * This is not strictly 
     */

    if (haveDelayedMotionEvent) {
	InsertEvent(&delayedMotionEvent, TK_QUEUE_TAIL);
	haveDelayedMotionEvent = 0;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_QueueEvent --
 *
 *	Insert an event into the Tk event queue at the specified
 *	position.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	This routine performs X motion event collapsing as well as
 *	inserting events into the queue.
 *
 *----------------------------------------------------------------------
 */

void
Tk_QueueEvent(event, position)
    Tk_Event* event;		/* Event to add to queue. */
    Tk_QueuePosition position;	/* One of TK_QUEUE_TAIL, TK_QUEUE_HEAD,
				 * TK_QUEUE_MARK. */
{
    /*
     * Deal with mouse-motion-collapsing and event-delaying here.   If
     * there's already a delayed event, then add that event to the
     * queue if it's incompatible with the new event (new event not
     * mouse motion, or window changed, or state changed).  If the new
     * event is mouse motion, then delay the event so that it can be
     * merged with immediately following motion events.  We only collapse
     * or defer events which are added to the end of the queue.
     */

    if ((event->type == TK_WINDOW_EVENTS) && (position == TK_QUEUE_TAIL)) {
	if (haveDelayedMotionEvent) {
	    if (((event->window.type != MotionNotify)
			&& (event->window.type != GraphicsExpose)
			&& (event->window.type != NoExpose)
			&& (event->window.type != Expose))
		    || (event->window.event.xany.display
			    != delayedMotionEvent.window.event.xany.display)
		    || (event->window.event.xany.window
			    != delayedMotionEvent.window.event.xany.window)) {
		haveDelayedMotionEvent = 0;
		Tk_CancelIdleCall(MotionIdleHandler, &haveDelayedMotionEvent);
		InsertEvent(&delayedMotionEvent, TK_QUEUE_TAIL);
	    }
	}
	if (event->window.type == MotionNotify) {
	    haveDelayedMotionEvent = 1;
	    delayedMotionEvent = *event;
	    Tk_DoWhenIdle(MotionIdleHandler, &haveDelayedMotionEvent);
	    return;
	}
    }
    InsertEvent(event, position);
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_ServiceEvent --
 *
 *	Process one event from Tk's internal event queue.  This
 *	routine is called by the notifier whenever it wants Tk to
 *	process an event.  
 *
 * Results:
 *
 *	The return value is 1 if the procedure actually found an event
 *	to process.  If no processing occurred, then 0 is returned.
 *	The notifier should keep calling Tk_ServiceEvent until it
 *	returns 0.
 *
 * Side effects:
 *	Invokes all of the event handlers for the highest priority
 *	event in the event queue.  May collapse some events into a
 *	single event or discard stale events.
 *
 *----------------------------------------------------------------------
 */

int
Tk_ServiceEvent(flags)
    int flags;			/* May be any combination of
				 * TK_WINDOW_EVENTS, TK_FILE_EVENTS,
				 * TK_TIMER_EVENTS. */
{
    FileHandler *filePtr;
    EventList *entry, *prevPtr, *nextPtr;

    /*
     * No event flags is equivalent to TK_ALL_EVENTS.
     */
    
    if ((flags & TK_ALL_EVENTS) == 0) {
	flags |= TK_ALL_EVENTS;
    }

    for (entry = firstEventPtr, prevPtr = NULL; entry != NULL;
	    entry = nextPtr) {

	/*
	 * We need to remember the nextPtr before we go any further, since
	 * entry will get deleted if the current event is handled.
	 */
	nextPtr = entry->nextPtr;

	/*
	 * Skip masked out events.
	 */
	if (!(entry->event.type & flags)) {
	    prevPtr = entry;
	    continue;
	}

	/*
	 * If there is a restrict procedure, we need to find out
	 * the disposition of this event.
	 */
	
	if ((restrictProc != NULL)
		&& (entry->event.type == TK_WINDOW_EVENTS)) {
	    Tk_RestrictAction result;

	    result = (*restrictProc)(restrictArg, &entry->event.window.event);
	    if (result != TK_PROCESS_EVENT) {
		if (result == TK_DEFER_EVENT) {
		    prevPtr = entry;
		    continue;
		} else {

		    /*
		     * Mark the event with a bogus event type so it will be
		     * discarded.
		     */

		    entry->event.type = 0;
		}
	    }
	}

	/*
	 * Remove event from list before handling the event to avoid
	 * recursion problems.
	 */

	if (prevPtr == NULL) {
	    firstEventPtr = nextPtr;
	} else {
	    prevPtr->nextPtr = nextPtr;
	}
	if (nextPtr == NULL) {
	    lastEventPtr = prevPtr;
	}
	if (markerEventPtr == entry) {
	    markerEventPtr = NULL;
	}

	switch (entry->event.type) {
	case TK_TIMER_EVENTS: {
	    register TimerEvent *timerPtr = firstTimerHandlerPtr;
	    register Tk_Time *eventTime = &entry->event.timer.time;
	    if (timerPtr != NULL) {

		/*
		 * Check to see if the first timer has expired.  We
		 * need to check because there could be stale timers.
		 */

		if ((timerPtr->time.sec < eventTime->sec)
			|| ((timerPtr->time.sec == eventTime->sec)
			&&  (timerPtr->time.usec <= eventTime->usec))) {
		    firstTimerHandlerPtr = timerPtr->nextPtr;
		    (*timerPtr->proc)(timerPtr->clientData);
		    ckfree((char *) timerPtr);
		}

		/*
		 * Reset the timer for the next handler on the list.
		 */

		if (firstTimerHandlerPtr != NULL) {
		    Tk_NotifyTimer(&firstTimerHandlerPtr->time);
		}
		ckfree((char *) entry);
		return 1;
	    }
	    break;
	}
	case TK_FILE_EVENTS: {

	    /*
	     * Find the file handler for this event.
	     */

	    for (filePtr = firstFileHandlerPtr; filePtr != NULL;
	    	    filePtr = filePtr->nextPtr) {
		if (filePtr->fd == entry->event.file.fd) {
		    break;
		}
	    }

	    /*
	     * Note that we don't bother checking against the handlers mask
	     * because any stale events are deleted by Tk_CreateFileHandler.
	     */

	    if (filePtr != NULL) {
		if (filePtr->proc != NULL) {
		    /*
		     * We reregister the file before calling the proc so
		     * we don't have to worry about cleaning up if the
		     * handler gets deleted.
		     */

		    Tk_NotifyFile(filePtr->fd, filePtr->mask);
		    (*filePtr->proc)(filePtr->clientData,
			    entry->event.file.mask);
		    ckfree((char *) entry);
		    return 1;
		} else {
		    /*
		     * Careful!  The file handler could get deleted during
		     * the callback.  Register the handler so we'll know if
		     * it's been deleted,  then don't touch it after proc2
		     * returns if it has been deleted.
		     */

		    ActiveFile active;
		    int newMask;

		    active.filePtr = filePtr;
		    active.nextPtr = firstActiveFilePtr;
		    firstActiveFilePtr = &active;
		    newMask = (*filePtr->proc2)(filePtr->clientData,
			    entry->event.file.mask, flags);
		    firstActiveFilePtr = active.nextPtr;
		    if (active.filePtr != NULL) {
			filePtr->mask = newMask;
			if (filePtr->mask != TK_FILE_HANDLED) {
			    Tk_NotifyFile(filePtr->fd, filePtr->mask);
			}
		    }
		    ckfree((char *) entry);
		    return 1;
		}
	    }
	    break;
	}

	case TK_WINDOW_EVENTS:
	    if (tkHandleEventProc != NULL) {
		(*tkHandleEventProc)(&entry->event.window.event);
	    }
	    ckfree((char *) entry);
	    return 1;

	default:

	    /*
	     * Discard the event because we don't understand it.
	     */

	    break;
	}

	/*
	 * Dispose of the event and continue with the next one.
	 */

	ckfree((char *) entry);
    }

    /*
     * We also need to call all of the Tk_FileProc2 procedures to see if
     * there is any more work to be done.
     */

    for (filePtr = firstFileHandlerPtr; filePtr != NULL;
	    filePtr = filePtr->nextPtr) {
	if (filePtr->proc2 != NULL) {
	    filePtr->mask = (*filePtr->proc2)(filePtr->clientData, 0, flags);
	    if (filePtr->mask != TK_FILE_HANDLED) {
		Tk_NotifyFile(filePtr->fd, filePtr->mask);
	    } else {
		return 1;
	    }
	}
    }
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_IdlePending --
 *
 *	This function is called by the notifier subsystem to determine
 *	whether there are any idle handlers currently scheduled.
 *
 * Results:
 *	Returns 0 if the idle list is empty, otherwise it returns 1.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
Tk_IdlePending()
{
    return (idleList == NULL) ? 0 : 1;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_ServiceIdle --
 *
 *	This procedure is invoked by the notifier when it becomes idle.
 *
 * Results:
 *	The return value is 1 if the procedure actually found an idle
 *	handler to invoke.  If no handler was found then 0 is returned.
 *
 * Side effects:
 *	Invokes all idle handlers.
 *
 *----------------------------------------------------------------------
 */

int
Tk_ServiceIdle()
{
    IdleHandler *idlePtr;
    int oldGeneration;
    int foundIdle;

    if (idleList == NULL) {
	return 0;
    }
    
    foundIdle = 0;
    oldGeneration = idleGeneration;
    idleGeneration++;

    /*
     * The code below is trickier than it may look, for the following
     * reasons:
     *
     * 1. New handlers can get added to the list while the current
     *    one is being processed.  If new ones get added, we don't
     *    want to process them during this pass through the list (want
     *    to check for other work to do first).  This is implemented
     *    using the generation number in the handler:  new handlers
     *    will have a different generation than any of the ones currently
     *    on the list.
     * 2. The handler can call Tk_DoOneEvent, so we have to remove
     *    the handler from the list before calling it. Otherwise an
     *    infinite loop could result.
     * 3. Tk_CancelIdleCall can be called to remove an element from
     *    the list while a handler is executing, so the list could
     *    change structure during the call.
     */

    for (idlePtr = idleList;
	    ((idlePtr != NULL)
		    && ((oldGeneration - idlePtr->generation) >= 0));
	    idlePtr = idleList) {
	idleList = idlePtr->nextPtr;
	if (idleList == NULL) {
	    lastIdlePtr = NULL;
	}
	foundIdle = 1;
	(*idlePtr->proc)(idlePtr->clientData);
	ckfree((char *) idlePtr);
    }

    return foundIdle;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_BackgroundError --
 *
 *	This procedure is invoked to handle errors that occur in Tcl
 *	commands that are invoked in "background" (e.g. from event or
 *	timer bindings).
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The command "tkerror" is invoked later as an idle handler to
 *	process the error, passing it the error message.  If that fails,
 *	then an error message is output on stderr.
 *
 *----------------------------------------------------------------------
 */

void
Tk_BackgroundError(interp)
    Tcl_Interp *interp;		/* Interpreter in which an error has
				 * occurred. */
{
    BgError *errPtr;
    char *varValue;

    /*
     * The Tcl_AddErrorInfo call below (with an empty string) ensures that
     * errorInfo gets properly set.  It's needed in cases where the error
     * came from a utility procedure like Tcl_GetVar instead of Tcl_Eval;
     * in these cases errorInfo still won't have been set when this
     * procedure is called.
     */

    Tcl_AddErrorInfo(interp, "");
    errPtr = (BgError *) ckalloc(sizeof(BgError));
    errPtr->interp = interp;
    errPtr->errorMsg = (char *) ckalloc((unsigned) (strlen(interp->result)
	    + 1));
    strcpy(errPtr->errorMsg, interp->result);
    varValue = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    if (varValue == NULL) {
	varValue = errPtr->errorMsg;
    }
    errPtr->errorInfo = (char *) ckalloc((unsigned) (strlen(varValue) + 1));
    strcpy(errPtr->errorInfo, varValue);
    varValue = Tcl_GetVar(interp, "errorCode", TCL_GLOBAL_ONLY);
    if (varValue == NULL) {
	varValue = "";
    }
    errPtr->errorCode = (char *) ckalloc((unsigned) (strlen(varValue) + 1));
    strcpy(errPtr->errorCode, varValue);
    errPtr->nextPtr = NULL;
    if (firstBgPtr == NULL) {
	firstBgPtr = errPtr;
	Tk_DoWhenIdle(HandleBgErrors, (ClientData) NULL);
    } else {
	lastBgPtr->nextPtr = errPtr;
    }
    lastBgPtr = errPtr;
    Tcl_ResetResult(interp);
}

/*
 *----------------------------------------------------------------------
 *
 * HandleBgErrors --
 *
 *	This procedure is invoked as an idle handler to process all of
 *	the accumulated background errors.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Depends on what actions "tkerror" takes for the errors.
 *
 *----------------------------------------------------------------------
 */

static void
HandleBgErrors(clientData)
    ClientData clientData;		/* Not used. */
{
    Tcl_Interp *interp;
    char *command;
    char *argv[2];
    int code;
    BgError *errPtr;

    while (firstBgPtr != NULL) {
	interp = firstBgPtr->interp;
	if (interp == NULL) {
	    goto doneWithReport;
	}

	/*
	 * Restore important state variables to what they were at
	 * the time the error occurred.
	 */

	Tcl_SetVar(interp, "errorInfo", firstBgPtr->errorInfo,
		TCL_GLOBAL_ONLY);
	Tcl_SetVar(interp, "errorCode", firstBgPtr->errorCode,
		TCL_GLOBAL_ONLY);

	/*
	 * Create and invoke the tkerror command.
	 */

	argv[0] = "tkerror";
	argv[1] = firstBgPtr->errorMsg;
	command = Tcl_Merge(2, argv);
	Tcl_AllowExceptions(interp);
	code = Tcl_GlobalEval(interp, command);
	ckfree(command);
	if (code == TCL_ERROR) {
	    if (strcmp(interp->result, "\"tkerror\" is an invalid command name or ambiguous abbreviation") == 0) {
		fprintf(stderr, "%s\n", firstBgPtr->errorInfo);
	    } else {
		fprintf(stderr, "tkerror failed to handle background error.\n");
		fprintf(stderr, "    Original error: %s\n",
			firstBgPtr->errorMsg);
		fprintf(stderr, "    Error in tkerror: %s\n", interp->result);
	    }
	} else if (code == TCL_BREAK) {
	    /*
	     * Break means cancel any remaining error reports for this
	     * interpreter.
	     */

	    for (errPtr = firstBgPtr; errPtr != NULL;
		    errPtr = errPtr->nextPtr) {
		if (errPtr->interp == interp) {
		    errPtr->interp = NULL;
		}
	    }
	}

	/*
	 * Discard the command and the information about the error report.
	 */

	doneWithReport:
	ckfree(firstBgPtr->errorMsg);
	ckfree(firstBgPtr->errorInfo);
	ckfree(firstBgPtr->errorCode);
	errPtr = firstBgPtr->nextPtr;
	ckfree((char *) firstBgPtr);
	firstBgPtr = errPtr;
    }
    lastBgPtr = NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_AfterCmd --
 *
 *	This procedure is invoked to process the "after" Tcl command.
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
Tk_AfterCmd(clientData, interp, argc, argv)
    ClientData clientData;	/* Main window associated with
				 * interpreter.  Not used.*/
    Tcl_Interp *interp;		/* Current interpreter. */
    int argc;			/* Number of arguments. */
    char **argv;		/* Argument strings. */
{
    /*
     * The variable below is used to generate unique identifiers for
     * after commands.  This id can wrap around, which can potentially
     * cause problems.  However, there are not likely to be problems
     * in practice, because after commands can only be requested to
     * about a month in the future, and wrap-around is unlikely to
     * occur in less than about 1-10 years.  Thus it's unlikely that
     * any old ids will still be around when wrap-around occurs.
     */

    static int nextId = 1;
    int ms;
    AfterInfo *afterPtr;
    size_t length;

    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " option ?arg arg ...?\"", (char *) NULL);
	return TCL_ERROR;
    }

    length = strlen(argv[1]);
    if (isdigit(UCHAR(argv[1][0]))) {
	if (Tcl_GetInt(interp, argv[1], &ms) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (ms < 0) {
	    ms = 0;
	}
	if (argc == 2) {
	    Tk_Sleep(ms);
	    return TCL_OK;
	}
	afterPtr = (AfterInfo *) ckalloc((unsigned) (sizeof(AfterInfo)));
	afterPtr->interp = interp;
	if (argc == 3) {
	    afterPtr->command = (char *) ckalloc((unsigned)
		    (strlen(argv[2]) + 1));
	    strcpy(afterPtr->command, argv[2]);
	} else {
	    afterPtr->command = Tcl_Concat(argc-2, argv+2);
	}
	afterPtr->id = nextId;
	nextId += 1;
	afterPtr->token = Tk_CreateTimerHandler(ms, AfterProc,
		(ClientData) afterPtr);
	afterPtr->nextPtr = firstAfterPtr;
	firstAfterPtr = afterPtr;
	sprintf(interp->result, "after#%d", afterPtr->id);
    } else if (strncmp(argv[1], "cancel", length) == 0) {
	char *arg;

	if (argc < 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " cancel id|command\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (argc == 3) {
	    arg = argv[2];
	} else {
	    arg = Tcl_Concat(argc-2, argv+2);
	}
	for (afterPtr = firstAfterPtr; afterPtr != NULL;
		afterPtr = afterPtr->nextPtr) {
	    if (afterPtr->interp != interp) {
		continue;
	    }
	    if (strcmp(afterPtr->command, arg) == 0) {
		break;
	    }
	}
	if (afterPtr == NULL) {
	    afterPtr = GetAfterEvent(interp, arg);
	}
	if (arg != argv[2]) {
	    ckfree(arg);
	}
	if (afterPtr != NULL) {
	    if (afterPtr->token != NULL) {
		Tk_DeleteTimerHandler(afterPtr->token);
	    } else {
		Tk_CancelIdleCall(AfterProc, (ClientData) afterPtr);
	    }
	    FreeAfterPtr(afterPtr);
	}
    } else if ((strncmp(argv[1], "idle", length) == 0)
	     && (length >= 2)) {
	if (argc < 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " idle script script ...\"", (char *) NULL);
	    return TCL_ERROR;
	}
	afterPtr = (AfterInfo *) ckalloc((unsigned) (sizeof(AfterInfo)));
	afterPtr->interp = interp;
	if (argc == 3) {
	    afterPtr->command = (char *) ckalloc((unsigned)
		    (strlen(argv[2]) + 1));
	    strcpy(afterPtr->command, argv[2]);
	} else {
	    afterPtr->command = Tcl_Concat(argc-2, argv+2);
	}
	afterPtr->id = nextId;
	nextId += 1;
	afterPtr->token = NULL;
	afterPtr->nextPtr = firstAfterPtr;
	firstAfterPtr = afterPtr;
	Tk_DoWhenIdle(AfterProc, (ClientData) afterPtr);
	sprintf(interp->result, "after#%d", afterPtr->id);
    } else if ((strncmp(argv[1], "info", length) == 0)
	     && (length >= 2)) {
	if (argc == 2) {
	    char buffer[30];
	    
	    for (afterPtr = firstAfterPtr; afterPtr != NULL;
		    afterPtr = afterPtr->nextPtr) {
		if (afterPtr->interp == interp) {
		    sprintf(buffer, "after#%d", afterPtr->id);
		    Tcl_AppendElement(interp, buffer);
		}
	    }
	    return TCL_OK;
	}
	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " info ?id?\"", (char *) NULL);
	    return TCL_ERROR;
	}
	afterPtr = GetAfterEvent(interp, argv[2]);
	if (afterPtr == NULL) {
	    Tcl_AppendResult(interp, "event \"", argv[2],
		    "\" doesn't exist", (char *) NULL);
	    return TCL_ERROR;
	}
	Tcl_AppendElement(interp, afterPtr->command);
	Tcl_AppendElement(interp,
		(afterPtr->token == NULL) ? "idle" : "timer");
    } else {
	Tcl_AppendResult(interp, "bad argument \"", argv[1],
		"\": must be cancel, idle, info, or a number",
		(char *) NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * GetAfterEvent --
 *
 *	This procedure parses an "after" id such as "after#4" and
 *	returns a pointer to the AfterInfo structure.
 *
 * Results:
 *	The return value is either a pointer to an AfterInfo structure,
 *	if one is found that corresponds to "string" and is for interp,
 *	or NULL if no corresponding after event can be found.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static AfterInfo *
GetAfterEvent(interp, string)
    Tcl_Interp *interp;		/* Only events for this interpreter are
				 * considered. */
    char *string;		/* Textual identifier for after event, such
				 * as "after#6". */
{
    AfterInfo *afterPtr;
    int id;
    char *end;

    if (strncmp(string, "after#", 6) != 0) {
	return NULL;
    }
    string += 6;
    id = strtoul(string, &end, 10);
    if ((end == string) || (*end != 0)) {
	return NULL;
    }
    for (afterPtr = firstAfterPtr; afterPtr != NULL;
	    afterPtr = afterPtr->nextPtr) {
	if ((afterPtr->id == id) && (afterPtr->interp == interp)) {
	    return afterPtr;
	}
    }
    return NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * AfterProc --
 *
 *	Timer callback to execute commands registered with the
 *	"after" command.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Executes whatever command was specified.  If the command
 *	returns an error, then the command "tkerror" is invoked
 *	to process the error;  if tkerror fails then information
 *	about the error is output on stderr.
 *
 *----------------------------------------------------------------------
 */

static void
AfterProc(clientData)
    ClientData clientData;	/* Describes command to execute. */
{
    AfterInfo *afterPtr = (AfterInfo *) clientData;
    AfterInfo *prevPtr;
    int result;

    /*
     * First remove the callback from our list of callbacks;  otherwise
     * someone could delete the callback while it's being executed, which
     * could cause a core dump.
     */

    if (firstAfterPtr == afterPtr) {
	firstAfterPtr = afterPtr->nextPtr;
    } else {
	for (prevPtr = firstAfterPtr; prevPtr->nextPtr != afterPtr;
		prevPtr = prevPtr->nextPtr) {
	    /* Empty loop body. */
	}
	prevPtr->nextPtr = afterPtr->nextPtr;
    }

    /*
     * Execute the callback.
     */

    result = Tcl_GlobalEval(afterPtr->interp, afterPtr->command);
    if (result != TCL_OK) {
	Tcl_AddErrorInfo(afterPtr->interp, "\n    (\"after\" script)");
	Tk_BackgroundError(afterPtr->interp);
    }

    /*
     * Free the memory for the callback.
     */

    ckfree(afterPtr->command);
    ckfree((char *) afterPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * FreeAfterPtr --
 *
 *	This procedure removes an "after" command from the list of
 *	those that are pending and frees its resources.  This procedure
 *	does *not* cancel the timer handler;  if that's needed, the
 *	caller must do it.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The memory associated with afterPtr is released.
 *
 *----------------------------------------------------------------------
 */

static void
FreeAfterPtr(afterPtr)
    AfterInfo *afterPtr;		/* Command to be deleted. */
{
    AfterInfo *prevPtr;
    if (firstAfterPtr == afterPtr) {
	firstAfterPtr = afterPtr->nextPtr;
    } else {
	for (prevPtr = firstAfterPtr; prevPtr->nextPtr != afterPtr;
		prevPtr = prevPtr->nextPtr) {
	    /* Empty loop body. */
	}
	prevPtr->nextPtr = afterPtr->nextPtr;
    }
    ckfree(afterPtr->command);
    ckfree((char *) afterPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_FileeventCmd --
 *
 *	This procedure is invoked to process the "fileevent" Tcl
 *	command. See the user documentation for details on what it does.
 *	This command is based on Mark Diekhans' "addinput" command.
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
Tk_FileeventCmd(clientData, interp, argc, argv)
    ClientData clientData;	/* Main window associated with interpreter.
				 * Not used.*/
    Tcl_Interp *interp;		/* Current interpreter. */
    int argc;			/* Number of arguments. */
    char **argv;		/* Argument strings. */
{
    FILE *f;
    int index, c;
    size_t length;
    FileEvent *fevPtr, *prevPtr;

    /*
     * Parse arguments.
     */

    if ((argc != 3) && (argc != 4)) {
	Tcl_AppendResult(interp, "wrong # args: must be \"", argv[0],
		" fileId event ?script?", (char *) NULL);
	return TCL_ERROR;
    }
    c = argv[2][0];
    length = strlen(argv[2]);
    if ((c == 'r') && (strncmp(argv[2], "readable", length) == 0)) {
	index = 0;
    } else if ((c == 'w') && (strncmp(argv[2], "writable", length) == 0)) {
	index = 1;
    } else {
	Tcl_AppendResult(interp, "bad event name \"", argv[2],
		"\": must be readable or writable", (char *) NULL);
	return TCL_ERROR;
    }
    if (Tcl_GetOpenFile(interp, argv[1], index, 1, &f) != TCL_OK) {
	return TCL_ERROR;
    }

    /*
     * Locate an existing file handler for this file, if one exists,
     * and make a new one if none currently exists.
     */

    for (fevPtr = firstFileEventPtr; ; fevPtr = fevPtr->nextPtr) {
	if (fevPtr == NULL) {
	    if ((argc == 3) || (argv[3][0] == 0)) {
		return TCL_OK;
	    }
	    fevPtr = (FileEvent *) ckalloc(sizeof(FileEvent));
	    fevPtr->f = f;
	    fevPtr->interps[0] = NULL;
	    fevPtr->interps[1] = NULL;
	    fevPtr->scripts[0] = NULL;
	    fevPtr->scripts[1] = NULL;
	    fevPtr->nextPtr = firstFileEventPtr;
	    firstFileEventPtr = fevPtr;
	    Tk_CreateFileHandler2(fileno(f), FileEventProc,
		    (ClientData) fevPtr);
	    Tcl_SetFileCloseProc(DeleteFileEvent);
	    break;
	}
	if (fevPtr->f == f) {
	    break;
	}
    }

    /*
     * If we're just supposed to return the current script, do so.
     */

    if (argc == 3) {
	if (fevPtr->scripts[index] != NULL) {
	    interp->result = fevPtr->scripts[index];
	}
	return TCL_OK;
    }

    /*
     * If we're supposed to delete the event handler, do so.
     */

    if (argv[3][0] == 0) {
	if (fevPtr->scripts[index] != NULL) {
	    fevPtr->interps[index] = NULL;
	    ckfree(fevPtr->scripts[index]);
	    fevPtr->scripts[index] = NULL;
	}
	if ((fevPtr->scripts[0] == NULL) && (fevPtr->scripts[1] == NULL)) {
	    if (firstFileEventPtr == fevPtr) {
		firstFileEventPtr = fevPtr->nextPtr;
	    } else {
		for (prevPtr = firstFileEventPtr; prevPtr->nextPtr != fevPtr;
			prevPtr = prevPtr->nextPtr) {
		    /* Empty loop body. */
		}
		prevPtr->nextPtr = fevPtr->nextPtr;
	    }
	    Tk_DeleteFileHandler(fileno(fevPtr->f));
	    ckfree((char *) fevPtr);
	}
	return TCL_OK;
    }

    /*
     * This is a new handler being created.  Save its script.
     */

    fevPtr->interps[index] = interp;
    if (fevPtr->scripts[index] != NULL) {
	ckfree(fevPtr->scripts[index]);
    }
    fevPtr->scripts[index] = (char *) ckalloc((unsigned) (strlen(argv[3]) + 1));
    strcpy(fevPtr->scripts[index], argv[3]);
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * FileEventProc --
 *
 *	This procedure is invoked by Tk's event loop to deal with file
 *	event bindings created by the "fileevent" command.
 *
 * Results:
 *	The return value is TK_FILE_HANDLED if the file was ready and
 *	a script was invoked to handle it.  Otherwise an OR-ed combination
 *	of TK_READABLE and TK_WRITABLE is returned, indicating the events
 *	that should be checked in future calls to select.
 *
 * Side effects:
 *	Whatever the event script does.
 *
 *----------------------------------------------------------------------
 */

static int
FileEventProc(clientData, mask, flags)
    ClientData clientData;	/* Pointer to FileEvent structure for file. */
    int mask;			/* OR-ed combination of the bits TK_READABLE,
				 * TK_WRITABLE, and TK_EXCEPTION, indicating
				 * current state of file. */
    int flags;			/* Flag bits passed to Tk_DoOneEvent;
				 * contains bits such as TK_DONT_WAIT,
				 * TK_WINDOW_EVENTS, Tk_FILE_EVENTS, etc. */
{
    FileEvent *fevPtr = (FileEvent *) clientData;
    Tcl_DString script;
    Tcl_Interp *interp;
    FILE *f;
    int code, checkMask;

    if (!(flags & TK_FILE_EVENTS)) {
	return 0;
    }

    /*
     * The code here is a little tricky, because the script for an
     * event could delete the event handler.  Thus, after we call
     * Tcl_GlobalEval we can't use fevPtr anymore.  We also have to
     * copy the script to make sure that it doesn't get freed while
     * being evaluated.
     */

    checkMask = 0;
    f = fevPtr->f;
    if (fevPtr->scripts[1] != NULL) {
	if (mask & TK_WRITABLE) {
	    Tcl_DStringInit(&script);
	    Tcl_DStringAppend(&script, fevPtr->scripts[1], -1);
	    interp = fevPtr->interps[1];
	    code = Tcl_GlobalEval(interp, Tcl_DStringValue(&script));
	    Tcl_DStringFree(&script);
	    if (code != TCL_OK) {
		goto error;
	    }
	    return TK_FILE_HANDLED;
	} else {
	    checkMask |= TK_WRITABLE;
	}
    }
    if (fevPtr->scripts[0] != NULL) {
	if ((mask & TK_READABLE) || TK_READ_DATA_PENDING(f)) {
	    Tcl_DStringInit(&script);
	    Tcl_DStringAppend(&script, fevPtr->scripts[0], -1);
	    interp = fevPtr->interps[0];
	    code = Tcl_GlobalEval(interp, Tcl_DStringValue(&script));
	    Tcl_DStringFree(&script);
	    if (code != TCL_OK) {
		goto error;
	    }
	    return TK_FILE_HANDLED;
	} else {
	    checkMask |= TK_READABLE;
	}
    }
    return checkMask;

    /*
     * An error occurred in the script, so we have to call
     * Tk_BackgroundError.  However, it's possible that the file ready
     * condition didn't get cleared for the file, so we could end
     * up in an infinite loop if we're not careful.  To be safe,
     * delete the event handler.
     */

    error:
    DeleteFileEvent(f);
    Tcl_AddErrorInfo(interp,
	    "\n    (script bound to file event - binding deleted)");
    Tk_BackgroundError(interp);
    return TK_FILE_HANDLED;
}

/*
 *----------------------------------------------------------------------
 *
 * DeleteFileEvent --
 *
 *	This procedure is invoked to delete all file event handlers
 *	for a file.  For example, this is necessary if a file is closed,
 *	or if an error occurs in a handler for a file.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The file event handler is removed, so it will never be invoked
 *	again.
 *
 *----------------------------------------------------------------------
 */

static void
DeleteFileEvent(f)
    FILE *f;			/* Stdio structure describing open file. */
{
    register FileEvent *fevPtr;
    FileEvent *prevPtr;

    /*
     * See if there exists a file handler for the given file.
     */

    for (prevPtr = NULL, fevPtr = firstFileEventPtr; ;
	    prevPtr = fevPtr, fevPtr = fevPtr->nextPtr) {
	if (fevPtr == NULL) {
	    return;
	}
	if (fevPtr->f == f) {
	    break;
	}
    }

    /*
     * Unlink it from the list, then free it.
     */

    if (prevPtr == NULL) {
	firstFileEventPtr = fevPtr->nextPtr;
    } else {
	prevPtr->nextPtr = fevPtr->nextPtr;
    }
    Tk_DeleteFileHandler(fileno(fevPtr->f));
    if (fevPtr->scripts[0] != NULL) {
	ckfree(fevPtr->scripts[0]);
    }
    if (fevPtr->scripts[1] != NULL) {
	ckfree(fevPtr->scripts[1]);
    }
    ckfree((char *) fevPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * TkEventCleanupProc --
 *
 *	This procedure is invoked whenever an interpreter is deleted.
 *	It deletes any file events and after commands that refer to
 *	that interpreter.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	File event handlers and after commands are removed.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
void
TkEventCleanupProc(clientData, interp)
    ClientData clientData;	/* Not used. */
    Tcl_Interp *interp;		/* Interpreter that is being deleted. */
{
    FileEvent *fevPtr, *prevPtr, *nextPtr;
    AfterInfo *afterPtr, *prevAfterPtr, *nextAfterPtr;
    int i;

    prevPtr = NULL;
    fevPtr = firstFileEventPtr;
    while (fevPtr != NULL) {
	for (i = 0; i < 2; i++) {
	    if (fevPtr->interps[i] == interp) {
		fevPtr->interps[i] = NULL;
		ckfree((char *) fevPtr->scripts[i]);
		fevPtr->scripts[i] = NULL;
	    }
	}
	if ((fevPtr->scripts[0] != NULL) || (fevPtr->scripts[1] != NULL)) {
	    prevPtr = fevPtr;
	    fevPtr = fevPtr->nextPtr;
	    continue;
	}
	nextPtr = fevPtr->nextPtr;
	if (prevPtr == NULL) {
	    firstFileEventPtr = nextPtr;
	} else {
	    prevPtr->nextPtr = nextPtr;
	}
	Tk_DeleteFileHandler(fileno(fevPtr->f));
	ckfree((char *) fevPtr);
	fevPtr = nextPtr;
    }

    prevAfterPtr = NULL;
    afterPtr = firstAfterPtr;
    while (afterPtr != NULL) {
	if (afterPtr->interp != interp) {
	    prevAfterPtr = afterPtr;
	    afterPtr = afterPtr->nextPtr;
	    continue;
	}
	nextAfterPtr = afterPtr->nextPtr;
	if (prevAfterPtr == NULL) {
	    firstAfterPtr = nextAfterPtr;
	} else {
	    prevAfterPtr->nextPtr = nextAfterPtr;
	}
	if (afterPtr->token != NULL) {
	    Tk_DeleteTimerHandler(afterPtr->token);
	} else {
	    Tk_CancelIdleCall(AfterProc, (ClientData) afterPtr);
	}
	ckfree(afterPtr->command);
	ckfree((char *) afterPtr);
	afterPtr = nextAfterPtr;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * TkwaitCmd2 --
 *
 *	This procedure is invoked to process the "tkwait" Tcl command.
 *	See the user documentation for details on what it does.  This
 *	is a modified version of tkwait with only the "variable"
 *	option, suitable for use in stand-alone mode without the rest
 *	of Tk.  It's only used when Tk_EventInit has been called.
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
static int
TkwaitCmd2(clientData, interp, argc, argv)
    ClientData clientData;	/* Not used. */
    Tcl_Interp *interp;		/* Current interpreter. */
    int argc;			/* Number of arguments. */
    char **argv;		/* Argument strings. */
{
    int c, done;
    size_t length;

    if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " variable name\"", (char *) NULL);
	return TCL_ERROR;
    }
    c = argv[1][0];
    length = strlen(argv[1]);
    if ((c == 'v') && (strncmp(argv[1], "variable", length) == 0)
	    && (length >= 2)) {
	Tcl_TraceVar(interp, argv[2],
		TCL_GLOBAL_ONLY|TCL_TRACE_WRITES|TCL_TRACE_UNSETS,
		WaitVariableProc2, (ClientData) &done);
	done = 0;
	while (!done) {
	    Tk_DoOneEvent(0);
	}
	Tcl_UntraceVar(interp, argv[2],
		TCL_GLOBAL_ONLY|TCL_TRACE_WRITES|TCL_TRACE_UNSETS,
		WaitVariableProc2, (ClientData) &done);
    } else {
	Tcl_AppendResult(interp, "bad option \"", argv[1],
		"\": must be variable", (char *) NULL);
	return TCL_ERROR;
    }

    /*
     * Clear out the interpreter's result, since it may have been set
     * by event handlers.
     */

    Tcl_ResetResult(interp);
    return TCL_OK;
}

	/* ARGSUSED */
static char *
WaitVariableProc2(clientData, interp, name1, name2, flags)
    ClientData clientData;	/* Pointer to integer to set to 1. */
    Tcl_Interp *interp;		/* Interpreter containing variable. */
    char *name1;		/* Name of variable. */
    char *name2;		/* Second part of variable name. */
    int flags;			/* Information about what happened. */
{
    int *donePtr = (int *) clientData;

    *donePtr = 1;
    return (char *) NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * UpdateCmd2 --
 *
 *	This procedure is invoked to process the "update" Tcl command.
 *	See the user documentation for details on what it does.  This
 *	is a modified version of the command that doesn't deal with
 *	windows, suitable for use in stand-alone mode without the rest
 *	of Tk.  It's only used when Tk_EventInit has been called.
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
static int
UpdateCmd2(clientData, interp, argc, argv)
    ClientData clientData;	/* Not used. */
    Tcl_Interp *interp;		/* Current interpreter. */
    int argc;			/* Number of arguments. */
    char **argv;		/* Argument strings. */
{
    int flags;

    if (argc == 1) {
	flags = TK_DONT_WAIT|TK_FILE_EVENTS|TK_TIMER_EVENTS|TK_IDLE_EVENTS;
    } else if (argc == 2) {
	if (strncmp(argv[1], "idletasks", strlen(argv[1])) != 0) {
	    Tcl_AppendResult(interp, "bad argument \"", argv[1],
		    "\": must be idletasks", (char *) NULL);
	    return TCL_ERROR;
	}
	flags = TK_IDLE_EVENTS;
    } else {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " ?idletasks?\"", (char *) NULL);
	return TCL_ERROR;
    }

    /*
     * Handle all pending events.
     */

    while (Tk_DoOneEvent(flags) != 0) {
	/* Empty loop body */
    }

    /*
     * Must clear the interpreter's result because event handlers could
     * have executed commands.
     */

    Tcl_ResetResult(interp);
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_EventInit --
 *
 *	This procedure is invoked from Tcl_AppInit if the Tk event stuff
 *	is being used by itself (without the rest of Tk) in an application.
 *	It creates the "after" and "fileevent" commands.
 *
 * Results:
 *	Always returns TCL_OK.
 *
 * Side effects:
 *	New commands get added to interp.
 *
 *----------------------------------------------------------------------
 */

int
Tk_EventInit(interp)
    Tcl_Interp *interp;		/* Interpreter in which to set up
				 * event-handling. */
{
    Tcl_CreateCommand(interp, "after", Tk_AfterCmd, (ClientData) NULL,
	    (void (*)()) NULL);
    Tcl_CreateCommand(interp, "fileevent", Tk_FileeventCmd, (ClientData) NULL,
	    (void (*)()) NULL);
    Tcl_CreateCommand(interp, "tkwait", TkwaitCmd2, (ClientData) NULL,
	    (void (*)()) NULL);
    Tcl_CreateCommand(interp, "update", UpdateCmd2, (ClientData) NULL,
	    (void (*)()) NULL);
    Tcl_CallWhenDeleted(interp, TkEventCleanupProc, (ClientData) NULL);
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_RestrictEvents --
 *
 *	This procedure is used to globally restrict the set of events
 *	that will be dispatched.  The restriction is done by filtering
 *	all incoming X events through a procedure that determines
 *	whether they are to be processed immediately, deferred, or
 *	discarded.
 *
 * Results:
 *	The return value is the previous restriction procedure in effect,
 *	if there was one, or NULL if there wasn't.
 *
 * Side effects:
 *	From now on, proc will be called to determine whether to process,
 *	defer or discard each incoming X event.
 *
 *----------------------------------------------------------------------
 */

Tk_RestrictProc *
Tk_RestrictEvents(proc, arg, prevArgPtr)
    Tk_RestrictProc *proc;	/* Procedure to call for each incoming
				 * event. */
    ClientData arg;		/* Arbitrary argument to pass to proc. */
    ClientData *prevArgPtr;	/* Place to store information about previous
				 * argument. */
{
    Tk_RestrictProc *prev;

    prev = restrictProc;
    *prevArgPtr = restrictArg;
    restrictProc = proc;
    restrictArg = arg;
    return prev;
}
