/* 
 * tclMacUtil.c --
 *
 *  This contains utility functions used to help with
 *  implementing Macintosh specific portions of the Tcl port.
 *
 * Copyright 1993-1994 Lockheed Missle & Space Company, AI Center
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tcl.h"
#include "tclInt.h"
#include "tclPort.h"
#include <math.h>

#include <Errors.h>
#include <Files.h>
#include <Folders.h>
#include <Fonts.h>
#include <Memory.h>
#include <Packages.h>
#include <Resources.h>
#include <Script.h>
#include <Strings.h>
#include <TextUtils.h>
#include <ToolUtils.h>

/* The following two Includes are from the More Files package */
#include <FileCopy.h>
#include <MoreFiles.h>
#include <FullPath.h>

/*
 *----------------------------------------------------------------------
 *
 * ConvertPartial2Full --
 *
 *	Converts a partial path name to a full path name using the default
 *	directory as a starting point.
 *
 * Results:
 *	Returns malloced space to path name.
 *
 * Side effects:
 *	Returned value is malloced, remember to dealloc.
 *
 *----------------------------------------------------------------------
 */
char *
ConvertPartial2Full(fileName)
    char * fileName;
{
    char *tempName;
    FSSpec current;
    OSErr err;
    short size;

    /*
     * First get current directory - same as GetCurrentDir
     */
    FSpGetDefaultDir(&current);

    /*
     * Check to see if partial or full path name
     */
    if (isFullPathName(fileName)) {
	tempName = (char *) ckalloc((int) strlen(fileName) + 2);
	strcpy(tempName+1,fileName);
	tempName[0] = strlen(fileName);
    } else {
	Handle theString;

	err = GetFullPath(current.vRefNum, current.parID,
		current.name, &size, &theString);

	HLock(theString);
	tempName = (char *) ckalloc(size + strlen(fileName) + 2);
	strncpy(tempName + 1, *theString, size);
	if (fileName[0] == ':') {
	    strcpy(tempName + size + 1, fileName + 1);
	} else {
	    strcpy(tempName +  size + 1, fileName);
	}
	tempName[0] = strlen(tempName + 1);
	HUnlock(theString);
	DisposHandle(theString);
    }
		
    return tempName;
}

/*
 *----------------------------------------------------------------------
 *
 * isFullPathName --
 *
 *	This function determines wether a given string represents a full
 *	path name or not.  Two rules apply.  It must contain the ':' character
 *	somewhere in the path and the first character must not be a ':'.
 *
 * Results:
 *	Returns 'true' if path is a full path name, else it returns 'false'.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

Boolean
isFullPathName(path)
    char *path;
{
    char *p;

    p = strchr(path, ':');
	
    if (p == path || p == NULL) {
	return false;
    }
	
    return true;
}

/*
 *----------------------------------------------------------------------
 *
 * globArgs --
 *
 *	The following function was taken from Peter Keleher's Alpha Editor.
 *	*argc should only count the end arguments that should be globed.
 *	argv should be incremented to point to the first arg to be globed.
 *
 * Results:
 *	Returns 'true' if it worked & memory was allocated, else 'false'.
 *
 * Side effects:
 *	argv will be alloced, the call will need to release the memory
 *
 *----------------------------------------------------------------------
 */
 
int
globArgs(interp, argc, argv)
    Tcl_Interp *interp;
    int *argc;
    char ***argv;
{
    int res, len;
    char *list;
	
    /*
     * Places the globbed args all into 'interp->result' as a list.
     */
    res = Tcl_GlobCmd(NULL, interp, *argc + 1, *argv - 1);
    if (res != TCL_OK) {
	return FALSE;
    }
    len = strlen(interp->result);
    list = (char *)ckalloc(len + 1);
    strcpy(list, interp->result);
    Tcl_ResetResult(interp);
	
    res = Tcl_SplitList(interp, list, argc, argv);
    if (res != TCL_OK) {
	return false;
    }
    return true;
}

/*
 *----------------------------------------------------------------------
 *
 * hypot --
 *
 *	The standard math function hypot is not supported by Think C.
 *	It is included here so everything works.
 *
 * Results:
 *	Result of computation.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */
 
#if defined(THINK_C)
double
hypot(x, y)
    double x, y;
{
    double sum;

    sum = x*x + y*y;
    return sqrt(sum);
}
#endif

/*
 *-----------------------------------------------------------------------------
 *
 * Mac_EvalResource --
 *    Used to extend the source command.  Sources Tcl code from a Text
 *    resource.  Currently only sources the resouce by name file ID may be
 *    supported at a later date.
 *
 * Side Effects:
 *	Depends on the Tcl code in the resource.
 *
 * Results:
 *      Returns a Tcl result.
 *
 *-----------------------------------------------------------------------------
 */
int
Mac_EvalResource(interp, resourceName, resourceNumber, fileName)
    Tcl_Interp *interp;		/* Interpreter in which to process file. */
    char *resourceName;		/* Name of TEXT resource to source,
				   NULL if number should be used. */
    int resourceNumber;		/* Resource id of source. */
    char *fileName;		/* Name of file to process.
				   NULL if application resource. */
{
    Handle sourceText;
    Str255 rezName;
    char msg[200];
    int result;
    short saveRef, fileRef = -1;
    char idStr[64];
    FSSpec fileSpec;

    saveRef = CurResFile();
	
    if (fileName != NULL) {
	OSErr err;
		
	c2pstr(fileName);
	err = FSMakeFSSpec(0, 0, (ConstStr255Param) fileName, &fileSpec);
	if (err != noErr) {
	    result = TCL_ERROR;
	    goto rezEvalError;
	}
		
	fileRef = FSpOpenResFile(&fileSpec, fsRdPerm);
	if (fileRef == -1) {
	    result = TCL_ERROR;
	    goto rezEvalError;
	}
		
	UseResFile(fileRef);
    } else {
	/*
	 * The default behavior will search through all open resource files.
	 * This may not be the behavior you desire.  If you want the behavior
	 * of this call to *only* search the application resource fork, you
	 * must call UseResFile at this point to set it to the application
	 * file.  This means you must have already obtained the application's 
	 * fileRef when the application started up.
	 */
    }
	
    /* Load the resource by name or ID */
    if (resourceName != NULL) {
	strcpy((char *) rezName + 1, resourceName);
	rezName[0] = strlen(resourceName);
	sourceText = GetNamedResource('TEXT', rezName);
    } else {
	sourceText = GetResource('TEXT', (short) resourceNumber);
    }
	
    if (sourceText == NULL) {
	result = TCL_ERROR;
    } else {
	int i, size;
	char *theSource = NULL, *allocSpace = NULL;
	char lastChar;
	
	HLock(sourceText);

	size = SizeResource(sourceText);
		
	/*
	 * To avoid cutting off part of some needed code we may 
	 * have to alloc space.
	 */
	
	lastChar = (*sourceText)[size - 1];
	if (lastChar == ' ' || lastChar == '\n' ||
		lastChar == '\t' || lastChar == '\r') {
	    (*sourceText)[size - 1] = '\0';
	    theSource = *sourceText;
	} else {
	    allocSpace = (char *) ckalloc(size + 1);
	    strncpy(allocSpace, *sourceText, size);
	    allocSpace[size] = '\0';
	    theSource = allocSpace;
	}
		
	/*
	 * Convert all carriage returns to newlines
	 */
	for (i=0; i<size; i++) {
	    if (theSource[i] == '\r') {
		theSource[i] = '\n';
	    }
	}
		
	/*
	 * We now evaluate the Tcl source
	 */
	result = Tcl_Eval(interp, theSource);
	if (result == TCL_RETURN) {
	    result = TCL_OK;
	} else if (result == TCL_ERROR) {
	    sprintf(msg, "\n    (rsrc \"%.150s\" line %d)", resourceName,
		    interp->errorLine);
	    Tcl_AddErrorInfo(interp, msg);
	}
		
	if (allocSpace != NULL) {
	    ckfree(allocSpace);
	}
	HUnlock(sourceText);
	ReleaseResource(sourceText);
		
	goto rezEvalCleanUp;
    }
	
    rezEvalError:
    sprintf(idStr, "ID=%d", resourceNumber);
    Tcl_AppendResult(interp, "The resource \"",
	    (resourceName != NULL ? resourceName : idStr),
	    "\" could not be loaded from ",
	    (fileName != NULL ? fileName : "application"),
	    ".", NULL);

    rezEvalCleanUp:
    if (fileRef != -1) {
	CloseResFile(fileRef);
    }

    UseResFile(saveRef);
	
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * FSpGetDefaultDir --
 *
 *	This function sets the provided FSSpec to the location of the 
 *	default directory.
 *
 * Results:
 *	The provided FSSpec is changed to point to the "default" directory.
 *	The function returns what ever errors FSMakeFSSpec may encounter.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
FSpGetDefaultDir(theSpec)
	FSSpecPtr theSpec;
{
    OSErr err;
    short vRefNum = 0;
    long int dirID = 0;

    err = HGetVol(NULL, &vRefNum, &dirID);
	
    if (err == noErr) {
	err = FSMakeFSSpec(vRefNum, dirID, (ConstStr255Param) NULL, theSpec);
    }
	
    return err;
}

/*
 *----------------------------------------------------------------------
 *
 * FSpSetDefaultDir --
 *
 *	This function sets the default directory to the directory pointed to 
 *	by the provided FSSpec.
 *
 * Results:
 *	The function returns what ever errors HSetVol may encounter.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
FSpSetDefaultDir(theSpec)
	FSSpecPtr theSpec;
{
    OSErr err;

    err = HSetVol( theSpec->name, theSpec->vRefNum, theSpec->parID );
    return err;
}

/*
 *----------------------------------------------------------------------
 *
 * FSpFindFolder --
 *
 *	This function is a version of the FindFolder function that returns
 *	the result as a FSSpec rather than a vRefNum and dirID.
 *
 * Results:
 *	Results will be simaler to that of the FindFolder function.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

#ifdef __MWERKS__
OSErr
FSpFindFolder(short vRefNum, OSType folderType, Boolean createFolder,
	FSSpec *spec)
#else
OSErr
FSpFindFolder(vRefNum, folderType, createFolder, spec)
    short vRefNum;
    OSType folderType;
    Boolean createFolder;
    FSSpec *spec;
#endif
{
    short foundVRefNum;
    long foundDirID;
    OSErr err;

    err = FindFolder(vRefNum, folderType, createFolder,
	    &foundVRefNum, &foundDirID);
    if (err != noErr) {
	return err;
    }
		
    err = FSMakeFSSpec(foundVRefNum, foundDirID, "\p", spec);
    return err;
}
