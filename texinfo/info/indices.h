/* indices.h -- Functions defined in indices.c. */

#if !defined (_INDICES_H_)
#define _INDICES_H_

/* User-visible variable controls the output of info-index-next. */
extern int show_index_match;

extern REFERENCE **info_indices_of_window (), **info_indices_of_file_buffer ();

/* User visible functions declared in indices.c. */
extern void info_index_search (), info_next_index_match ();

#endif /* !_INDICES_H_ */
