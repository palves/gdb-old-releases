#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#define DONTDECLARE_MALLOC
#include "hosts/i386v.h"

/* Might be required by trad-core.c (not sure if it is).  */
#include <sys/file.h>

/* Linux writes the task structure at the end of the core file.  Currently it
   is 2912 bytes.  It is possible that this should be a pickier check, but
   we should probably not be too picky (the size of the task structure might
   vary, and if it's not the length we expect it to be, it doesn't affect
   our ability to process the core file).  So allow 0-4096 extra bytes at
   the end.  */

#define TRAD_CORE_EXTRA_SIZE_ALLOWED 4096

/* Used for core file stuff only.  */
#if 0 /* Turning this on requires that you have support code which, as
	 I write this, is alpha-test code no more than a few days old.
	 This is stupid.  Because of the way BFD is set up, it means
	 you can't build a Linux-hosted ELF-targeted toolchain.  Leave
	 it disabled until BFD is fixed or the newest Linux ELF code
	 is widely distributed (including a few CD-ROM releases).
	 [raeburn:95/04/24] */
#define HAVE_PROCFS
#endif

