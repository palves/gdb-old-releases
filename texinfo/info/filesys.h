/* filesys.h -- External declarations of functions and vars in filesys.c. */

#if !defined (_FILESYS_H_)
#define _FILESYS_H_

/* The path on which we look for info files.  You can initialize this
   from the environment variable INFOPATH if there is one, or you can
   call info_add_path () to add paths to the beginning or end of it. */
extern char *infopath;

/* Make INFOPATH have absolutely nothing in it. */
extern void zap_infopath ();

/* Add PATH to the list of paths found in INFOPATH.  2nd argument says
   whether to put PATH at the front or end of INFOPATH. */
extern void info_add_path ();

/* Defines that are passed along with the pathname to info_add_path (). */
#define INFOPATH_PREPEND 0
#define INFOPATH_APPEND  1

/* Expand the filename in PARTIAL to make a real name for this operating
   system.  This looks in INFO_PATHS in order to find the correct file.
   If it can't find the file, it returns NULL. */
extern char *info_find_fullpath ();

/* Read the contents of PATHNAME, returning a buffer with the contents of
   that file in it, and returning the size of that buffer in FILESIZE.
   FINFO is a stat struct which has already been filled in by the caller.
   If the file cannot be read, return a NULL pointer. */
extern char *filesys_read_info_file ();
extern char *filesys_read_compressed ();

/* Return the command string that would be used to decompress FILENAME. */
extern char *filesys_decompressor_for_file ();
extern int compressed_filename_p ();

/* A function which returns a pointer to a static buffer containing
   an error message for FILENAME and ERROR_NUM. */
extern char *filesys_error_string ();

/* The number of the most recent file system error. */
extern int filesys_error_number;

/* Given a string containing units of information separated by colons,
   return the next one pointed to by IDX, or NULL if there are no more.
   Advance IDX to the character after the colon. */
extern char *extract_colon_unit ();

/* The default value of INFOPATH. */
#if !defined (DEFAULT_INFOPATH)
#  define DEFAULT_INFOPATH "/usr/gnu/info:/usr/gnu/lib/info:/usr/gnu/lib/emacs/info:/usr/local/gnu/info:/usr/local/gnu/lib/info:/usr/local/gnu/lib/emacs/info:/usr/local/lib/info:/usr/local/lib/emacs/info:/usr/local/emacs/info:."
#endif /* !DEFAULT_INFOPATH */

#if !defined (S_ISREG) && defined (S_IFREG)
#  define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif /* !S_ISREG && S_IFREG */

#if !defined (S_ISDIR) && defined (S_IFDIR)
#  define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif /* !S_ISDIR && S_IFDIR */

#endif /* !_FILESYS_H_ */
