/* echo_area.h -- Functions used in reading information from the echo area. */

#if !defined (_ECHO_AREA_H_)
#define _ECHO_AREA_H_

#define EA_MAX_INPUT 256

extern int echo_area_is_active, info_aborted_echo_area;

/* Non-zero means that the last command executed while reading input
   killed some text. */
extern int echo_area_last_command_was_kill;

extern void inform_in_echo_area (), echo_area_inform_of_deleted_window ();
extern void echo_area_prep_read ();
extern VFunction *ea_last_executed_command;

/* Read a line of text in the echo area.  Return a malloc ()'ed string,
   or NULL if the user aborted out of this read.  WINDOW is the currently
   active window, so that we can restore it when we need to.  PROMPT, if
   non-null, is a prompt to print before reading the line. */
extern char *info_read_in_echo_area ();

/* Read a line in the echo area with completion over COMPLETIONS.
   Takes arguments of WINDOW, PROMPT, and COMPLETIONS, a REFERENCE **. */
char *info_read_completing_in_echo_area ();

/* Read a line in the echo area allowing completion over COMPLETIONS, but
   not requiring it.  Takes arguments of WINDOW, PROMPT, and COMPLETIONS,
   a REFERENCE **. */
extern char *info_read_maybe_completing ();

extern void ea_insert (), ea_quoted_insert ();
extern void ea_beg_of_line (), ea_backward (), ea_delete (), ea_end_of_line ();
extern void ea_forward (), ea_abort (), ea_rubout (), ea_complete ();
extern void ea_newline (), ea_kill_line (), ea_transpose_chars ();
extern void ea_yank (), ea_tab_insert (), ea_possible_completions ();
extern void ea_backward_word (), ea_kill_word (), ea_forward_word ();
extern void ea_yank_pop (), ea_backward_kill_word ();
extern void ea_scroll_completions_window ();

#endif /* _ECHO_AREA_H_ */
