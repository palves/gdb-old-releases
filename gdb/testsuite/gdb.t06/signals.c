/* Test GDB dealing with stuff like stepping into sigtramp.  */
#include <signal.h>

static int count = 0;

static void
handler (sig)
     int sig;
{
  signal (sig, handler);
  ++count;
}

static void
func1 ()
{
  ++count;
}

static void
func2 ()
{
  ++count;
}

int
main ()
{
  signal (SIGALRM, handler);
  signal (SIGUSR1, handler);
  alarm (1);
  ++count;
  alarm (1);
  ++count;
  func1 ();
  alarm (1);
  func2 ();
  return count;
}
