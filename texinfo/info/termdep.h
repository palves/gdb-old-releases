/* termdep.h -- System things that terminal.c depends on. */

#if defined (HAVE_SYS_FCNTL_H)
#  include <sys/fcntl.h>
#else
#  include <fcntl.h>
#endif /* !HAVE_SYS_FCNTL_H */

#if defined (HAVE_TERMIO_H)
#  include <termio.h>
#  include <string.h>
#  if defined (HAVE_SYS_PTEM_H)
#      include <sys/stream.h>
#      include <sys/ptem.h>
#      undef TIOCGETC
#    if defined (M_XENIX)
#      define tchars tc
#    endif /* M_XENIX */
#  endif /* HAVE_SYS_PTEM_H */
#    define bcopy(source, dest, count) memcpy((dest), (source), (count))
#  if !defined (_AIX)
#    define index(s, c) strchr((s), (c))
#    define rindex(s, c) strrchr((s), (c))
#  endif /* !_AIX */
#else /* !HAVE_TERMIO_H */
#  include <sys/file.h>
#  include <sgtty.h>
#  include <strings.h>
#endif /* !HAVE_TERMIO_H */

#if defined (HAVE_SYS_TTOLD_H)
#  include <sys/ttold.h>
#endif /* HAVE_SYS_TTOLD_H */
