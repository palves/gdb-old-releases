/*
 * tclWinPort.h --
 *
 *	This header file handles porting issues that occur because of
 *	differences between Windows and Unix. It should be the only
 *	file that contains #ifdefs to handle different flavors of OS.
 *
 * Copyright (c) 1994-1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *

#ifndef _TCLWINPORT
#define _TCLWINPORT

#include <malloc.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <process.h>
#include <signal.h>
#include <winsock.h>
#include <sys/stat.h>
#include <io.h>
#include <fcntl.h>

/*
 * Declare dynamic loading extension macro.
 */

#define TCL_SHLIB_EXT ".dll"

/*
 * Declare directory manipulation routines.
 */

#ifdef HAS_DIRENT
#	include <dirent.h>
#	include <dir.h>
#else /* HAS_DIRENT */

#include <direct.h>
#define MAXNAMLEN 255

struct dirent {
    long d_ino;			/* Inode number of entry */
    short d_reclen;		/* Length of this record */
    short d_namlen;		/* Length of string in d_name */
    char d_name[MAXNAMLEN + 1];
				/* Name must be no longer than this */
};

typedef struct _dirdesc DIR;

EXTERN void			closedir _ANSI_ARGS_((DIR *dirp));
EXTERN DIR *			opendir _ANSI_ARGS_((char *name));
EXTERN struct dirent *		readdir _ANSI_ARGS_((DIR *dirp));
#endif /* HAS_DIRENT */

/*
 * Supply definitions for macros to query wait status, if not already
 * defined in header files above.
 */

#if TCL_UNION_WAIT
#   define WAIT_STATUS_TYPE union wait
#else
#   define WAIT_STATUS_TYPE int
#endif

#ifndef WIFEXITED
#   define WIFEXITED(stat)  (((*((int *) &(stat))) & 0xff) == 0)
#endif

#ifndef WEXITSTATUS
#   define WEXITSTATUS(stat) (((*((int *) &(stat))) >> 8) & 0xff)
#endif

#ifndef WIFSIGNALED
#   define WIFSIGNALED(stat) (((*((int *) &(stat)))) && ((*((int *) &(stat))) == ((*((int *) &(stat))) & 0x00ff)))
#endif

#ifndef WTERMSIG
#   define WTERMSIG(stat)    ((*((int *) &(stat))) & 0x7f)
#endif

#ifndef WIFSTOPPED
#   define WIFSTOPPED(stat)  (((*((int *) &(stat))) & 0xff) == 0177)
#endif

#ifndef WSTOPSIG
#   define WSTOPSIG(stat)    (((*((int *) &(stat))) >> 8) & 0xff)
#endif

/*
 * Define constants for waitpid() system call if they aren't defined
 * by a system header file.
 */

#ifndef WNOHANG
#   define WNOHANG 1
#endif
#ifndef WUNTRACED
#   define WUNTRACED 2
#endif

/*
 * Define MAXPATHLEN in terms of MAXPATH if available
 */

#ifndef MAXPATH
#define MAXPATH MAX_PATH
#endif /* MAXPATH */

#ifndef MAXPATHLEN
#define MAXPATHLEN MAXPATH
#endif /* MAXPATHLEN */

#ifndef F_OK
#    define F_OK 00
#endif
#ifndef X_OK
#    define X_OK 01
#endif
#ifndef W_OK
#    define W_OK 02
#endif
#ifndef R_OK
#    define R_OK 04
#endif

/*
 * On systems without symbolic links (i.e. S_IFLNK isn't defined)
 * define "lstat" to use "stat" instead.
 */

#ifndef S_IFLNK
#   define lstat stat
#endif

/*
 * Define macros to query file type bits, if they're not already
 * defined.
 */

#ifndef S_ISREG
#   ifdef S_IFREG
#       define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#   else
#       define S_ISREG(m) 0
#   endif
# endif
#ifndef S_ISDIR
#   ifdef S_IFDIR
#       define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#   else
#       define S_ISDIR(m) 0
#   endif
# endif
#ifndef S_ISCHR
#   ifdef S_IFCHR
#       define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#   else
#       define S_ISCHR(m) 0
#   endif
# endif
#ifndef S_ISBLK
#   ifdef S_IFBLK
#       define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#   else
#       define S_ISBLK(m) 0
#   endif
# endif
#ifndef S_ISFIFO
#   ifdef S_IFIFO
#       define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#   else
#       define S_ISFIFO(m) 0
#   endif
# endif
#ifndef S_ISLNK
#   ifdef S_IFLNK
#       define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#   else
#       define S_ISLNK(m) 0
#   endif
# endif
#ifndef S_ISSOCK
#   ifdef S_IFSOCK
#       define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
#   else
#       define S_ISSOCK(m) 0
#   endif
# endif

/*
 * Define pid_t and uid_t if they're not already defined.
 */

#if ! TCL_PID_T
#   define pid_t int
#endif
#if ! TCL_UID_T
#   define uid_t int
#endif

/*
 * Substitute Tcl's own versions for several system calls.
 */

#define open(a,b,c)	TclWinOpen(a,b,c)
#define read(a,b,c)	TclWinRead(a,b,c)
#define waitpid(a,b,c)	TclWinWaitpid(a,b,c)
#define getcwd(a,b)	TclWinGetCwd(a,b)

/*
 * Declarations for Windows specific functions.
 */

EXTERN void		TclWinConvertError _ANSI_ARGS_((DWORD errCode));
EXTERN void		TclWinConvertToStdPath _ANSI_ARGS_((char *path));
EXTERN void		TclWinConvertToNativePath _ANSI_ARGS_((char *path));
EXTERN char *		TclWinGetCwd _ANSI_ARGS_((char *buffer, int maxlen));
EXTERN HINSTANCE	TclWinLoadLibrary _ANSI_ARGS_((char *name));
EXTERN int		TclWinOpen _ANSI_ARGS_((char *path, int oflag,
			    int mode));
EXTERN int		TclWinRead _ANSI_ARGS_((int fd, const void *buf,
			    int num));
EXTERN int		TclWinWaitpid _ANSI_ARGS_((pid_t pid, int *statPtr,
			    int options));

/*
 * Provide an implementation of TclSetSystemEnv in terms of the equivalent
 * Win32 call.
 */

#define TclSetSystemEnv(a,b) SetEnvironmentVariable(a,b)

/*
 * Define timezone for gettimeofday.
 */

struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};

extern int gettimeofday(struct timeval *, struct timezone *);

/*
 * Visual C++ has some odd names for common functions, so we need to
 * define a few macros to handle them.  Also, it defines EDEADLOCK and
 * EDEADLK as the same value, which confuses Tcl_ErrnoId().
 */

#ifdef VISUAL_CPLUSPLUS
#    define environ _environ
#    define hypot _hypot
#    undef EDEADLOCK
#endif /* VISUAL_CPLUSPLUS */

/*
 * The following macro defines the character that marks the end of
 * a line.
 */

#define NEWLINE_CHAR '\n'

#endif /* _TCLWINPORT */
