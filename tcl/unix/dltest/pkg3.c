/* 
 * pkg3.c --
 *
 *	This file contains a simple Tcl package "pkg3" that is intended
 *	for testing the Tcl dynamic loading facilities.  It can be used
 *	in both safe and unsafe interpreters.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#include "tcl.h"

/*
 * Prototypes for procedures defined later in this file:
 */

static int	Pkg3_SubCmd _ANSI_ARGS_((ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv));
static int	Pkg3_UnsafeCmd _ANSI_ARGS_((ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv));

/*
 *----------------------------------------------------------------------
 *
 * Pkg3_SubCmd --
 *
 *	This procedure is invoked to process the "pkg3_sub" Tcl command.
 *	It expects two arguments and returns their difference.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

static int
Pkg3_SubCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int first, second;

    if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" num num\"", (char *) NULL);
	return TCL_ERROR;
    }
    if ((Tcl_GetInt(interp, argv[1], &first) != TCL_OK)
	    || (Tcl_GetInt(interp, argv[2], &second) != TCL_OK)) {
	return TCL_ERROR;
    }
    sprintf(interp->result, "%d", first - second);
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Pkg3_UnsafeCmd --
 *
 *	This procedure is invoked to process the "pkg3_unsafe" Tcl command.
 *	It just returns a constant string.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

static int
Pkg3_UnsafeCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    interp->result = "unsafe command invoked";
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Pkg3_Init --
 *
 *	This is a package initialization procedure, which is called
 *	by Tcl when this package is to be added to an interpreter.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
Pkg3_Init(interp)
    Tcl_Interp *interp;		/* Interpreter in which the package is
				 * to be made available. */
{
    Tcl_CreateCommand(interp, "pkg3_sub", Pkg3_SubCmd, (ClientData) 0,
	    (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "pkg3_unsafe", Pkg3_UnsafeCmd, (ClientData) 0,
	    (Tcl_CmdDeleteProc *) NULL);
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Pkg3_SafeInit --
 *
 *	This is a package initialization procedure, which is called
 *	by Tcl when this package is to be added to an unsafe interpreter.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
Pkg3_SafeInit(interp)
    Tcl_Interp *interp;		/* Interpreter in which the package is
				 * to be made available. */
{
    Tcl_CreateCommand(interp, "pkg3_sub", Pkg3_SubCmd, (ClientData) 0,
	    (Tcl_CmdDeleteProc *) NULL);
    return TCL_OK;
}