/*
 * tclMacPort.h --
 *
 *	This header file handles porting issues that occur because of
 *	differences between the Mac and Unix. It should be the only
 *	file that contains #ifdefs to handle different flavors of OS.
 *
 * Copyright (c) 1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef _MACPORT
#define _MACPORT

#ifndef _TCL
#include "tcl.h"
#endif

/* Includes */
#ifdef THINK_C
#   include <pascal.h>
#   include <posix.h>
#   include <string.h>
#   include <errno.h>
#   include <fcntl.h>
#   include <pwd.h>
#   include <signal.h>
#   include <sys/param.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <dirent.h>
#   include <unistd.h>
#elif defined(__MWERKS__)
#   include <Files.h>
#   include <sys/types.h>

/*
 * We must explicitly include both errno.h and
 * sys/errno.h.  sys/errno.h is defined in CWGUSI
 */
#   include "errno.h"
#   include "sys/errno.h"

#   include <signal.h>
#   include <unistd.h>
#   include <sys/unistd.h>
#   include <sys/fcntl.h>
#   include <sys/stat.h>
#   define isatty(arg) 1
#endif

/*
 * waitpid doesn't work on a Mac - the following makes
 * Tcl compile without errors.  These would normally
 * be defined in sys/wait.h
 */
#ifdef NO_SYS_WAIT_H
#   define WNOHANG 1
#   define WIFSTOPPED(stat) (1)
#   define WIFSIGNALED(stat) (1)
#   define WIFEXITED(stat) (1)
#   define WIFSTOPSIG(stat) (1)
#   define WIFTERMSIG(stat) (1)
#   define WIFEXITSTATUS(stat) (1)
#   define WEXITSTATUS(stat) (1)
#   define WTERMSIG(status) (1)
#   define WSTOPSIG(status) (1)
#endif

/*
 * These functions always return dummy values on Mac.
 */
#define geteuid() 1
#define getpid() -1

/*
 * Macros to do string compares.  They pre-check the first character
 * before checking if the strings are equal.
 */

#define STREQU(str1, str2) \
        (((str1) [0] == (str2) [0]) && (strcmp (str1, str2) == 0))
#define STRNEQU(str1, str2, cnt) \
        (((str1) [0] == (str2) [0]) && (strncmp (str1, str2, cnt) == 0))

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define NO_SYS_ERRLIST

/*
 * Make sure that MAXPATHLEN is defined.
 */

#define WAIT_STATUS_TYPE pid_t

#ifndef MAXPATHLEN
#   ifdef PATH_MAX
#       define MAXPATHLEN PATH_MAX
#   else
#       define MAXPATHLEN 2048
#   endif
#endif

/*
 * Substitute Tcl's own versions for several system calls.  The
 * Tcl versions retry automatically if interrupted by signals.
 * (see tclUnixUtil.c).
 */

#define open(a,b,c) MacOpen(a,b,c)
#define read(a,b,c) MacRead(a,b,c)
#define waitpid(a,b,c) MacWaitpid(a,b,c)
#define write(a,b,c) MacWrite(a,b,c)
EXTERN int	MacOpen _ANSI_ARGS_((char *path, int oflag, int mode));
EXTERN int	MacRead _ANSI_ARGS_((int fd, VOID *buf, size_t numBytes));
EXTERN int	MacWaitpid _ANSI_ARGS_((pid_t pid, int *statPtr, int options));
EXTERN int	MacWrite _ANSI_ARGS_((int fd, VOID *buf, size_t numBytes));

/*
 * The following functions are declared in tclInt.h but don't do anything
 * on Macintosh systems.
 */

#define TclRemoveTempFiles()
#define TclSetSystemEnv(a,b)

/*
 * The following macro defines the character that marks the end of
 * a line.
 */

#define NEWLINE_CHAR '\r'

/*
 * Many signals are not supported on the Mac and are thus not defined in
 * <signal.h>.  They are defined here so that Tcl will compile with less
 * modification.
  */

#ifndef SIGQUIT
#define SIGQUIT 300
#endif

#ifndef SIGPIPE
#define SIGPIPE 13
#endif

#ifndef SIGHUP
#define SIGHUP  100
#endif

/* A few error types are used by Tcl that don't make sense on a Mac */
/* They are defined here so that Tcl will compile with less modification */
#define ECHILD	10

extern char **environ;

/*
 * Prototypes of Mac-Unix Tcl Commands
 */
EXTERN int 	Tcl_LsCmd _ANSI_ARGS_((ClientData dummy, Tcl_Interp *interp,
		    int argc, char **argv));
EXTERN int 	Tcl_MkdirCmd _ANSI_ARGS_((ClientData dummy, Tcl_Interp *interp,
		    int argc, char **argv));
EXTERN int 	Tcl_RmdirCmd _ANSI_ARGS_((ClientData dummy, Tcl_Interp *interp,
		    int argc, char **argv));
EXTERN int 	Tcl_RmCmd _ANSI_ARGS_((ClientData dummy, Tcl_Interp *interp,
		    int argc, char **argv));
EXTERN int 	Tcl_CpCmd _ANSI_ARGS_((ClientData dummy, Tcl_Interp *interp,
		    int argc, char **argv));
EXTERN int 	Tcl_MvCmd _ANSI_ARGS_((ClientData dummy, Tcl_Interp *interp,
		    int argc, char **argv));
EXTERN int 	Tcl_EchoCmd _ANSI_ARGS_((ClientData dummy, Tcl_Interp *interp,
		    int argc, char **argv));
EXTERN int 	Mac_PutsCmd _ANSI_ARGS_((ClientData dummy, Tcl_Interp *interp,
		    int argc, char **argv));
EXTERN int 	Mac_EvalResource _ANSI_ARGS_((Tcl_Interp *interp,
		    char *resourceName, int resourceNumber, char *fileName));

/*
 * Prototypes for Mac Tcl commands
 */
EXTERN int 	Mac_BeepCmd _ANSI_ARGS_((ClientData dummy, Tcl_Interp *interp,
		    int argc, char **argv));
EXTERN int 	Mac_SourceCmd _ANSI_ARGS_((ClientData dummy,
		    Tcl_Interp *interp, int argc, char **argv));

/*
 * Prototypes for functions found in the POSIX compatability library
 */
int 	FSpGetDefaultDir _ANSI_ARGS_((FSSpecPtr theSpec));
int 	FSpSetDefaultDir _ANSI_ARGS_((FSSpecPtr theSpec));
OSErr 	FSpFindFolder _ANSI_ARGS_((short vRefNum, OSType folderType,
	Boolean createFolder, FSSpec *spec));

/*
 * Other prototypes needed for compatability
 */
int 	TclMacCreateEnv _ANSI_ARGS_((void));
int	globArgs _ANSI_ARGS_((Tcl_Interp *interp, int *argc, char ***argv));
Boolean	isFullPathName _ANSI_ARGS_((char * path));
char * 	ConvertPartial2Full _ANSI_ARGS_((char * fileName));
int	SpinCursor _ANSI_ARGS_((void));
int	SPINCURSOR _ANSI_ARGS_((void));

extern FILE * fdopen(int fd, const char *mode);
extern int fileno(FILE *stream);

#if (defined(THINK_C) || defined(__MWERKS__))
double	hypot(double x, double y);
#endif

/*
 * These definitions force putenv & company to use the version
 * supplied with Tcl.
 */
#ifndef putenv
#   define unsetenv	TclUnsetEnv
#   define putenv	Tcl_PutEnv
#   define setenv	TclSetEnv
void	TclSetEnv(CONST char *name, CONST char *value);
int	Tcl_PutEnv(CONST char *string);
void	TclUnsetEnv(CONST char *name);
char *	TclGetEnv(CONST char *name);
#endif

#endif /* _MACPORT */
