/* search.h -- Structure used to search large bodies of text, with bounds. */

/* The search functions take two arguments:

     1) a string to search for, and

     2) a pointer to a SEARCH_BINDING which contains the buffer, start,
        and end of the search.

   They return a long, which is the offset from the start of the buffer
   at which the match was found.  An offset of -1 indicates failure. */

#if !defined (_SEARCH_H_)
#define _SEARCH_H_

typedef struct {
  char *buffer;			/* The buffer of text to search. */
  long start;			/* Offset of the start of the search. */
  long end;			/* Offset of the end of the searh. */
  int flags;			/* Flags controlling the type of search. */
} SEARCH_BINDING;

#define S_FoldCase	0x01	/* Set means fold case in searches. */
#define S_SkipDest	0x02	/* Set means return pointing after the dest. */

SEARCH_BINDING *make_binding (), *copy_binding ();
extern long search_forward (), search_backward (), search ();
extern int looking_at ();

/* Note that STRING_IN_LINE () always returns the offset of the 1st character
   after the string. */
extern int string_in_line ();

/* Unix doesn't have stricmp () functions. */
/* strcmp (), but caseless. */
extern int stricmp ();

/* Compare at most COUNT characters from STRING1 to STRING2.  Case
   doesn't matter. */
extern int strnicmp ();

/* Function names that start with "skip" are passed a string, and return
   an offset from the start of that string.  Function names that start
   with "find" are passed a SEARCH_BINDING, and return an absolute position
   marker of the item being searched for.  "Find" functions return a value
   of -1 if the item being looked for couldn't be found. */
extern int skip_whitespace (), skip_non_whitespace ();
extern int skip_whitespace_and_newlines (), skip_line ();
extern int skip_node_characters (), skip_node_separator ();
#define DONT_SKIP_NEWLINES 0
#define SKIP_NEWLINES 1

extern long find_node_separator (), find_tags_table ();
extern long find_node_in_binding ();

#endif /* !_SEARCH_H_ */

