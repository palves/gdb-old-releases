/* 
 * pkg1.c --
 *
 *	This file contains a simple Tcl package "pkg1" that is intended
 *	for testing the Tcl dynamic loading facilities.
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

static int	Pkg1_EqCmd _ANSI_ARGS_((ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv));
static int	Pkg1_QuoteCmd _ANSI_ARGS_((ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv));

/*
 *----------------------------------------------------------------------
 *
 * Pkg1_EqCmd --
 *
 *	This procedure is invoked to process the "pkg1_eq" Tcl command.
 *	It expects two arguments and returns 1 if they are the same,
 *	0 if they are different.
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
Pkg1_EqCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" string1 string2\"", (char *) NULL);
	return TCL_ERROR;
    }

    if (strcmp(argv[1], argv[2]) == 0) {
	interp->result = "1";
    } else {
	interp->result = "0";
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Pkg1_quoteCmd --
 *
 *	This procedure is invoked to process the "pkg1_quote" Tcl command.
 *	It expects one argument, which it returns as result.
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
Pkg1_QuoteCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" value\"", (char *) NULL);
	return TCL_ERROR;
    }
    strcpy(interp->result, argv[1]);
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Pkg1_Init --
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
Pkg1_Init(interp)
    Tcl_Interp *interp;		/* Interpreter in which the package is
				 * to be made available. */
{
    Tcl_CreateCommand(interp, "pkg1_eq", Pkg1_EqCmd, (ClientData) 0,
	    (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "pkg1_quote", Pkg1_QuoteCmd, (ClientData) 0,
	    (Tcl_CmdDeleteProc *) NULL);
    return TCL_OK;
}
