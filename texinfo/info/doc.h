/* doc.h -- Structure associating function pointers with documentation. */

#if !defined (_DOC_H_)
#define _DOC_H_

#if !defined (NULL)
#  define NULL 0x0
#endif /* !NULL */

#if !defined (__FUNCTION_DEF)
#  define __FUNCTION_DEF
typedef int Function ();
typedef void VFunction ();
#endif /* _FUNCTION_DEF */

typedef struct {
  VFunction *func;
#if defined (NAMED_FUNCTIONS)
  char *func_name;
#endif /* NAMED_FUNCTIONS */
  char *doc;
} FUNCTION_DOC;

extern FUNCTION_DOC function_doc_array[];

extern char *function_documentation ();
extern char *key_documentation ();
extern char *pretty_keyname ();
extern char *replace_in_documentation ();
extern void info_document_key ();
extern void dump_map_to_message_buffer ();

#if defined (NAMED_FUNCTIONS)
extern char *function_name ();
extern VFunction *named_function ();
#endif /* NAMED_FUNCTIONS */
#endif /* !_DOC_H_ */
