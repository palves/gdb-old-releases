/* signals.h -- Header to include system dependent signal definitions. */

#if !defined (_SIGNALS_H_)
#define _SIGNALS_H_

#include <signal.h>

#if !defined (HAVE_SIGPROCMASK) && !defined (sigmask)
#  define sigmask(x) (1 << ((x)-1))
#endif /* !defined (HAVE_SIGPROCMASK) && ! defined (sigmask) */

#if !defined (HAVE_SIGPROCMASK)
#  if !defined (SIG_BLOCK)
#    define SIG_UNBLOCK 1
#    define SIG_BLOCK   2
#    define SIG_SETMASK 3
#  endif /* SIG_BLOCK */

/* Type of a signal set. */
#  define sigset_t int

/* Make SET have no signals in it. */
#  define sigemptyset(set) (*(set) = (sigset_t)0x0)

/* Make SET have the full range of signal specifications possible. */
#  define sigfillset(set) (*(set) = (sigset_t)0xffffffffff)

/* Add SIG to the contents of SET. */
#  define sigaddset(set, sig) *(set) |= sigmask (sig)

/* Delete SIG from the contents of SET. */
#  define sigdelset(set, sig) *(set) &= ~(sigmask (sig))

/* Tell if SET contains SIG. */
#  define sigismember(set, sig) (*(set) & (sigmask (sig)))

/* Suspend the process until the reception of one of the signals
   not present in SET. */
#  define sigsuspend(set) sigpause (*(set))
#endif /* ! defined (HAVE_SIGPROCMASK) */

#if defined (HAVE_SIGPROCMASK) || defined (HAVE_SIGSETMASK)
/* These definitions are used both in POSIX and non-POSIX implementations. */

#define BLOCK_SIGNAL(sig) \
  do { \
    sigset_t nvar, ovar; \
    sigemptyset (&nvar); \
    sigemptyset (&ovar); \
    sigaddset (&nvar, sig); \
    sigprocmask (SIG_BLOCK, &nvar, &ovar); \
  } while (0)

#define UNBLOCK_SIGNAL(sig) \
  do { \
    sigset_t nvar, ovar; \
    sigemptyset (&ovar); \
    sigemptyset (&nvar); \
    sigaddset (&nvar, sig); \
    sigprocmask (SIG_UNBLOCK, &nvar, &ovar); \
  } while (0)

#else /* ! defined (HAVE_SIGPROC_MASK) && ! defined (HAVE_SIGSETMASK) */

#define BLOCK_SIGNAL(sig)
#define UNBLOCK_SIGNAL(sig)

#endif /* ! defined (HAVE_SIGPROC_MASK) && ! defined (HAVE_SIGSETMASK) */

#endif /* !_SIGNALS_H_ */
