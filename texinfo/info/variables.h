/* variables.h -- Description of user visible variables in Info. */

#if !defined (_VARIABLES_H_)
#define _VARIABLES_H_

/* A variable (in the Info sense) is an integer value with a user-visible
   name.  You may supply an array of strings to complete over when the
   variable is set; in that case, the variable is set to the index of the
   string that the user chose.  If you supply a null list, the user can
   set the variable to a numeric value. */

/* Structure describing a user visible variable. */
typedef struct {
  char *name;			/* Polite name. */
  char *doc;			/* Documentation string. */
  int *value;			/* Address of value. */
  char **choices;		/* Array of strings or NULL if numeric only. */
} VARIABLE_ALIST;

/* Read the name of an Info variable in the echo area and return the
   address of a VARIABLE_ALIST member.  A return value of NULL indicates
   that no variable could be read. */
extern VARIABLE_ALIST *read_variable_name ();

/* Make an array of REFERENCE which actually contains the names of the
   variables available in Info. */
extern REFERENCE **make_variable_completions_array ();

/* Set the value of an info variable. */
extern void set_variable ();

/* The list of user-visible variables. */
extern int auto_footnotes_p;
extern int auto_tiling_p;
extern int terminal_use_visible_bell_p;
extern int info_error_rings_bell_p;
extern int gc_compressed_files;
extern int show_index_match;
extern int info_scroll_behaviour;
extern int window_scroll_step;
extern int ISO_Latin_p;

#endif /* _VARIABLES_H_ */
