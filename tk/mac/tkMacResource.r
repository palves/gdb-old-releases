/*
 * tkMacResources.r --
 *
 *	This file creates resources for use in a simple shell.
 *	This is designed to be an example of using the Tcl/Tk 
 *	libraries in a Macintosh Application.
 *
 * Copyright (c) 1993-94 Lockheed Missle & Space Company, AI Center
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/* 
 * Resources for a Simple Tcl/Tk Shell 
 */

#include "Types.r"
#include "SysTypes.r"

#ifdef applec
#	define	__kPrefSize 384
#	define	__kMinSize 256	
#	include "siow.r"
#	ifdef GUSI
#		include "gusi.r"
#	endif
#endif

#define RELEASE_CODE 0x00
#define TCL_LIBRARY "Dev:export:proj:mactcl7.5:library"
#define TK_LIBRARY "Dev:export:proj:tk4.x:library"

resource 'vers' (1) {
	0x04, 0x01, beta,
	RELEASE_CODE, 0,
	"4.1.0",
	"4.1.0" ", by Ray Johnson © 1993-95" "\n" "Sun Microsystems Labratories"
};

resource 'vers' (2) {
	0x04, 0x01, beta,
	RELEASE_CODE, 0,
	"4.1.0",
	"Simple Tcl & Tk Shell 4.1 © 1993-95"
};


/* 
 * The mechanisim below loads Tcl source into the resource fork of the
 * application.  The example below creates a TEXT resource named
 * "Init" from the file "init.tcl".  This allows applications to use
 * Tcl to define the behavior of the application without having to
 * require some predetermined file structure - all needed Tcl "files"
 * are located within the application.  To source a file for the
 * resource fork the source command has been modified to support
 * sourcing from resources.  In the below case "source -rsrc {Init}"
 * will load the TEXT resource named "Init".
 */

read 'TEXT' (0, "Init", purgeable, preload) TCL_LIBRARY":init.tcl";
read 'TEXT' (1, "tk", purgeable, preload) TK_LIBRARY":tk.tcl";
read 'TEXT' (2, "button", purgeable, preload) TK_LIBRARY":button.tcl";
read 'TEXT' (3, "dialog", purgeable, preload) TK_LIBRARY":dialog.tcl";
read 'TEXT' (4, "entry", purgeable, preload) TK_LIBRARY":entry.tcl";
read 'TEXT' (5, "focus", purgeable, preload) TK_LIBRARY":focus.tcl";
read 'TEXT' (6, "listbox", purgeable, preload) TK_LIBRARY":listbox.tcl";
read 'TEXT' (7, "menu", purgeable, preload) TK_LIBRARY":menu.tcl";
read 'TEXT' (8, "optionMenu", purgeable, preload) TK_LIBRARY":optMenu.tcl";
read 'TEXT' (9, "palette", purgeable, preload) TK_LIBRARY":palette.tcl";
read 'TEXT' (10, "scale", purgeable, preload) TK_LIBRARY":scale.tcl";
read 'TEXT' (11, "scrollbar", purgeable, preload) TK_LIBRARY":scrlbar.tcl";
read 'TEXT' (12, "tearoff", purgeable, preload) TK_LIBRARY":tearoff.tcl";
read 'TEXT' (13, "text", purgeable, preload) TK_LIBRARY":text.tcl";
read 'TEXT' (14, "tkerror", purgeable, preload) TK_LIBRARY":tkerror.tcl";
read 'TEXT' (15, "Console", purgeable, preload) TK_LIBRARY":console.tcl";

/*
 * The following resource is used when creating the 'env' variable in
 * the Macintosh environment.  The creation mechanisim looks for the
 * 'STR#' resource named "Tcl Environment Variables" rather than a
 * specific resource number.  (In other words, feel free to change the
 * resource id if it conflicts with your application.)  Each string in
 * the resource must be of the form "KEYWORD=SOME STRING".  See Tcl
 * documentation for futher information about the env variable.
 */
 
/* A good example of something you may want to set is:
 * "TCL_LIBRARY=My disk:etc." 
 */
		
resource 'STR#' (128, "Tcl Environment Variables") {
	{	"SCHEDULE_NAME=Agent Controller Schedule",
		"SCHEDULE_PATH=Lozoya:System Folder:Tcl Lib:Tcl-Scheduler"
	};
};
