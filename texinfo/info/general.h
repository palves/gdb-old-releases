/* general.h -- Some generally useful defines. */

#if !defined (_GENERAL_H_)
#define _GENERAL_H_

extern void *xmalloc (), *xrealloc ();

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if defined (HAVE_STRING_H)
#  include <string.h>
#else
#  include <strings.h>
#endif /* !HAVE_STRING_H */

#if !defined (savestring)
#  define savestring(x) (char *)strcpy ((char *)xmalloc (1 + strlen (x)), x)
#endif /* !savestring */

#define info_toupper(x) (islower (x) ? toupper (x) : x)
#define info_tolower(x) (isupper (x) ? tolower (x) : x)

#if !defined (whitespace)
#  define whitespace(c) ((c == ' ') || (c == '\t'))
#endif /* !whitespace */

#if !defined (whitespace_or_newline)
#  define whitespace_or_newline(c) (whitespace (c) || (c == '\n'))
#endif /* !whitespace_or_newline */

#if !defined (__FUNCTION_DEF)
#  define __FUNCTION_DEF
typedef int Function ();
typedef void VFunction ();
#endif /* _FUNCTION_DEF */

/* Add POINTER to the list of pointers found in ARRAY.  SLOTS is the number
   of slots that have already been allocated.  INDEX is the index into the
   array where POINTER should be added.  GROW is the number of slots to grow
   ARRAY by, in the case that it needs growing.  TYPE is a cast of the type
   of object stored in ARRAY (e.g., NODE_ENTRY *. */
#define add_pointer_to_array(pointer, idx, array, slots, grow, type) \
  do { \
    if (idx + 2 >= slots) \
      array = (type *)(xrealloc (array, (slots += grow) * sizeof (type))); \
    array[idx++] = (type)pointer; \
    array[idx] = (type)NULL; \
  } while (0)

#define maybe_free(x) do { if (x) free (x); } while (0)

#if !defined (HAVE_BZERO)
#  define zero_mem(mem, length) memset (mem, 0, length)
#else
#  define zero_mem(mem, length) bzero (mem, length)
#endif /* !BZERO_MISSING */

#endif /* !_GENERAL_H_ */
