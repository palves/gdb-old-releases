/* 
 * gettimeofday.c --
 *
 *	Implements the Unix function gettimeofday.
 *	The gettimeofday is NOT in the POSIX standard.  However,
 *	POSIX does not define the needed functions for time.  The
 *	gettimeofday is the most widely used function used for this
 *	purpose.
 *
 * Copyright (c) 1994-95 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include <OSUtils.h>
#include <Timer.h>
#include <Events.h>
#include <sys/errno.h>
#include <sys/time.h>

int gettimeofday(struct timeval *tp, struct timezone *tz);

/*
 *----------------------------------------------------------------------
 *
 * gettimeofday --
 *
 *	Get the current time.
 *
 * Results:
 *  0 on success, -1 if error.
 *  Possible errors: ENOSYS
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
gettimeofday(tp, tz)
    struct timeval *tp;
    struct timezone *tz;
{
    unsigned long secs;
    UnsignedWide micro;
	
    if (tp == NULL) {
	return 0;
    }
	
    if (ReadDateTime(&secs) != noErr) {
	/*
	 * This should never happen!
	 */
	errno = ENOSYS;
	return -1;
    }

    Microseconds(&micro);
    tp->tv_sec = secs;
    tp->tv_usec = micro.lo % 1000000;
	
    return 0;
}
