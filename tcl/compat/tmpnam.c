/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific written prior permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <stdio.h>

/*
 * Use /tmp instead of /usr/tmp, because L_tmpname is only 14 chars
 * on some machines (like NeXT machines) and /usr/tmp will cause
 * buffer overflows.
 */

#undef P_tmpdir
#define	P_tmpdir	"/tmp"

char *
tmpnam(s)
	char *s;
{
	static char name[MAXPATHLEN+2];
	char *mktemp();

#ifndef LYNX			/* The only OS I know like this. */

	if (!s)
		s = name;
	(void)sprintf(s, "%s/XXXXXX", P_tmpdir);
	return(mktemp(s));

#else

    name = mktemp (P_tmpdir "c@c%");
    if (!s) return name;

    strcpy (s, name);
    return s;
#endif
}
