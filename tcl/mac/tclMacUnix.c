/* 
 * tclMacUnix.c --
 *
 *	This file contains routines to implement several features
 *	available to the Unix implementation, but that require
 *      extra work to do on a Macintosh.  These include routines
 *      Unix Tcl normally hands off to the Unix OS.
 *
 * Copyright 1993-1994 Lockheed Missle & Space Company, AI Center
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include <Files.h>
#include <Strings.h>
#include <TextUtils.h>
#include <Finder.h>
#include <Aliases.h>
#include <Folders.h>
#include <Packages.h>
#include <Resources.h>
#include <Errors.h>
#include <Sound.h>
#include <Script.h>
#include <Timer.h>
#ifdef THINK_C
#	include <pascal.h>
#endif

#include "tcl.h"
#include "tclInt.h"
#include "tclPort.h"

#include <sys/stat.h>
#include <time.h>

/*
 * The following two Includes are from the More Files package
 */
#include "FileCopy.h"
#include "FullPath.h"
#include "MoreFiles.h"
#include "MoreFilesExtras.h"

/*
 * The following may not be defined in some versions of
 * MPW header files.
 */
#ifndef kIsInvisible
#define kIsInvisible 0x4000
#endif
#ifndef kIsAlias
#define kIsAlias 0x8000
#endif

/*
 * Missing error codes
 */
#define usageErr		500
#define noSourceErr		501
#define isDirErr		502


/*
 *----------------------------------------------------------------------
 *
 * Tcl_EchoCmd --
 *
 *    Implements the TCL echo command:
 *        echo ?str ...?
 *
 * Results:
 *      Always returns TCL_OK.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_EchoCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int idx;

    for (idx = 1; idx < argc; idx++) {
        fputs(argv [idx], stdout);
        if (idx < (argc - 1)) {
            printf(" ");
	}
    }
    printf("\n");
    return TCL_OK;
} /* Tcl_EchoCmd */

/*
 *----------------------------------------------------------------------
 *
 * Tcl_TimeCmd --
 *
 *	This procedure is invoked to process the "time" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 * Note:
 *	This funtion is a modified version from the core Tcl release version.
 *	This version is also based on changes origionally done for the Mac by
 *	Tim Endres.
 *----------------------------------------------------------------------
 */

int
Tcl_TimeCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int count, i, result;
    double timePer;
    UnsignedWide start, stop;
    UInt32 diff;

    if (argc == 2) {
	count = 1;
    } else if (argc == 3) {
	if (Tcl_GetInt(interp, argv[2], &count) != TCL_OK) {
	    return TCL_ERROR;
	}
    } else {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" command ?count?\"", (char *) NULL);
	return TCL_ERROR;
    }

    Microseconds(&start);

    for (i = count ; i > 0; i--) {
	result = Tcl_Eval(interp, argv[1]);
	if (result != TCL_OK) {
	    if (result == TCL_ERROR) {
		char msg[60];
		sprintf(msg, "\n    (\"time\" body line %d)",
			interp->errorLine);
		Tcl_AddErrorInfo(interp, msg);
	    }
	    return result;
	}
    }

    Microseconds(&stop);
    if (stop.hi == start.hi) {
	diff = stop.lo - start.lo;
    } else {
    	/* TODO: figure out how to do the math */
	diff = 0;
    }
    timePer = (double) diff / (double) count;

    Tcl_ResetResult(interp);
    sprintf(interp->result, "%.0lf microseconds per iteration", timePer);
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_LsCmd --
 *
 *	This procedure is invoked to process the "ls" Tcl command.
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
Tcl_LsCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
#define STRING_LENGTH 80
#define CR '\n'
    int i, j;
    int fieldLength, len = 0, maxLen = 0, perLine;
    char **origArgv = argv;
    OSErr err;
    CInfoPBRec paramBlock;
    HFileInfo *hpb = (HFileInfo *)&paramBlock;
    DirInfo *dpb = (DirInfo *)&paramBlock;
    char theFile[256];
    char theLine[STRING_LENGTH + 2];
    int aIndex;
    int fFlag = false, pFlag = false, aFlag = false, lFlag = false,
	cFlag = false, hFlag = false;

    /*
     * Process command flags.  End if argument doesn't start
     * with a dash or is a dash ny itself.  The remaining arguments
     * should be files.
     */
    for (i = 1; i < argc; i++) {
	if (argv[i][0] != '-') {
	    break;
	}
		
	if (!strcmp(argv[i], "-")) {
	    i++;
	    break;
	}
		
	for (j = 1 ; argv[i][j] ; ++j) {
	    switch(argv[i][j]) {
	    case 'a':
	    case 'A':
		aFlag = true;
		aIndex = i;
		break;
	    case '1':
		cFlag = false;
		break;
	    case 'C':
		cFlag = true;
		break;
	    case 'F':
		fFlag = true;
		break;
	    case 'H':
		hFlag = true;
		break;
	    case 'p':
		pFlag = true;
		break;
	    case 'l':
		pFlag = false;
		lFlag = true;
		break;
	    default:
		Tcl_AppendResult(interp, "error - unknown flag ",
			"usage: ls -apCFHl1 ?files? ", NULL);
		return TCL_ERROR;
	    }
	}
    }

    argv += i;
    argc -= i;

    /*
     * No file specifications means we search for all files.
     * Glob will be doing most of the work.  Set up for the
     * call to glob here.
     */
     if (!argc) {
	argc = 1;
	argv = origArgv;
	if (aFlag) {
	    strcpy(argv[0],"-i");
	    argc = 2;
	    strcpy(argv[1],"*");
	} else {
	    strcpy(argv[0], "*");
	}
    } else {
	if (aFlag) {
	    /*
	     * If -a, set argv[0] to the origArgv that
	     * contains "-a", and replace 'a' with 'i'
	     * so that glob lists invisible files.
	     */
	    argc++;
	    argv--;
	    argv[0] = origArgv[aIndex];
	    argv[0][1] = 'i';
	}
    }

    /*
     * This next delightfull piece of code works around the fact that
     * the GUSI 'chdir' command doesn't actually change the working
     * directory for the Macintosh.  Matthias has said this will be
     * fixed in a future version - but it has yet to be fixed.  If you
     * see this comment go ahead and send him a reminder!
     */

    {
	char buffer[MAXPATHLEN+1];
	FSSpec spec;
	    
	if (getcwd(buffer, MAXPATHLEN+1) == NULL) {
	    if (errno == ERANGE) {
		interp->result = "working directory name is too long";
	    } else {
		Tcl_AppendResult(interp,
			"error getting working directory name: ",
			Tcl_PosixError(interp), (char *) NULL);
	    }
	    return TCL_ERROR;
	}
	FSpLocationFromFullPath(strlen(buffer), buffer, &spec);
	FSpSetDefaultDir(&spec);
    }

    /*
     * We let glob do most of the hard work.  If it finds nothing
     * we return nothing.
     */
    if (!globArgs(interp, &argc, &argv)) {
	Tcl_ResetResult(interp);
	return TCL_OK;
    }

    /*
     * There are two major methods for listing files: the long
     * method and the normal method.
     */
    if (lFlag) {
	char	creator[5], type[5], time[16], date[16];
	char	lineTag;
	long	size;
	unsigned short flags;

	/*
	 * Print the header for long listing.
	 */
	if (hFlag) {
	    sprintf(theLine, "T %7s %8s %8s %4s %4s %6s %s",
		    "Size", "ModTime", "ModDate",
		    "CRTR", "TYPE", "Flags", "Name");
	    Tcl_AppendResult(interp, theLine, "\n", NULL);
	    Tcl_AppendResult(interp,
		    "-------------------------------------------------------------\n",
		    NULL);
	}
		
	for (i = 0; i < argc; i++) {
	    strcpy(theFile, argv[i]);
			
	    c2pstr(theFile);
	    hpb->ioCompletion = NULL;
	    hpb->ioVRefNum = 0;
	    hpb->ioFDirIndex = 0;
	    hpb->ioNamePtr = (StringPtr) theFile;
	    hpb->ioDirID = 0L;
	    err = PBGetCatInfoSync(&paramBlock);
	    p2cstr((StringPtr) theFile);

	    if (hpb->ioFlAttrib & 16) {
		/*
		 * For directories use zero as the size, use no Creator
		 * type, and use 'DIR ' as the file type.
		 */
		if ((aFlag == false) && (dpb->ioDrUsrWds.frFlags & 0x1000)) {
		    continue;
		}
		lineTag = 'D';
		size = 0;
		IUTimeString(dpb->ioDrMdDat, FALSE, (unsigned char *)time);
		p2cstr((StringPtr)time);
		IUDateString(dpb->ioDrMdDat, shortDate, (unsigned char *)date);
		p2cstr((StringPtr)date);
		strcpy(creator, "    ");
		strcpy(type, "DIR ");
		flags = dpb->ioDrUsrWds.frFlags;
		if (fFlag || pFlag) {
		    strcat(theFile, ":");
		}
	    } else {
		/*
		 * All information for files should be printed.  This
		 * includes size, modtime, moddate, creator type, file
		 * type, flags, anf file name.
		 */
		if ((aFlag == false) &&
			(hpb->ioFlFndrInfo.fdFlags & kIsInvisible)) {
		    continue;
		}
		lineTag = 'F';
		size = hpb->ioFlLgLen + hpb->ioFlRLgLen;
		IUTimeString(hpb->ioFlMdDat, FALSE, (unsigned char *)time);
		p2cstr((StringPtr)time);
		IUDateString(hpb->ioFlMdDat, shortDate, (unsigned char *)date);
		p2cstr((StringPtr)date);
		strncpy(creator, (char *) &hpb->ioFlFndrInfo.fdCreator, 4);
		creator[4] = 0;
		strncpy(type, (char *) &hpb->ioFlFndrInfo.fdType, 4);
		type[4] = 0;
		flags = hpb->ioFlFndrInfo.fdFlags;
		if (fFlag) {
		    if (hpb->ioFlFndrInfo.fdFlags & kIsAlias) {
			strcat(theFile, "@");
		    } else if (hpb->ioFlFndrInfo.fdType == 'APPL') {
			strcat(theFile, "*");
		    }
		}
	    }
			
	    sprintf(theLine, "%c %7ld %8s %8s %-4.4s %-4.4s 0x%4.4X %s",
		    lineTag, size, time, date, creator, type, flags, theFile);
						 
	    Tcl_AppendResult(interp, theLine, "\n", NULL);
	    
	}
		
	if ((interp->result != NULL) && (*(interp->result) != '\0')) {
	    int slen = strlen(interp->result);
	    if (interp->result[slen - 1] == '\n') {
		interp->result[slen - 1] = '\0';
	    }
	}
    } else {
	/*
	 * Not in long format. We only print files names.  If the
	 * -C flag is set we need to print in multiple coloumns.
	 */
	int argCount, linePos;
	Boolean needNewLine = false;

	/*
	 * Fiend the field length: the length each string printed
	 * to the terminal will be.
	 */
	if (!cFlag) {
	    perLine = 1;
	    fieldLength = STRING_LENGTH;
	} else {
	    for (i = 0; i < argc; i++) {
		len = strlen(argv[i]);
		if (len > maxLen) {
		    maxLen = len;
		}
	    }
	    fieldLength = maxLen + 3;
	    perLine = STRING_LENGTH / fieldLength;
	}

	argCount = 0;
	linePos = 0;
	memset(theLine, ' ', STRING_LENGTH);
	while (argCount < argc) {
	    strcpy(theFile, argv[argCount]);
			
	    c2pstr(theFile);
	    hpb->ioCompletion = NULL;
	    hpb->ioVRefNum = 0;
	    hpb->ioFDirIndex = 0;
	    hpb->ioNamePtr = (StringPtr) theFile;
	    hpb->ioDirID = 0L;
	    err = PBGetCatInfoSync(&paramBlock);
	    p2cstr((StringPtr) theFile);

	    if (hpb->ioFlAttrib & 16) {
		/*
		 * Directory. If -a show hidden files.  If -f or -p
		 * denote that this is a directory.
		 */
		if ((aFlag == false) && (dpb->ioDrUsrWds.frFlags & 0x1000)) {
		    argCount++;
		    continue;
		}
		if (fFlag || pFlag) {
		    strcat(theFile, ":");
		}
	    } else {
		/*
		 * File: If -a show hidden files, if -f show links
		 * (aliases) and executables (APPLs).
		 */
		if ((aFlag == false) &&
			(hpb->ioFlFndrInfo.fdFlags & kIsInvisible)) {
		    argCount++;
		    continue;
		}
		if (fFlag) {
		    if (hpb->ioFlFndrInfo.fdFlags & kIsAlias) {
			strcat(theFile, "@");
		    } else if (hpb->ioFlFndrInfo.fdType == 'APPL') {
			strcat(theFile, "*");
		    }
		}
	    }

	    /*
	     * Print the item, taking into account multi-
	     * coloum output.
	     */
	    strncpy(theLine + (linePos * fieldLength), theFile,
		    strlen(theFile));
	    linePos++;
			
	    if (linePos == perLine) {
		theLine[STRING_LENGTH] = '\0';
		if (needNewLine) {
		    Tcl_AppendResult(interp, "\n", theLine, NULL);
		} else {
		    Tcl_AppendResult(interp, theLine, NULL);
		    needNewLine = true;
		}
		linePos = 0;
		memset(theLine, ' ', STRING_LENGTH);
	    }
			
	    argCount++;
	}
		
	if (linePos != 0) {
	    theLine[STRING_LENGTH] = '\0';
	    if (needNewLine) {
		Tcl_AppendResult(interp, "\n", theLine, NULL);
	    } else {
		Tcl_AppendResult(interp, theLine, NULL);
	    }
	}
    }
	
    ckfree((char *) argv);
	
    return TCL_OK;
} /* Tcl_LsCmd */

/*
 *----------------------------------------------------------------------
 *
 * Tcl_MkdirCmd --
 *
 *	This procedure is invoked to process the "mkdir" Tcl command.
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
Tcl_MkdirCmd (dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int           idx, dirArgc, result;
    char        **dirArgv, *dirName, *scanPtr;
    struct stat   statBuf;
    Tcl_DString   tildeBuf;

    Tcl_DStringInit (&tildeBuf);

    if ((argc < 2) || (argc > 3)) {
        goto usageError;
    }
    if ((argc == 3) && !STREQU(argv [1], "-path")) {
        goto usageError;
    }

    if (Tcl_SplitList(interp, argv [argc - 1], &dirArgc, &dirArgv) != TCL_OK) {
        return TCL_ERROR;
    }
    
    /*
     * Make all the directories, optionally making directories along the path.
     */

    for (idx = 0; idx < dirArgc; idx++) {
        dirName = Tcl_TildeSubst(interp, dirArgv [idx], &tildeBuf);
        if (dirName == NULL) {
           goto errorExit;
	}

        /*
         * Make leading directories, if requested.
         */
        if (argc == 3) {
            scanPtr = dirName;
            result = 0;  /* Start out ok, for dirs that are skipped */

            while (*scanPtr != '\0') {
                scanPtr = strchr (scanPtr+1, ':');
                if ((scanPtr == NULL) || (*(scanPtr+1) == '\0')) {
                    break;
		}
                *scanPtr = '\0';
                if (stat (dirArgv [idx], &statBuf) < 0) {
                    result = mkdir (dirArgv [idx], S_IFDIR | 0777);
		}
                *scanPtr = ':';
                if (result < 0) {
                   goto mkdirError;
		}
            }
        }
        /*
         * Make final directory in the path.
         */
        if (mkdir(dirName, S_IFDIR | 0777) != 0) {
           goto mkdirError;
	}

        Tcl_DStringFree(&tildeBuf);
    }

    ckfree((char *) dirArgv);
    return TCL_OK;

    mkdirError:
    Tcl_AppendResult(interp, dirArgv[idx], ": ",
	    Tcl_PosixError (interp), (char *) NULL);

    errorExit:
    Tcl_DStringFree(&tildeBuf);
    ckfree((char *) dirArgv);
    return TCL_ERROR;

    usageError:
    Tcl_AppendResult(interp, "wrong # args: ", argv [0], 
	    " ?-path? dirlist", (char *) NULL);
    return TCL_ERROR;
} /* Tcl_MkdirCmd */

/*
 *----------------------------------------------------------------------
 *
 * Tcl_RmdirCmd --
 *
 *     Implements the Tcl rmdir command:
 *
 * Results:
 *	Standard TCL results, may return the UNIX system error message.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_RmdirCmd (dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int          idx, dirArgc;
    char       **dirArgv, *dirName;
    int          noComplain;
    Tcl_DString  tildeBuf;

    Tcl_DStringInit (&tildeBuf);
    
    if ((argc < 2) || (argc > 3)) {
        goto badArgs;
    }

    if (argc == 3) {
        if (!STREQU(argv [1], "-nocomplain")) {
            goto badArgs;
	}
        noComplain = true;
    } else {
        noComplain = false;
    }

    if (Tcl_SplitList(interp, argv [argc - 1], &dirArgc, &dirArgv) != TCL_OK) {
        return TCL_ERROR;
    }

    for (idx = 0; idx < dirArgc; idx++) {
        dirName = Tcl_TildeSubst(interp, dirArgv [idx], &tildeBuf);
        if (dirName == NULL) {
            if (!noComplain) {
                goto errorExit;
	    }
            Tcl_DStringFree(&tildeBuf);
            continue;
        }
        if ((rmdir(dirName) != 0) && !noComplain) {
           Tcl_AppendResult(interp, dirArgv [idx], ": ",
                             Tcl_PosixError (interp), (char *) NULL);
           goto errorExit;
        }
        Tcl_DStringFree(&tildeBuf);
    }

    ckfree ((char *) dirArgv);
    return TCL_OK;

    errorExit:
    Tcl_DStringFree(&tildeBuf);
    ckfree((char *) dirArgv);
    return TCL_ERROR;;

    badArgs:
    Tcl_AppendResult(interp, "wrong # args: ", argv [0], 
	    " ?-nocomplain? dirlist", (char *) NULL);
    return TCL_ERROR;
} /* Tcl_RmdirCmd */

/*
 *----------------------------------------------------------------------
 *
 * Tcl_RmCmd --
 *
 *	This procedure is invoked to process the "rm" Tcl command.
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
Tcl_RmCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int nocomplain = 0;
    OSErr err;
    FSSpec theSpec;
    int i, j, newArgc;
    char **newArgv;

    /*
     * Process options.  A dash by itself means no more options.
     */
    for (i = 1; i < argc; i++) {
	if (argv[i][0] != '-') {
	    break;
	}
		
	if (!strcmp(argv[i], "-nocomplain")) {
	    nocomplain = 1;
	}
		
	if (!strcmp(argv[i], "-")) {
	    i++;
	    break;
	}
		
	for (j = 1 ; argv[i][j] ; ++j) {
	    switch(argv[i][j]) {
	    	case 'f':
		    nocomplain = 1;
		    break;
		default:
		    Tcl_AppendResult(interp, 
			    "usage: rm [-nocomplain | -f] ?files? ",
			    TCL_STATIC);
		    return TCL_ERROR;
	    }
	}
    }

    newArgv = argv + i;
    newArgc = argc - i;
    
    /*
     * Use glob to expand file references.
     */
    if (!globArgs(interp, &newArgc, &newArgv)) {
	return TCL_OK;
    }

    for (i = 0 ; i < newArgc ; i++) {
	if (newArgv[i] == NULL) {
	    if (!nocomplain) {
		Tcl_AppendResult(interp,
			"could not substitute for directory \"",
			newArgv[i], "\" ", (char *) NULL);
		ckfree((char *) newArgv);
		return TCL_ERROR;
	    }
	    continue;
	}
		
	c2pstr(newArgv[i]);
	if ((err = FSMakeFSSpec(0, 0, (StringPtr) newArgv[i], &theSpec))
		!= noErr) {
	    if (!nocomplain) {
		p2cstr((StringPtr) newArgv[i]);
		Tcl_AppendResult(interp, "could not locate file \"",
			newArgv[i], "\" ", Tcl_PosixError(interp),
			(char *) NULL);
		ckfree((char *) newArgv);
		return TCL_ERROR;
	    }
	    p2cstr((StringPtr) newArgv[i]);
	    continue;
	}
	p2cstr((StringPtr) newArgv[i]);
		
	if ((err = FSpDelete(&theSpec)) != noErr) {
	    if (err != noErr && !nocomplain) {
		Tcl_AppendResult(interp, "\"", newArgv[0], "\" ",
			"error deleting \"", newArgv[1], "\" ", (char *) NULL);
		ckfree((char *) newArgv);
		return TCL_ERROR;
	    }
	    continue;
	}
    }
	
    ckfree((char *) newArgv);

    return TCL_OK;
} /* Tcl_RmCmd */

/*
 *----------------------------------------------------------------------
 *
 * Tcl_CpCmd --
 *
 *	This procedure is invoked to process the "cp" Tcl command.
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
Tcl_CpCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    FSSpec fromSpec, toSpec;
    char fromName[256], toName[256];
    char *errStr;
    StringPtr fileName = NULL;
    OSErr err;
    long dirID;
    Boolean isDirectory;
    char *theName;
    Tcl_DString buffer;

    if (argc != 3) goto cpUsage;

    /*
     * Get the source files for the copy command
     */
    theName = Tcl_TildeSubst(interp, argv[1],&buffer);
    if (theName == 0) {
	Tcl_DStringFree(&buffer);
	Tcl_AppendResult(interp,"Error - Could not tilde substitute \"",
		argv[1], "\"", 0);
	return TCL_ERROR;
    }
    strcpy(fromName, theName);
    Tcl_DStringFree(&buffer);
    c2pstr(fromName);
    if (FSMakeFSSpec(0, 0, (ConstStr255Param) fromName, &fromSpec)) {
	goto cpUsage;
    }
    err = DirIDFromFSSpec(&fromSpec, &dirID, &isDirectory);
    if (err != noErr) {
	errStr = "Couldn't find source file.";
	goto cpError;
    }
    if (isDirectory) {
	errStr = "Can't copy directory.";
	goto cpError;
    }
	
    /*
     * Get destination directory or new file name
     */
    theName = Tcl_TildeSubst(interp, argv[2], &buffer);
    if (theName == 0) {
	Tcl_DStringFree(&buffer);
	Tcl_AppendResult(interp, "Error - Could not tilde substitute \"",
		argv[2], "\"", 0);
	return TCL_ERROR;
    }
    strcpy(toName, theName);
    Tcl_DStringFree(&buffer);
    c2pstr(toName);
    err = FSMakeFSSpec(0, 0, (ConstStr255Param) toName, &toSpec);
    err = DirIDFromFSSpec(&toSpec, &dirID, &isDirectory);
    if ((err == fnfErr) || !isDirectory) {
	/*
	 * Set toSpec to parent dir & fileName to actual file name
	 */
	p2cstr((StringPtr) toName);
	fileName = (StringPtr) strrchr(toName, ':');
	if (fileName == NULL) {
	    fileName = (StringPtr) toName;
	} else {
	    fileName++;
	}
	c2pstr((char *) fileName);
	FSMakeFSSpec(toSpec.vRefNum, dirID, NULL, &toSpec);
    }
		
    err = FSpFileCopy(&fromSpec, &toSpec, fileName, NULL, 0, true);
    switch (err) {
    case noErr:
	return TCL_OK;
    case dupFNErr:
	errStr = "Destination file already exists.";
	goto cpError;
    case dirNFErr:
	errStr = "Couldn't resolve destination directory.";
	goto cpError;
    case dskFulErr:
	errStr = "Not enough space to copy file.";
	goto cpError;
    case fnfErr:
	errStr = "File not found.";
	goto cpError;
    default:
	errStr = "Problem trying to copy file.";
	goto cpError;
    }
	
    cpUsage:
    errStr = "should be: cp <srcFile> [<destFile> | <destDir>]";
    
    cpError:
    Tcl_SetResult(interp, errStr, TCL_STATIC); 
    return TCL_ERROR;
} /* Tcl_CpCmd */

/*
 *----------------------------------------------------------------------
 *
 * Tcl_MvCmd --
 *
 *	This procedure is invoked to process the "cp" Tcl command.
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
Tcl_MvCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    OSErr err;
    Boolean isDirectory, doMoveRename;
    long fromID, toID, infoSize;
    FSSpec fromSpec, newDirSpec;
    char fromName[80], newName[80];
    char *errStr;
    GetVolParmsInfoBuffer volParmsInfo;
	
    if (argc != 3) {
	goto mvUsage;
    }

    /*
     * Get file that will be moved
     */
    strcpy(fromName, argv[1]);
    c2pstr(fromName);
    if (FSMakeFSSpec(0, 0, (ConstStr255Param) fromName, &fromSpec)) {
	goto mvUsage;
    }
    err = DirIDFromFSSpec(&fromSpec, &fromID, &isDirectory);
    if (err != noErr) {
	errStr = "Couldn't find source file.";
	goto mvError;
    }

    /*
     * First check case of renaming (or just a dir name not a path)
     */
    strcpy(newName, argv[2]);
    c2pstr(newName);
    if (strchr(newName, ':') == NULL) {
	err = FSpRename(&fromSpec, (StringPtr) newName);
	if (err == noErr) {
	    return TCL_OK;
	} else if (err == dupFNErr) {
	    /*
	     * File may be dirctory, change to ":newName" & continue
	     */
	    newName[0] = ':';
	    c2pstr(newName);
	} else {
	    errStr = "Error occured while renaming file.";
	    goto mvError;
	}
    }

    /*
     * Check the volume to see what operations we can support
     */
    err = HGetVolParms(NULL, fromSpec.vRefNum, &volParmsInfo, &infoSize);
    if (err == noErr) {
	doMoveRename = hasMoveRename(volParmsInfo);
    } else {
	doMoveRename = false;
    }
	
    /*
     * Get destination directory (and maybe new file name as well)
     */
    err = FSMakeFSSpec(0, 0, (ConstStr255Param) newName, &newDirSpec);
    err = DirIDFromFSSpec(&newDirSpec, &toID, &isDirectory);
    if (err == noErr) {
	if (isDirectory) {
	    if (fromID == toID) {
		errStr = "Destination same as source.";
		goto mvError;
	    }
	    if (fromSpec.vRefNum != newDirSpec.vRefNum) {
		errStr = "Can't move across volumes.";
	    }
	    /*
	     * Changing location of file, name will remain the same
	     */
	    err = FSpCatMove(&fromSpec, &newDirSpec);
	} else {
	    errStr = "File already exists.";
	    goto mvError;
	}
    } else {
	if (err == fnfErr) {
	    /*
	     * Set newDirSpec to parent dir & fileName to actual file name
	     */
	    strcpy(newName, (char *) newDirSpec.name);
	    err = FSMakeFSSpec(newDirSpec.vRefNum, toID, NULL, &newDirSpec);
	    err = DirIDFromFSSpec(&newDirSpec, &toID, &isDirectory);
	    if (err != noErr || !isDirectory) {
		errStr = "No such directory.";
		goto mvError;
	    }
	    if (doMoveRename) {
		err = FSpMoveRename(&fromSpec, &newDirSpec,
			(StringPtr) newName);
	    } else {
		/*
		 * First do move
		 */
		err = FSpCatMove(&fromSpec, &newDirSpec);
		if (err == noErr) {
		    /*
		     * If that worked, rename the file
		     */
		    err = DirIDFromFSSpec(&newDirSpec, &toID, &isDirectory);
		    err = FSMakeFSSpec(newDirSpec.vRefNum, toID,
			    fromSpec.name, &newDirSpec);
		    err = FSpRename(&newDirSpec, (StringPtr) newName);
		}
	    }			
	}
    }
	
    if (err != noErr) {
	errStr = "an unknown error occured.";
	goto mvError;
    } else {
	return TCL_OK;
    }
	
    mvUsage:
    errStr = "should be: mv <srcFile> [<destFile> | <destDir>]";
    
    mvError:
    Tcl_SetResult(interp, errStr, TCL_STATIC); 
    return TCL_ERROR;
} /* Tcl_MvCmd */

/*
 *----------------------------------------------------------------------
 *
 * Mac_SourceCmd --
 *
 *	This procedure is invoked to process the "source" Tcl command.
 *	See the user documentation for details on what it does.  In addition,
 *	it supports sourceing from the resource fork of type 'TEXT'.
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
Mac_SourceCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    char *errNum = "wrong # args: ";
    char *errBad = "bad argument: ";
    char *errStr;
    char *fileName = NULL, *rsrcName = NULL;
    int rsrcID = -1, i;

    if (argc < 2)  {
    	errStr = errNum;
    	goto sourceFmtErr;
    }
    
    if (argc == 2)  {
	return Tcl_EvalFile(interp, argv[1]);
    }
    
    for (i = 1 ; i < argc ; i++) {
	if (!strcmp(argv[i], "-rsrc") || !strcmp(argv[i], "-rsrcname")) {
	    rsrcName = argv[i + 1];
	    i++;
	} else if (!strcmp(argv[i], "-rsrcid")) {
	    rsrcID = atoi(argv[i + 1]);
	    i++;
	} else if (!strcmp(argv[i], "-file") ||
		!strcmp(argv[i], "-rsrcfile")) {
	    fileName = argv[i + 1];
	    i++;
	} else {
	    errStr = errBad;
	    goto sourceFmtErr;
	}
    }
	
    if (rsrcName == NULL && rsrcID < 0) {
    	errStr = errBad;
    	goto sourceFmtErr;
    }
	
    return Mac_EvalResource(interp, rsrcName, rsrcID, fileName);
	
    sourceFmtErr:
    Tcl_AppendResult(interp, errStr, "should be \"", argv[0],
	    " fileName\" or \"", argv[0], " -rsrc name ?-file path?\" or \"",
	    argv[0], " -rsrcid id ?-file path?\"", (char *) NULL);
    return TCL_ERROR;
} /* Mac_SourceCmd */

/*
 *----------------------------------------------------------------------
 *
 * Mac_BeepCmd --
 *
 *	This procedure makes the beep sound.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	Makes a beep.
 *
 *----------------------------------------------------------------------
 */

#pragma require_prototypes off
int
Mac_BeepCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    Handle sound;
    Str255 sndName;
    int volume = -1;
    char * sndArg = NULL;
    short curVolume;

    if (argc == 1) {
	SysBeep(1);
	return TCL_OK;
    } else if (argc == 2) {
	if (!strcmp(argv[1], "-list")) {
	    int count, i;
	    short id;
	    Str255 theName;
	    ResType theType;
			
	    Tcl_ResetResult(interp);
	    count = CountResources('snd ');
	    for (i = 1; i <= count; i++) {
		sound = GetIndResource('snd ', i);
		if (sound != NULL) {
		    GetResInfo(sound, &id, &theType, theName);
		    if (theName[0] == 0) {
			continue;
		    }
		    theName[theName[0]+1] = '\0';
		    Tcl_AppendElement(interp, (char *) theName + 1);
		}
	    }
	    return TCL_OK;
	} else {
	    sndArg = argv[1];
	}
    } else if (argc == 3) {
	if (!strcmp(argv[1], "-volume")) {
	    volume = atoi(argv[2]);
	} else {
	    goto beepUsage;
	}
    } else if (argc == 4) {
	if (!strcmp(argv[1], "-volume")) {
	    volume = atoi(argv[2]);
	    sndArg = argv[3];
	} else {
	    goto beepUsage;
	}
    } else {
	goto beepUsage;
    }
	
    /*
     * Set Volume
     */
    if (volume >= 0) {
	/* TODO: this needs to be updated to work with 3.0 */
	/* Get/SetSysBeepVolume */
	GetSoundVol(&curVolume);
	SetSoundVol((short) volume);
    }
	
    /*
     * Play the sound
     */
    if (sndArg == NULL) {
	SysBeep(1);
    } else {
	strcpy((char *) sndName + 1, sndArg);
	sndName[0] = strlen(sndArg);
	sound = GetNamedResource('snd ', sndName);
	if (sound != NULL) {
#if (THINK_C == 7)
	    SndPlay(NULL, sound, FALSE);
#else
	    SndPlay(NULL, (SndListHandle) sound, FALSE);
#endif
	    return TCL_OK;
	} else {
	    if (volume >= 0) {
		SetSoundVol(curVolume);
	    }
	    Tcl_ResetResult(interp);
	    Tcl_AppendResult(interp, " \"", sndArg, 
		    "\" is not a valid sound.  (Try ", argv[0],
		    " -list)", NULL);
	    return TCL_ERROR;
	}
    }

    /*
     * Reset Volume
     */
    if (volume >= 0) {
	SetSoundVol(curVolume);
    }
    return TCL_OK;

    beepUsage:
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " [-volume num] [-list | sndName]?\"", (char *) NULL);
    return TCL_ERROR;
} /* Mac_BeepCmd */
#pragma require_prototypes reset

/*
 *----------------------------------------------------------------------
 *
 * Mac_PutsCmd --
 *
 *	This procedure is invoked to process the "puts" Tcl command.
 *	It behaves much like the Unix version, however, it has the option
 *	to redirect io to another function.
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
Mac_PutsCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    FILE *f;
    int i, newline;
    char *fileId;

    i = 1;
    newline = 1;
    if ((argc >= 2) && (strcmp(argv[1], "-nonewline") == 0)) {
	newline = 0;
	i++;
    }
    if ((i < (argc-3)) || (i >= argc)) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" ?-nonewline? ?fileId? string\"", (char *) NULL);
	return TCL_ERROR;
    }

    /*
     * The code below provides backwards compatibility with an old
     * form of the command that is no longer recommended or documented.
     */

    if (i == (argc-3)) {
	if (strncmp(argv[i+2], "nonewline", strlen(argv[i+2])) != 0) {
	    Tcl_AppendResult(interp, "bad argument \"", argv[i+2],
		    "\": should be \"nonewline\"", (char *) NULL);
	    return TCL_ERROR;
	}
	newline = 0;
    }
    if (i == (argc-1)) {
	fileId = "stdout";
    } else {
	fileId = argv[i];
	i++;
    }

    if (Tcl_GetOpenFile(interp, fileId, 1, 1, &f) != TCL_OK) {
	return TCL_ERROR;
    }

    clearerr(f);
    fputs(argv[i], f);
    if (newline) {
	fputc('\n', f);
    }
    if (ferror(f)) {
	Tcl_AppendResult(interp, "error writing \"", fileId,
		"\": ", Tcl_PosixError(interp), (char *) NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
} /* Mac_PutsCmd */

/*
 *----------------------------------------------------------------------
 *
 * MacOpen, etc. --
 *
 *	Below are a bunch of procedures that are used by Tcl instead
 *	of system calls.  The MacTcl versions differ from the Tcl
 *	versions in the fact that MacTcl doesn't support signals.
 *
 * Results:
 *	Whatever the system call would normally return.
 *
 * Side effects:
 *	Whatever the system call would normally do.
 *
 * NOTE:
 *	This should be the last page of this file, since it undefines
 *	the macros that redirect read etc. to the procedures below.
 *
 *----------------------------------------------------------------------
 */

#undef open
int
MacOpen(path, oflag, mode)
    char *path;
    int oflag;
    int mode;
{
/*
 * The Think C unix lib does not support the mode
 * Set open to automatically convert 0x0d<->0x0a
 */
#ifdef THINK_C
    oflag |= O_TEXT;
#endif
    return open(path, oflag);
} /* MacOpen */

#undef read
int
MacRead(fd, buf, numBytes)
    int fd;
    VOID *buf;
    size_t numBytes;
{
    return read(fd, buf, (size_t) numBytes);
} /* MacRead */

#undef waitpid
#ifdef __MWERKS__
int
MacWaitpid(pid_t pid, int *statPtr, int options)
#else
int
MacWaitpid(pid, statPtr, options)
    pid_t pid;
    int *statPtr;
    int options;
#endif
{
    return -1;
} /* MacWaitpid */

#undef write
int
MacWrite(fd, buf, numBytes)
    int fd;
    VOID *buf;
    size_t numBytes;
{
    return write(fd, buf, (size_t) numBytes);
} /* MacWrite */

