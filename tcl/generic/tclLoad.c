/* 
 * tclLoad.c --
 *
 *	This file provides the generic portion (those that are the same
 *	on all platforms) of Tcl's dynamic loading facilities.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tclInt.h"

/*
 * The following structure describes a package that has been loaded
 * either dynamically (with the "load" command) or statically (as
 * indicated by a call to Tcl_PackageLoaded).  All such packages
 * are linked together into a single list for the process.  Packages
 * are never unloaded, so these structures are never freed.
 */

typedef struct LoadedPackage {
    char *fileName;		/* Name of the file from which the
				 * package was loaded.  An empty string
				 * means the package is loaded statically. */
    char *packageName;		/* Name of package prefix for the package,
				 * properly capitalized (first letter UC,
				 * others LC), no "_", as in "Net". */
    Tcl_PackageInitProc *initProc;
				/* Initialization procedure to call to
				 * incorporate this package into a trusted
				 * interpreter. */
    Tcl_PackageInitProc *safeInitProc;
				/* Initialization procedure to call to
				 * incorporate this package into a safe
				 * interpreter (one that will execute
				 * untrusted scripts). */
    struct LoadedPackage *nextPtr;
				/* Next in list of all packages loaded into
				 * this application process.  NULL means
				 * end of list. */
} LoadedPackage;

static LoadedPackage *firstPackagePtr = NULL;
				/* First in list of all packages loaded into
				 * this process. */

/*
 * The following structure represents a particular package that has
 * been incorporated into a particular interpreter (by calling its
 * initialization procedure.  There is a list of these structures for
 * each interpreter, with an AssocData value (key "load") for the
 * interpreter that points to the first package (if any).
 */

typedef struct InterpPackage {
    LoadedPackage *pkgPtr;	/* Points to detailed information about
				 * package. */
    struct InterpPackage *nextPtr;
				/* Next package in this interpreter, or
				 * NULL for end of list. */
} InterpPackage;

/*
 * Prototypes for procedures that are private to this file:
 */

static void		LoadCleanupProc _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp));

/*
 *----------------------------------------------------------------------
 *
 * Tcl_LoadCmd --
 *
 *	This procedure is invoked to process the "load" Tcl command.
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

int
Tcl_LoadCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    Tcl_Interp *target;
    LoadedPackage *pkgPtr;
    Tcl_DString pkgName, initName, safeInitName, fileName;
    Tcl_PackageInitProc *initProc, *safeInitProc;
    InterpPackage *ipFirstPtr, *ipPtr;
    int code, c;
    char *p, *fullFileName;

    if ((argc != 3) && (argc != 4)) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" fileName packageName ?interp?\"", (char *) NULL);
	return TCL_ERROR;
    }
    fullFileName = Tcl_TildeSubst(interp, argv[1], &fileName);
    if (fullFileName == NULL) {
	return TCL_ERROR;
    }

    /*
     * Figure out which interpreter we're going to load the package into.
     */

    target = interp;
    if (argc == 4) {
	target = Tcl_GetSlave(interp, argv[3]);
	if (target == NULL) {
	    Tcl_AppendResult(interp, "couldn't find slave interpreter named \"",
		    argv[3], "\"", (char *) NULL);
	    return TCL_ERROR;
	}
    }

    /*
     * Fix the capitalization in the package name so that the first
     * character is in caps but the others are all lower-case.
     */

    Tcl_DStringInit(&pkgName);
    Tcl_DStringInit(&initName);
    Tcl_DStringInit(&safeInitName);
    Tcl_DStringAppend(&pkgName, argv[2], -1);
    p = Tcl_DStringValue(&pkgName);
    c = UCHAR(*p);
    if (c != 0) {
	if (islower(c)) {
	    *p = toupper(c);
	}
	p++;
	while (1) {
	    c = UCHAR(*p);
	    if (c == 0) {
		break;
	    }
	    if (isupper(c)) {
		*p = tolower(c);
	    }
	    p++;
	}
    }

    /*
     * Make sure that the package isn't already loaded in the current
     * interpreter.  If so, it's an error unless it was loaded from
     * the same file asked for here (in that case, there is nothing
     * for us to do).
     */

    ipFirstPtr = (InterpPackage *) Tcl_GetAssocData(target, "tclLoad",
	    (Tcl_InterpDeleteProc **) NULL);
    for (ipPtr = ipFirstPtr; ipPtr != NULL; ipPtr = ipPtr->nextPtr) {
	if (strcmp(ipPtr->pkgPtr->packageName, Tcl_DStringValue(&pkgName))
		== 0) {
	    if (strcmp(ipPtr->pkgPtr->fileName, fullFileName) == 0) {
		code = TCL_OK;
	    } else {
		Tcl_AppendResult(interp, "package \"",
			Tcl_DStringValue(&pkgName),
			"\" is already loaded in the interpreter",
			(char *) NULL);
		code = TCL_ERROR;
	    }
	    goto done;
	}
    }

    /*
     * See if the desired file is already loaded.  If so, its package
     * name must agree with ours.
     */

    for (pkgPtr = firstPackagePtr; pkgPtr != NULL; pkgPtr = pkgPtr->nextPtr) {
	if (strcmp(pkgPtr->fileName, fullFileName) == 0) {
	    if (strcmp(pkgPtr->packageName, Tcl_DStringValue(&pkgName)) != 0) {
		Tcl_AppendResult(interp, "file \"", fullFileName,
			"\" is already loaded for package \"",
			pkgPtr->packageName, "\"", (char *) NULL);
		code = TCL_ERROR;
		goto done;
	    }
	    break;
	}
    }

    if (pkgPtr == NULL) {
	/*
	 * The desired file isn't currently loaded, so load it.  First,
	 * compute the names of the two initialization procedures, based
	 * on the package name.
	 */
    
	Tcl_DStringAppend(&initName, Tcl_DStringValue(&pkgName), -1);
	Tcl_DStringAppend(&initName, "_Init", 5);
	Tcl_DStringAppend(&safeInitName, Tcl_DStringValue(&pkgName), -1);
	Tcl_DStringAppend(&safeInitName, "_SafeInit", 9);
    
	/*
	 * Call platform-specific code to load the package and find the
	 * two initialization procedures.
	 */
    
	code = TclLoadFile(interp, fullFileName, Tcl_DStringValue(&initName),
		Tcl_DStringValue(&safeInitName), &initProc, &safeInitProc);
	if (code != TCL_OK) {
	    goto done;
	}

	/*
	 * Create a new record to describe this package.
	 */

	pkgPtr = (LoadedPackage *) ckalloc(sizeof(LoadedPackage));
	pkgPtr->fileName = (char *) ckalloc((unsigned)
		(strlen(fullFileName) + 1));
	strcpy(pkgPtr->fileName, fullFileName);
	pkgPtr->packageName = (char *) ckalloc((unsigned)
		(Tcl_DStringLength(&pkgName) + 1));
	strcpy(pkgPtr->packageName, Tcl_DStringValue(&pkgName));
	pkgPtr->initProc = initProc;
	pkgPtr->safeInitProc = safeInitProc;
	pkgPtr->nextPtr = firstPackagePtr;
	firstPackagePtr = pkgPtr;
    }

    /*
     * Invoke the package's initialization procedure (either the
     * normal one or the safe one, depending on whether or not the
     * interpreter is safe).
     */

    if (Tcl_IsSafe(target)) {
	if (pkgPtr->safeInitProc != NULL) {
	    code = (*pkgPtr->safeInitProc)(target);
	} else {
	    Tcl_AppendResult(interp, "package \"", pkgPtr->packageName,
		    "\" can't be used in a safe interpreter: ",
		    "no ", pkgPtr->packageName, "_SafeInit procedure",
		    (char *) NULL);
	    code = TCL_ERROR;
	}
    } else {
	if (pkgPtr->initProc != NULL) {
	    code = (*pkgPtr->initProc)(target);
	} else {
	    Tcl_AppendResult(interp, "package \"", pkgPtr->packageName,
		    "\" has no ", pkgPtr->packageName, "_Init procedure",
		    (char *) NULL);
	    code = TCL_ERROR;
	}
    }

    /*
     * Record the fact that the package has been loaded in the
     * target interpreter.
     */

    if (code == TCL_OK) {
	ipPtr = (InterpPackage *) ckalloc(sizeof(InterpPackage));
	ipPtr->pkgPtr = pkgPtr;
	ipPtr->nextPtr = ipFirstPtr;
	Tcl_SetAssocData(target, "tclLoad", LoadCleanupProc,
		(ClientData) ipPtr);
    }

    done:
    Tcl_DStringFree(&pkgName);
    Tcl_DStringFree(&initName);
    Tcl_DStringFree(&safeInitName);
    return code;
}

/*
 *----------------------------------------------------------------------
 *
 * TclGetLoadedPackages --
 *
 *	This procedure returns information about all of the files
 *	that are loaded (either in a particular intepreter, or
 *	for all interpreters).
 *
 * Results:
 *	The return value is a standard Tcl completion code.  If
 *	successful, a list of lists is placed in interp->result.
 *	Each sublist corresponds to one loaded file;  its first
 *	element is the name of the file (or an empty string for
 *	something that's statically loaded) and the second element
 *	is the name of the package in that file.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
TclGetLoadedPackages(interp, targetName)
    Tcl_Interp *interp;		/* Interpreter in which to return
				 * information or error message. */
    char *targetName;		/* Name of target interpreter or NULL.
				 * If NULL, return info about all interps;
				 * otherwise, just return info about this
				 * interpreter. */
{
    Tcl_Interp *target;
    LoadedPackage *pkgPtr;
    InterpPackage *ipPtr;
    char *prefix;

    if (targetName == NULL) {
	/* 
	 * Return information about all of the available packages.
	 */

	prefix = "{";
	for (pkgPtr = firstPackagePtr; pkgPtr != NULL;
		pkgPtr = pkgPtr->nextPtr) {
	    Tcl_AppendResult(interp, prefix, (char *) NULL);
	    Tcl_AppendElement(interp, pkgPtr->fileName);
	    Tcl_AppendElement(interp, pkgPtr->packageName);
	    Tcl_AppendResult(interp, "}", (char *) NULL);
	    prefix = " {";
	}
	return TCL_OK;
    }

    /*
     * Return information about only the packages that are loaded in
     * a given interpreter.
     */

    target = Tcl_GetSlave(interp, targetName);
    if (target == NULL) {
	Tcl_AppendResult(interp, "couldn't find slave interpreter named \"",
		targetName, "\"", (char *) NULL);
	return TCL_ERROR;
    }
    ipPtr = (InterpPackage *) Tcl_GetAssocData(target, "tclLoad",
	    (Tcl_InterpDeleteProc **) NULL);
    prefix = "{";
    for ( ; ipPtr != NULL; ipPtr = ipPtr->nextPtr) {
	pkgPtr = ipPtr->pkgPtr;
	Tcl_AppendResult(interp, prefix, (char *) NULL);
	Tcl_AppendElement(interp, pkgPtr->fileName);
	Tcl_AppendElement(interp, pkgPtr->packageName);
	Tcl_AppendResult(interp, "}", (char *) NULL);
	prefix = " {";
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * LoadCleanupProc --
 *
 *	This procedure is called to delete all of the InterpPackage
 *	structures for an interpreter when the interpreter is deleted.
 *	It gets invoked via the Tcl AssocData mechanism.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Storage for all of the InterpPackage procedures for interp
 *	get deleted.
 *
 *----------------------------------------------------------------------
 */

static void
LoadCleanupProc(clientData, interp)
    ClientData clientData;	/* Pointer to first InterpPackage structure
				 * for interp. */
    Tcl_Interp *interp;		/* Interpreter that is being deleted. */
{
    InterpPackage *ipPtr, *nextPtr;

    ipPtr = (InterpPackage *) clientData;
    while (ipPtr != NULL) {
	nextPtr = ipPtr->nextPtr;
	ckfree((char *) ipPtr);
	ipPtr = nextPtr;
    }
}
