/* display.h -- How the display in Info is done. */

#if !defined (_DISPLAY_H_)
#define _DISPLAY_H_

#include "info-utils.h"
#include "terminal.h"

typedef struct {
  char *text;			/* Text of the line as it appears. */
  int textlen;			/* Printable Length of TEXT. */
  int inverse;			/* Non-zero means this line is inverse. */
} DISPLAY_LINE;

/* An array of display lines which tell us what is currently visible on
   the display.  */
extern DISPLAY_LINE **the_display;

/* Non-zero means do no output. */
extern int display_inhibited;

/* Non-zero if we didn't completely redisplay a window. */
extern int display_was_interrupted_p;

/* Initialize THE_DISPLAY to WIDTH and HEIGHT, with nothing in it. */
extern void display_initialize_display ();

/* Clear all of the lines in DISPLAY making the screen blank. */
extern void display_clear_display ();

/* Update the windows pointed to by WINDOWS in THE_DISPLAY.  This actually
   writes the text on the screen. */
extern void display_update_display ();

/* Display WIN on THE_DISPLAY.  Unlike display_update_display (), this
   function only does one window. */
extern void display_update_one_window ();

/* Move the screen cursor to directly over the current character in WINDOW. */
extern void display_cursor_at_point ();

/* Scroll the region of the_display starting at START, ending at END, and
   moving the lines AMOUNT lines.  If AMOUNT is less than zero, the lines
   are moved up in the screen, otherwise down.  Actually, it is possible
   for no scrolling to take place in the case that the terminal doesn't
   support it.  This doesn't matter to us. */
extern void display_scroll_display ();

#endif /* !_DISPLAY_H_ */
