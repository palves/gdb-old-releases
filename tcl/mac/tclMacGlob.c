/* 
 * tclMacGlob.c --
 *
 *	This file provides procedures and commands for file name
 *	manipulation, such as tilde expansion and globbing.  In
 *	future releases the Mac specific parts of this code will
 *	removed or merged into tclGlob.c.
 *
 * Copyright 1993-1994 Lockheed Missle & Space Company, AI Center
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */



#include "tclInt.h"
#include "tclPort.h"

#include <Errors.h>
#include <Strings.h>

/*
 * The structure below is used to keep track of a globbing result
 * being built up (i.e. a partial list of file names).  The list
 * grows dynamically to be as big as needed.
 */

typedef struct {
    char *result;		/* Pointer to result area. */
    int totalSpace;		/* Total number of characters allocated
				 * for result. */
    int spaceUsed;		/* Number of characters currently in use
				 * to hold the partial result (not including
				 * the terminating NULL). */
    int dynamic;		/* 0 means result is static space, 1 means
				 * it's dynamic. */
} GlobResult;

static int		gShowIniv = 0;
static int		gShowType = 0;
static int		gShowCreator = 0;
static ResType	gType;
static ResType	gCreator;

/*
 * Declarations for procedures local to this file:
 */

static int		DoGlob _ANSI_ARGS_((Tcl_Interp *interp, char *dir,
			    char *rem));

/*
 *----------------------------------------------------------------------
 *
 * DoGlob --
 *
 *	This recursive procedure forms the heart of the globbing
 *	code.  It performs a depth-first traversal of the tree
 *	given by the path name to be globbed.
 *
 * Results:
 *	The return value is a standard Tcl result indicating whether
 *	an error occurred in globbing.  After a normal return the
 *	result in interp will be set to hold all of the file names
 *	given by the dir and rem arguments.  After an error the
 *	result in interp will hold an error message.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
DoGlob(interp, dir, rem)
    Tcl_Interp	*interp;	/* Interpreter to use for error
				 * reporting (e.g. unmatched brace). */
    char *dir;		/* Name of a directory at which to
			 * start glob expansion.  This name
			 * is fixed: it doesn't contain any
			 * globbing chars. */
    char *rem;		/* Path to glob-expand. */
{
    /*
     * When this procedure is entered, the name to be globbed may
     * already have been partly expanded by ancestor invocations of
     * DoGlob.  The part that's already been expanded is in "dir"
     * (this may initially be empty), and the part still to expand
     * is in "rem".  This procedure expands "rem" one level, making
     * recursive calls to itself if there's still more stuff left
     * in the remainder.
     */

    Tcl_DString newName;	/* Holds new name consisting of
				 * dir plus the first part of rem. */
    register char *p;
    register char c;
    char *openBrace, *closeBrace, *name;
    int gotSpecial, baseLength;
    int result = TCL_OK, match, scan_volumes = 0;
    struct stat statBuf;
    char mac_name[256], *dirName;

    /*
     * Make sure that the directory part of the name really is a
     * directory.  If the directory name is "", use the name "."
     * instead, because some UNIX systems don't treat "" like "."
     * automatically. Keep the "" for use in generating file names,
     * otherwise "glob foo.c" would return "./foo.c".
     */

    if (*dir == '\0') {
	if ( strchr(rem, ':') != NULL )
	    scan_volumes = 1;
	dirName = ":";
    } else if ( *dir != ':' && strchr(dir, ':') == NULL ) {
	/*
	 * Special case where dir is name of a volume, 
	 * will add the optional colon to make things easier
	 */
	strcpy(mac_name, dir);
	strcat(mac_name, ":");
	dirName = mac_name;
    } else {
	dirName = dir;
    }
	
    if ((stat(dirName, &statBuf) != 0) || !S_ISDIR(statBuf.st_mode)) {
	return TCL_OK;
    }
    Tcl_DStringInit(&newName);

    /*
     * First, find the end of the next element in rem, checking
     * along the way for special globbing characters.
     */

    gotSpecial = 0;
    openBrace = closeBrace = NULL;
    for ( p = rem ; ; p++ ) {
	c = *p;
	if ((c == '\0') || ((openBrace == NULL) && (c == ':'))) {
	    break;
	}
	if ((c == '{') && (openBrace == NULL)) {
	    openBrace = p;
	}
	if ((c == '}') && (openBrace != NULL) && (closeBrace == NULL)) {
	    closeBrace = p;
	}
	if ((c == '*') || (c == '[') || (c == '\\') || (c == '?')) {
	    gotSpecial = 1;
	}
    }

    /*
     * If there is an open brace in the argument, then make a recursive
     * call for each element between the braces.  In this case, the
     * recursive call to DoGlob uses the same "dir" that we got.
     * If there are several brace-pairs in a single name, we just handle
     * one here, and the others will be handled in recursive calls.
     */

    if (openBrace != NULL) {
	char *element;

	if (closeBrace == NULL) {
	    Tcl_ResetResult(interp);
	    interp->result = "unmatched open-brace in file name";
	    result = TCL_ERROR;
	    goto done;
	}
			
	Tcl_DStringAppend(&newName, rem, openBrace-rem);
	baseLength = newName.length;
	p = openBrace;
		
	for ( p = openBrace ; *p != '}' ; ) {
	    element = p+1;
	    for ( p = element ; ((*p != '}') && (*p != ',')) ; p++ ) {
				/* Empty loop body. */
	    }
	    Tcl_DStringAppend(&newName, element, p-element);
	    Tcl_DStringAppend(&newName, closeBrace+1, -1);
	    result = DoGlob(interp, dir, newName.string);
	    if (result != TCL_OK) {
		goto done;
	    }
	    newName.length = baseLength;
	}
	goto done;
    }

    /*
     * Start building up the next-level name with dir plus a slash if
     * needed to separate it from the next file name.
     */

    Tcl_DStringAppend(&newName, dir, -1);
    if ((dir[0] != 0) && (newName.string[newName.length-1] != ':')) {
	Tcl_DStringAppend(&newName, ":", 1);
    }
    baseLength = newName.length;

    /*
     * If there were any pattern-matching characters, then scan through
     * the directory to find all the matching names.
     */

    if (gotSpecial) {
	CInfoPBRec cpb;
	HParamBlockRec	vpb;
	char *dir_ptr = dir;	/* Use for > 256 chars */
	int index;
	short vrefnum;
	long dirid;
	char savedChar;

	result = TCL_OK;
	savedChar = *p;
	*p = 0;

	if ( scan_volumes ) {
	    vrefnum = statBuf.st_dev;
	    dirid = statBuf.st_ino;
	} else if ( S_ISDIR(statBuf.st_mode) ) {
	    vrefnum = statBuf.st_dev;
	    dirid = statBuf.st_ino;
	} else {
	    /* RELATIVE */
	    WDPBRec wpb;
	    Str32 volname;
			
	    wpb.ioCompletion = 0;
	    wpb.ioNamePtr = volname;
	    PBHGetVol(&wpb, false);
			
	    vrefnum = wpb.ioWDVRefNum;
	    dirid = wpb.ioWDDirID;
	}
		 
	for ( index=1 ; ; index++ ) {
	    if ( scan_volumes ) {
		mac_name[0] = '\0';
		vpb.volumeParam.ioCompletion = 0;
		vpb.volumeParam.ioVolIndex = index;
		vpb.volumeParam.ioNamePtr = (unsigned char *)mac_name;
		vpb.volumeParam.ioVRefNum = 0;
		result = PBHGetVInfo( &vpb, false );
		if (result == nsvErr) {
		    result = TCL_OK;
		    break;
		}
					
		p2cstr( (StringPtr) mac_name);
	    } else {
		mac_name[0] = '\0';
		cpb.hFileInfo.ioCompletion = 0;
		cpb.hFileInfo.ioVRefNum = vrefnum;
		cpb.hFileInfo.ioNamePtr = (unsigned char *)mac_name;
		cpb.hFileInfo.ioFDirIndex = index;
		cpb.hFileInfo.ioDirID = dirid;
		result = PBGetCatInfo(&cpb, false);
		if (result == fnfErr) {
		    result = TCL_OK;
		    break;
		}
	
		if (!gShowIniv)
		    if ((cpb.hFileInfo.ioFlFndrInfo.fdFlags & fInvisible) != 0) {
			continue;
		    }
				
		if (gShowType)
		    if (cpb.hFileInfo.ioFlFndrInfo.fdType != gType) {
			continue;
		    }
				
		if (gShowCreator)
		    if (cpb.hFileInfo.ioFlFndrInfo.fdCreator != gCreator) {
			continue;
		    }
				
		p2cstr( (StringPtr) mac_name);
				
		/*
		 * Don't match names starting with "." unless the "." is
		 * present in the pattern.
		 */
		if ((*mac_name == '.') && (*rem != '.')) {
		    continue;
		}
	    } /* ! scan_volumes */
			
	    match = Tcl_StringMatch( (char *)mac_name, rem );

	    if ( match ) {
		newName.length = baseLength;
		Tcl_DStringAppend(&newName, mac_name, -1);
				
		if (savedChar == '\0') {
		    Tcl_AppendElement(interp, newName.string);
		} else if ( scan_volumes ||
			(cpb.hFileInfo.ioFlAttrib & ioDirMask) != 0 ) {
		    result = DoGlob(interp, newName.string, p + 1);
					
		    if (result != TCL_OK)
			break;
		}
	    } else {
	    }
	}
		
	*p = savedChar;
	goto done;
    }

    /*
     * The current element is a simple one with no fancy features.  Add
     * it to the new name.  If there are more elements still to come,
     * then recurse to process them.
     */
    if (*p == ':')
	++p;
		
    Tcl_DStringAppend(&newName, rem, p-rem);
    if ( *p != 0 ) {
	result = DoGlob( interp, newName.string, p );
	goto done;
    }

    /*
     * There are no more elements in the pattern.  Check to be sure the
     * file actually exists, then add its name to the list being formed
     * in interp-result.
     */

    name = newName.string;
    if (*name == 0) {
	name = ":";
    }
	
    if (access(name, F_OK) != 0) {
	goto done;
    }
	
    Tcl_AppendElement(interp, name);

    done:
    Tcl_DStringFree(&newName);
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_TildeSubst --
 *
 *	Given a name starting with a tilde, produce a name where
 *	the tilde and following characters have been replaced by
 *	the home directory location for the named user.
 *
 * Results:
 *	The result is a pointer to a static string containing
 *	the new name.  If there was an error in processing the
 *	tilde, then an error message is left in interp->result
 *	and the return value is NULL.  The result may be stored
 *	in bufferPtr; the caller must call Tcl_DStringFree(bufferPtr)
 *	to free the name.
 *
 * Side effects:
 *	Information may be left in bufferPtr.
 *
 *----------------------------------------------------------------------
 */

char *
Tcl_TildeSubst(interp, name, bufferPtr)
    Tcl_Interp *interp;		/* Interpreter in which to store error
				 * message (if necessary). */
    char *name;			/* File name, which may begin with "~/"
				 * (to indicate current user's home directory)
				 * or "~<user>/" (to indicate any user's
				 * home directory). */
    Tcl_DString *bufferPtr;	/* May be used to hold result.  Must not hold
				 * anything at the time of the call, and need
				 * not even be initialized. */
{
    char *ptr;

    Tcl_DStringInit(bufferPtr);
    if (name[0] == '~') {
	Tcl_ResetResult(interp);
	Tcl_AppendResult(interp, "tilde substitute not available", 
		(char *) NULL);
	return NULL;
    }

    /*
     * Convert Unix paths (using /'s) to Macintosh paths (using :'s)
     * The first part handles differences in absolute & relative
     * paths.  The second part just changes all /'s to :'s.
     */

    if (name[0] == '/') {
	/* Absolute Unix path */
	Tcl_DStringAppend(bufferPtr, name + 1, -1);
    } else if (name[0] == ':' || strchr(name, ':') != NULL) {
	/* Partial Mac path or assumed Absolute Mac path */
	Tcl_DStringAppend(bufferPtr, name, -1);
    } else if (strchr(name, '/') != NULL) {
	/* Assumed to be a partial unix path */
	Tcl_DStringAppend(bufferPtr, ":", -1);
	Tcl_DStringAppend(bufferPtr, name, -1);
    } else {
	/* Probably a file */
	Tcl_DStringAppend(bufferPtr, name, -1);
    }

    ptr = bufferPtr->string;
    while (*ptr != NULL) {
	if (*ptr == '/') {
	    *ptr = ':';
	}
	ptr++;
    }

    return bufferPtr->string;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_GlobCmd --
 *
 *	This procedure is invoked to process the "glob" Tcl command.
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
Tcl_GlobCmd(client, interp, argc, argv)
    ClientData	client;		/* Not used. */
    Tcl_Interp	*interp;	/* Current interpreter. */
    int			argc;		/* Number of arguments. */
    char		**argv;		/* Argument strings. */
{
    int i, result, noComplain, firstArg;

	gShowType = 0;
	gShowCreator = 0;
	gShowIniv = 0;

    if (argc < 2) {
		notEnoughArgs:
		Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
							" ?switches? name ?name ...?\"", (char *) NULL);
		return TCL_ERROR;
	}
	noComplain = 0;
	for (firstArg = 1; (firstArg < argc) && (argv[firstArg][0] == '-'); firstArg++) {
		if (strcmp(argv[firstArg], "-nocomplain") == 0) {
			noComplain = 1;
		} else if (strcmp(argv[firstArg], "--") == 0) {
			firstArg++;
			break;
		} else if (strcmp(argv[firstArg], "-i") == 0) {
			gShowIniv = 1;
		} else if (strcmp(argv[firstArg], "-c") == 0) {
			gShowCreator = 1;
			sprintf((char *)&gCreator, "%-4.4s", argv[++firstArg]);
		} else if (strcmp(argv[firstArg], "-t") == 0) {
			gShowType = 1;
			sprintf((char *)&gType, "%-4.4s", argv[++firstArg]);
		} else {
			Tcl_AppendResult(interp, "bad option \"", argv[firstArg],
						"\": must be -nocomplain, -i, -c, -t or --", (char *) NULL);
			return TCL_ERROR;
		}
	}
	if (firstArg >= argc) {
		goto notEnoughArgs;
	}
	
	for (i = firstArg; i < argc; i++) {
		char *thisName;
		Tcl_DString buffer;
		
		thisName = Tcl_TildeSubst(interp, argv[i], &buffer);
		if (thisName == NULL) {
			return TCL_ERROR;
		}
		
		result = DoGlob(interp, "", thisName);
		
		Tcl_DStringFree(&buffer);
		if (result != TCL_OK) {
			return result;
		}
	}
	
	if ((*interp->result == 0) && !noComplain) {
		char *sep = "";
		
		Tcl_AppendResult(interp, "no files matched glob pattern",
			(argc == 2) ? " \"" : "s \"", (char *) NULL);
		for (i = firstArg; i < argc; i++) {
			Tcl_AppendResult(interp, sep, argv[i], (char *) NULL);
			sep = " ";
		}
		Tcl_AppendResult(interp, "\"", (char *) NULL);
		return TCL_ERROR;
	}
	
	return TCL_OK;
}
