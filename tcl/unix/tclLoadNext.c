/* 
 * tclLoadNext.c --
 *
 *	This procedure provides a version of the TclLoadFile that
 *	works with NeXTs rld_* dynamic loading.  This file provided
 *	by Pedja Bogdanovich.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */


#include "tclInt.h"
#include <mach-o/rld.h>
#include <streams/streams.h>

/*
 *----------------------------------------------------------------------
 *
 * TclLoadFile --
 *
 *	Dynamically loads a binary code file into memory and returns
 *	the addresses of two procedures within that file, if they
 *	are defined.
 *
 * Results:
 *	A standard Tcl completion code.  If an error occurs, an error
 *	message is left in interp->result.  *proc1Ptr and *proc2Ptr
 *	are filled in with the addresses of the symbols given by
 *	*sym1 and *sym2, or NULL if those symbols can't be found.
 *
 * Side effects:
 *	New code suddenly appears in memory.
 *
 *----------------------------------------------------------------------
 */

int
TclLoadFile(interp, fileName, sym1, sym2, proc1Ptr, proc2Ptr)
    Tcl_Interp *interp;		/* Used for error reporting. */
    char *fileName;		/* Name of the file containing the desired
				 * code. */
    char *sym1, *sym2;		/* Names of two procedures to look up in
				 * the file's symbol table. */
    Tcl_PackageInitProc **proc1Ptr, **proc2Ptr;
				/* Where to return the addresses corresponding
				 * to sym1 and sym2. */
{
  struct mach_header *header;
  char *data;
  int len, maxlen;
  char *files[]={fileName,NULL};
  NXStream *errorStream=NXOpenMemory(0,0,NX_READWRITE);

  if(!rld_load(errorStream,&header,files,NULL)) {
    NXGetMemoryBuffer(errorStream,&data,&len,&maxlen);
    Tcl_AppendResult(interp,"couldn't load file \"",fileName,"\": ",data,NULL);
    NXCloseMemory(errorStream,NX_FREEBUFFER);
    return TCL_ERROR;
  }
  NXCloseMemory(errorStream,NX_FREEBUFFER);

  *proc1Ptr=NULL;
  if(sym1) {
    char sym[strlen(sym1)+2];
    sym[0]='_'; sym[1]=0; strcat(sym,sym1);
    rld_lookup(NULL,sym,(unsigned long *)proc1Ptr);
  }

  *proc2Ptr=NULL;
  if(sym2) {
    char sym[strlen(sym2)+2];
    sym[0]='_'; sym[1]=0; strcat(sym,sym2);
    rld_lookup(NULL,sym,(unsigned long *)proc2Ptr);
  }

  return TCL_OK;
}