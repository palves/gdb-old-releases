/* dribble.h -- Functions and vars declared in dribble.c. */

#if !defined (_DRIBBLE_H_)
#define _DRIBBLE_H_

/* When non-zero, it is a stream to write all input characters to for the
   duration of this info session. */
extern FILE *info_dribble_file;

/* Open a dribble file named NAME, perhaps closing an already open one.
   This sets the global variable INFO_DRIBBLE_FILE to the open stream. */
extern void open_dribble_file ();

/* If there is a dribble file already open, close it. */
extern void close_dribble_file ();

/* Write some output to our existing dribble file. */
extern void dribble ();

#endif /* !_DRIBBLE_H_ */
