/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 *
 * @(#) tclRegexp.h 1.2 94/08/09 11:45:53
 */

#ifndef _TCL
#include "tcl.h"
#endif
#ifndef _REGEXP
#define _REGEXP 1

#define NSUBEXP  50
typedef struct regexp {
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	char *regmust;		/* Internal use only. */
	int regmlen;		/* Internal use only. */
	char program[1];	/* Unwarranted chumminess with compiler. */
} regexp;

extern regexp *TclRegComp _ANSI_ARGS_((char *exp));
extern int TclRegExec _ANSI_ARGS_((regexp *prog, char *string, char *start));
extern void TclRegSub _ANSI_ARGS_((regexp *prog, char *source, char *dest));
extern void TclRegError _ANSI_ARGS_((char *msg));

#endif /* REGEXP */
