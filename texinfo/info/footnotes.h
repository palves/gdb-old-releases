/* footnotes.h -- Some functions for manipulating footnotes. */

#if !defined (_FOOTNOTES_H_)
#define _FOOTNOTES_H_

/* Magic string which indicates following text is footnotes. */
#define FOOTNOTE_LABEL "---------- Footnotes ----------"

#define FN_FOUND   0
#define FN_UNFOUND 1
#define FN_UNABLE  2


/* Create or delete the footnotes window depending on whether footnotes
   exist in WINDOW's node or not.  Returns FN_FOUND if footnotes were found
   and displayed.  Returns FN_UNFOUND if there were no footnotes found
   in WINDOW's node.  Returns FN_UNABLE if there were footnotes, but the
   window to show them couldn't be made. */
extern int info_get_or_remove_footnotes ();

/* Non-zero means attempt to show footnotes when displaying a new window. */
extern int auto_footnotes_p;

#endif /* !_FOOTNOTES_H_ */

