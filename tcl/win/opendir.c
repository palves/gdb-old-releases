/* 
 * opendir.c --
 *
 *	This file is used to provide directory related compatibility
 *	functions under Visual C++.  This file is not needed for
 *	Borland C++.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tclInt.h"
#include "tclPort.h"

/*
 * State that keeps track of the reading of a directory.
 */

typedef struct _dirdesc {
    HANDLE findfile_handle;	/* Handle for use by FindFirstFile,
				 * FindCloseFile and FindNextFile. */
    WIN32_FIND_DATA findfile_data;
				/* Data found for a file. */
    BOOL done;			/* Is this search exhausted? */
} DIR;

static DIR globalDir;
static struct dirent globalDirent;


/*
 * Open a directory.
 */
DIR *
opendir(name)
char *name;
{
    char findfile_pattern[512];
    DIR *dirp = &globalDir;
    
    if (name == NULL || *name == '\0') {
	name = ".";
    }
    sprintf(findfile_pattern, "%s\\*.*", name);
    dirp->findfile_handle =
        FindFirstFile(findfile_pattern, &(dirp->findfile_data));
    if (dirp->findfile_handle == INVALID_HANDLE_VALUE) {
        return NULL;
    }
    dirp->done = FALSE;
    
    return dirp;
}

/*
 * Get next entry in a directory.
 */
struct dirent *
readdir(dirp)
    register DIR *dirp;
{
    struct dirent *dp = &globalDirent;

    if (dirp->done) {
	return NULL;
    }

    dp->d_namlen = strlen(dirp->findfile_data.cFileName);
    strcpy(dp->d_name, dirp->findfile_data.cFileName);

    dp->d_ino = 0;
    dp->d_reclen = 0;

    if (!FindNextFile(dirp->findfile_handle, &(dirp->findfile_data))) {
        dirp->done = TRUE;
    }
    
    return dp;
}

/*
 * Close a directory.
 */
void
closedir(dirp)
    register DIR *dirp;
{
    (void) FindClose(dirp->findfile_handle);
}
