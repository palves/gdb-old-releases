/* Memory-access and commands for remote NINDY process, for GDB.
   Copyright (C)  1990 Free Software Foundation, Inc.

   MODIFIED BY CHRIS BENENATI, FOR INTEL CORPORATION, 12/88

GDB is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY.  No author or distributor accepts responsibility to anyone
for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.
Refer to the GDB General Public License for full details.

Everyone is granted permission to copy, modify and redistribute GDB,
but only under the conditions described in the GDB General Public
License.  A copy of this license is supposed to have been given to you
along with GDB so you can know your rights and responsibilities.  It
should be in a file named COPYING.  Among other things, the copyright
notice and this notice must be preserved on all copies.

In other words, go ahead and share GDB, but don't try to stop
anyone else from sharing it farther.  Help stamp out software hoarding!
*/

/*
Except for the data cache routines, this file bears little resemblence
to remote.c.  A new (although similar) protocol has been specified, and
portions of the code are entirely dependent on having an i80960 with a
NINDY ROM monitor at the other end of the line.
*/

/*****************************************************************************
 *
 * REMOTE COMMUNICATION PROTOCOL BETWEEN GDB960 AND THE NINDY ROM MONITOR.
 *
 *
 * MODES OF OPERATION
 * ----- -- ---------
 *	
 * As far as NINDY is concerned, GDB is always in one of two modes: command
 * mode or passthrough mode.
 *
 * In command mode (the default) pre-defined packets containing requests
 * are sent by GDB to NINDY.  NINDY never talks except in reponse to a request.
 *
 * Once the the user program is started, GDB enters passthrough mode, to give
 * the user program access to the terminal.  GDB remains in this mode until
 * NINDY indicates that the program has stopped.
 *
 *
 * PASSTHROUGH MODE
 * ----------- ----
 *
 * GDB writes all input received from the keyboard directly to NINDY, and writes
 * all characters received from NINDY directly to the monitor.
 *
 * Keyboard input is neither buffered nor echoed to the monitor.
 *
 * GDB remains in passthrough mode until NINDY sends a single ^P character,
 * to indicate that the user process has stopped.
 *
 * Note:
 *	GDB assumes NINDY performs a 'flushreg' when the user program stops.
 *
 *
 * COMMAND MODE
 * ------- ----
 *
 * All info (except for message ack and nak) is transferred between gdb
 * and the remote processor in messages of the following format:
 *
 *		<info>#<checksum>
 *
 * where 
 *	#	is a literal character
 *
 *	<info>	ASCII information;  all numeric information is in the
 *		form of hex digits ('0'-'9' and lowercase 'a'-'f').
 *
 *	<checksum>
 *		is a pair of ASCII hex digits representing an 8-bit
 *		checksum formed by adding together each of the
 *		characters in <info>.
 *
 * The receiver of a message always sends a single character to the sender
 * to indicate that the checksum was good ('+') or bad ('-');  the sender
 * re-transmits the entire message over until a '+' is received.
 *
 * In response to a command NINDY always sends back either data or
 * a result code of the form "Xnn", where "nn" are hex digits and "X00"
 * means no errors.  (Exceptions: the "s" and "c" commands don't respond.)
 *
 * SEE THE HEADER OF THE FILE "gdb.c" IN THE NINDY MONITOR SOURCE CODE FOR A
 * FULL DESCRIPTION OF LEGAL COMMANDS.
 *
 * SEE THE FILE "stop.h" IN THE NINDY MONITOR SOURCE CODE FOR A LIST
 * OF STOP CODES.
 *
 ******************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <setjmp.h>

#include "defs.h"
#include "param-no-tm.h"
#include "tm-i960.h"
#include "frame.h"
#include "inferior.h"
#include "target.h"
#include "gdbcore.h"
#include "command.h"
#include "bfd.h"

#include "wait.h"
#include <sys/ioctl.h>
#include <sys/file.h>
#include <ctype.h>
#include "nindy-share/ttycntl.h"
#include "nindy-share/demux.h"
#include "nindy-share/env.h"
#include "nindy-share/stop.h"

extern int unlink();
extern char *getenv();
extern char *mktemp();

extern char *coffstrip();
extern void add_file_addr_command ();
extern value call_function_by_hand ();
extern void generic_mourn_inferior ();

extern struct target_ops nindy_ops;
extern jmp_buf to_top_level;
extern FILE *instream;

extern char ninStopWhy ();

int nindy_initial_brk;	/* nonzero if want to send an initial BREAK to nindy */
int nindy_old_protocol;	/* nonzero if want to use old protocol */
char *nindy_ttyname;	/* name of tty to talk to nindy on, or null */

#define DLE	'\020'	/* Character NINDY sends to indicate user program has
			 * halted.  */
#define TRUE	1
#define FALSE	0

int nindy_fd = 0;	/* Descriptor for I/O to NINDY  */
static int have_regs = 0;	/* 1 iff regs read since i960 last halted */
static int regs_changed = 0;	/* 1 iff regs were modified since last read */

extern char *exists();
static void dcache_flush (), dcache_poke (), dcache_init();
static int dcache_fetch ();

/* FIXME, we can probably use the normal terminal_inferior stuff here.
   We have to do terminal_inferior and then set up the passthrough
   settings initially.  Thereafter, terminal_ours and terminal_inferior
   will automatically swap the settings around for us.  */

/* Restore TTY to normal operation */

static TTY_STRUCT orig_tty;	/* TTY attributes before entering passthrough */

static void
restore_tty()
{
	ioctl( 0, TIOCSETN, &orig_tty );
}


/* Recover from ^Z or ^C while remote process is running */

static void (*old_ctrlc)();    /* Signal handlers before entering passthrough */

#ifdef SIGTSTP
static void (*old_ctrlz)();
#endif

static
#ifdef USG
void
#endif
cleanup()
{
	restore_tty();
	signal(SIGINT, old_ctrlc);
#ifdef SIGTSTP
	signal(SIGTSTP, old_ctrlz);
#endif
	error("\n\nYou may need to reset the 80960 and/or reload your program.\n");
}

/* Open a connection to a remote debugger.   
   FIXME, there should be a way to specify the various options that are
   now specified with gdb command-line options.  (baud_rate, old_protocol,
   and initial_brk)  */
char *savename;

void
nindy_open (name, from_tty)
    char *name;		/* "/dev/ttyXX", "ttyXX", or "XX": tty to be opened */
    int from_tty;
{

  if (!name)
    error_no_arg ("serial port device name");

  if (savename)
    free (savename);
  savename = 0;

	have_regs = regs_changed = 0;
	dcache_init();

	if ( nindy_fd ){
		close( nindy_fd );
		nindy_fd = 0;
	}

	/* Allow user to interrupt the following -- we could hang if
	 * there's no NINDY at the other end of the remote tty.
	 */
	immediate_quit++;
	nindy_fd = ninConnect( name, baud_rate? baud_rate: "9600",
			nindy_initial_brk, !from_tty, nindy_old_protocol );
	immediate_quit--;

	if ( nindy_fd < 0 ){
		nindy_fd = 0;
		error( "Can't open tty '%s'", name );
	}

        savename = savestring (name, strlen (name));
	push_target (&nindy_ops);
	target_fetch_registers(-1);
}

static void
nindy_detach (name, from_tty)
     char *name;
     int from_tty;
{
  dont_repeat ();
  if (name)
    error ("Too many arguments");
  if (nindy_fd)
    close (nindy_fd);
  nindy_fd = 0;
  pop_target ();
}

static void
nindy_files_info ()
{
  printf("\tAttached to %s at %s bps%s%s.\n", savename,
	 baud_rate? baud_rate: "9600",
	 nindy_old_protocol? " in old protocol": "",
         nindy_initial_brk? " with initial break": "");
}

/******************************************************************************
 * remote_load:
 *	Download an object file to the remote system by invoking the "comm960"
 *	utility.  We look for "comm960" in $G960BIN, $G960BASE/bin, and
 *	DEFAULT_BASE/bin/HOST/bin where
 *		DEFAULT_BASE is defined in env.h, and
 *		HOST must be defined on the compiler invocation line.
 ******************************************************************************/

static void
nindy_load( filename, from_tty )
    char *filename;
    int from_tty;
{
  char *tmpfile;
  struct cleanup *old_chain;
  char *scratch_pathname;
  int scratch_chan;

  if (!filename)
    filename = get_exec_file (1);

  filename = tilde_expand (filename);
  make_cleanup (free, filename);

  scratch_chan = openp (getenv ("PATH"), 1, filename, O_RDONLY, 0,
			&scratch_pathname);
  if (scratch_chan < 0)
    perror_with_name (filename);
  close (scratch_chan);		/* Slightly wasteful FIXME */

  have_regs = regs_changed = 0;
  mark_breakpoints_out();
  inferior_pid = 0;
  dcache_flush();

  tmpfile = coffstrip(scratch_pathname);
  if ( tmpfile ){
	  old_chain = make_cleanup(unlink,tmpfile);
	  immediate_quit++;
	  ninDownload( tmpfile, !from_tty );
/* FIXME, don't we want this merged in here? */
	  immediate_quit--;
	  do_cleanups (old_chain);
  }
}



/* Return the number of characters in the buffer before the first DLE character.
 */

static
int
non_dle( buf, n )
    char *buf;		/* Character buffer; NOT '\0'-terminated */
    int n;		/* Number of characters in buffer */
{
	int i;

	for ( i = 0; i < n; i++ ){
		if ( buf[i] == DLE ){
			break;
		}
	}
	return i;
}

/* Tell the remote machine to resume.  */

void
nindy_resume (step, signal)
     int step, signal;
{
	dcache_flush();
	if ( regs_changed ){
		immediate_quit++;
		ninRegsPut( registers );
		immediate_quit--;
		regs_changed = 0;
	}
	have_regs = 0;
	ninGo( step );
}

/* Wait until the remote machine stops. While waiting, operate in passthrough
 * mode; i.e., pass everything NINDY sends to stdout, and everything from
 * stdin to NINDY.
 *
 * Return to caller, storing status in 'status' just as `wait' would.
 */

void
nindy_wait( status )
    WAITTYPE *status;
{
	DEMUX_DECL;	/* OS-dependent data needed by DEMUX... macros */
	char buf[500];	/* FIXME, what is "500" here? */
	int i, n;
	unsigned char stop_exit;
	unsigned char stop_code;
	TTY_STRUCT tty;
	long ip_value, fp_value, sp_value;	/* Reg values from stop */


	WSETEXIT( (*status), 0 );

	/* OPERATE IN PASSTHROUGH MODE UNTIL NINDY SENDS A DLE CHARACTER */

	/* Save current tty attributes, set up signals to restore them.
	 */
	ioctl( 0, TIOCGETP, &orig_tty );
	old_ctrlc = signal( SIGINT, cleanup );
#ifdef SIGTSTP
	old_ctrlz = signal( SIGTSTP, cleanup );
#endif

	/* Pass input from keyboard to NINDY as it arrives.
	 * NINDY will interpret <CR> and perform echo.
	 */
	tty = orig_tty;
	TTY_NINDYTERM( tty );
	ioctl( 0, TIOCSETN, &tty );

	while ( 1 ){
		/* Go to sleep until there's something for us on either
		 * the remote port or stdin.
		 */

		DEMUX_WAIT( nindy_fd );

		/* Pass input through to correct place */

		n = DEMUX_READ( 0, buf, sizeof(buf) );
		if ( n ){				/* Input on stdin */
			write( nindy_fd, buf, n );
		}

		n = DEMUX_READ( nindy_fd, buf, sizeof(buf) );
		if ( n ){				/* Input on remote */
			/* Write out any characters in buffer preceding DLE */
			i = non_dle( buf, n );
			if ( i > 0 ){
				write( 1, buf, i );
			}

			if ( i != n ){
				/* There *was* a DLE in the buffer */
				stop_exit = ninStopWhy( &stop_code,
					&ip_value, &fp_value, &sp_value);
				if ( !stop_exit && (stop_code==STOP_SRQ) ){
					immediate_quit++;
					ninSrq();
					immediate_quit--;
				} else {
					/* Get out of loop */
					supply_register (IP_REGNUM, &ip_value);
					supply_register (FP_REGNUM, &fp_value);
					supply_register (SP_REGNUM, &sp_value);
					break;
				}
			}
		}
	}

	signal( SIGINT, old_ctrlc );
#ifdef SIGTSTP
	signal( SIGTSTP, old_ctrlz );
#endif
	restore_tty();

	if ( stop_exit ){			/* User program exited */
		WSETEXIT( (*status), stop_code );
	} else {				/* Fault or trace */
		switch (stop_code){
		case STOP_GDB_BPT:
		case TRACE_STEP:
			/* Make it look like a VAX trace trap */
			stop_code = SIGTRAP;
			break;
		default:
			/* The 80960 is not running Unix, and its
			 * faults/traces do not map nicely into Unix signals.
			 * Make sure they do not get confused with Unix signals
			 * by numbering them with values higher than the highest
			 * legal Unix signal.  code in i960_print_fault() will
			 * interpret the value for normal_stop.
			 */
			stop_code += NSIG;
			break;
		}
		WSETSTOP( (*status), stop_code );
	}
}


/* Print out text describing a "signal number" with which the i80960 halted.
 *
 * SEE THE FILE "fault.c" IN THE NINDY MONITOR SOURCE CODE FOR A LIST
 * OF STOP CODES.
 */
void
i960_print_fault( signal )
    int signal;		/* Signal number, as returned by remote_wait() */
{
	static char unknown[] = "Unknown fault or trace";
	static char *sigmsgs[] = {
		/* FAULTS */
		"parallel fault",	/* 0x00 */
		unknown,		/* 0x01 */
		"operation fault",	/* 0x02 */
		"arithmetic fault",	/* 0x03 */
		"floating point fault",	/* 0x04 */
		"constraint fault",	/* 0x05 */
		"virtual memory fault",	/* 0x06 */
		"protection fault",	/* 0x07 */
		"machine fault",	/* 0x08 */
		"structural fault",	/* 0x09 */
		"type fault",		/* 0x0a */
		"reserved (0xb) fault",	/* 0x0b */
		"process fault",	/* 0x0c */
		"descriptor fault",	/* 0x0d */
		"event fault",		/* 0x0e */
		"reserved (0xf) fault",	/* 0x0f */

		/* TRACES */
		"single-step trace",	/* 0x10 */
		"branch trace",		/* 0x11 */
		"call trace",		/* 0x12 */
		"return trace",		/* 0x13 */
		"pre-return trace",	/* 0x14 */
		"supervisor call trace",/* 0x15 */
		"breakpoint trace",	/* 0x16 */
	};
#	define NUMMSGS ((int)( sizeof(sigmsgs) / sizeof(sigmsgs[0]) ))


	if ( signal == SIGTRAP ){
		printf( "\nUnexpected breakpoint.\n" );
		printf( "You should probably re-load your program.\n" );

	} else {

		/* remote_wait() biases the 80960 "signal number" by adding
		 * NSIG to it, so it won't get confused with any of the Unix
		 * signals elsewhere in GDB.  We need to "unbias" it before
		 * using it.
		 */
		signal -= NSIG;

		printf( "\nProgram stopped for reason #%d: %s.\n", signal,
				(signal < NUMMSGS && signal >= 0)?
				sigmsgs[signal] : unknown );
		printf( "Proceed at your own risk.\n\n" );
	}
}


/* Read the remote registers into the block REGS.  */

static int
nindy_fetch_registers(regno)
     int regno;
{
  immediate_quit++;
  ninRegsGet( (unsigned char *)registers );
  immediate_quit--;
  registers_fetched ();
  return 0;
}

static void
nindy_prepare_to_store()
{
  nindy_fetch_registers(-1);
}

static int
nindy_store_registers(regno)
     int regno;
{
  immediate_quit++;
  ninRegsPut( registers );
  immediate_quit--;
  return 0;
}

/* Read a word from remote address ADDR and return it.
 * This goes through the data cache.
 */
int
nindy_fetch_word (addr)
     CORE_ADDR addr;
{
	return dcache_fetch (addr);
}

/* Write a word WORD into remote address ADDR.
   This goes through the data cache.  */

void
nindy_store_word (addr, word)
     CORE_ADDR addr;
     int word;
{
	dcache_poke (addr, word);
}

/* Copy LEN bytes to or from inferior's memory starting at MEMADDR
   to debugger memory starting at MYADDR.   Copy to inferior if
   WRITE is nonzero.  Returns the length copied.

   This is stolen almost directly from infptrace.c's child_xfer_memory,
   which also deals with a word-oriented memory interface.  Sometime,
   FIXME, rewrite this to not use the word-oriented routines.  */

int
nindy_xfer_inferior_memory(memaddr, myaddr, len, write)
     CORE_ADDR memaddr;
     char *myaddr;
     int len;
     int write;
{
  register int i;
  /* Round starting address down to longword boundary.  */
  register CORE_ADDR addr = memaddr & - sizeof (int);
  /* Round ending address up; get number of longwords that makes.  */
  register int count
    = (((memaddr + len) - addr) + sizeof (int) - 1) / sizeof (int);
  /* Allocate buffer of that many longwords.  */
  register int *buffer = (int *) alloca (count * sizeof (int));
  extern int errno;

  if (write)
    {
      /* Fill start and end extra bytes of buffer with existing memory data.  */

      if (addr != memaddr || len < (int)sizeof (int)) {
	/* Need part of initial word -- fetch it.  */
        buffer[0] = nindy_fetch_word (addr);
      }

      if (count > 1)		/* FIXME, avoid if even boundary */
	{
	  buffer[count - 1]
	    = nindy_fetch_word (addr + (count - 1) * sizeof (int));
	}

      /* Copy data to be written over corresponding part of buffer */

      bcopy (myaddr, (char *) buffer + (memaddr & (sizeof (int) - 1)), len);

      /* Write the entire buffer.  */

      for (i = 0; i < count; i++, addr += sizeof (int))
	{
	  errno = 0;
	  nindy_store_word (addr, buffer[i]);
	  if (errno)
	    return 0;
	}
    }
  else
    {
      /* Read all the longwords */
      for (i = 0; i < count; i++, addr += sizeof (int))
	{
	  errno = 0;
	  buffer[i] = nindy_fetch_word (addr);
	  if (errno)
	    return 0;
	  QUIT;
	}

      /* Copy appropriate bytes out of the buffer.  */
      bcopy ((char *) buffer + (memaddr & (sizeof (int) - 1)), myaddr, len);
    }
  return len;
}

/* The data cache records all the data read from the remote machine
   since the last time it stopped.

   Each cache block holds 16 bytes of data
   starting at a multiple-of-16 address.  */

#define DCACHE_SIZE 64		/* Number of cache blocks */

struct dcache_block {
	struct dcache_block *next, *last;
	unsigned int addr;	/* Address for which data is recorded.  */
	int data[4];
};

struct dcache_block dcache_free, dcache_valid;

/* Free all the data cache blocks, thus discarding all cached data.  */ 
static
void
dcache_flush ()
{
  register struct dcache_block *db;

  while ((db = dcache_valid.next) != &dcache_valid)
    {
      remque (db);
      insque (db, &dcache_free);
    }
}

/*
 * If addr is present in the dcache, return the address of the block
 * containing it.
 */
static
struct dcache_block *
dcache_hit (addr)
     unsigned int addr;
{
  register struct dcache_block *db;

  if (addr & 3)
    abort ();

  /* Search all cache blocks for one that is at this address.  */
  db = dcache_valid.next;
  while (db != &dcache_valid)
    {
      if ((addr & 0xfffffff0) == db->addr)
	return db;
      db = db->next;
    }
  return NULL;
}

/*  Return the int data at address ADDR in dcache block DC.  */
static
int
dcache_value (db, addr)
     struct dcache_block *db;
     unsigned int addr;
{
  if (addr & 3)
    abort ();
  return (db->data[(addr>>2)&3]);
}

/* Get a free cache block, put or keep it on the valid list,
   and return its address.  The caller should store into the block
   the address and data that it describes, then remque it from the
   free list and insert it into the valid list.  This procedure
   prevents errors from creeping in if a ninMemGet is interrupted
   (which used to put garbage blocks in the valid list...).  */
static
struct dcache_block *
dcache_alloc ()
{
  register struct dcache_block *db;

  if ((db = dcache_free.next) == &dcache_free)
    {
      /* If we can't get one from the free list, take last valid and put
	 it on the free list.  */
      db = dcache_valid.last;
      remque (db);
      insque (db, &dcache_free);
    }

  remque (db);
  insque (db, &dcache_valid);
  return (db);
}

/* Return the contents of the word at address ADDR in the remote machine,
   using the data cache.  */
static
int
dcache_fetch (addr)
     CORE_ADDR addr;
{
  register struct dcache_block *db;

  db = dcache_hit (addr);
  if (db == 0)
    {
      db = dcache_alloc ();
      immediate_quit++;
      ninMemGet(addr & ~0xf, (unsigned char *)db->data, 16);
      immediate_quit--;
      db->addr = addr & ~0xf;
      remque (db);			/* Off the free list */
      insque (db, &dcache_valid);	/* On the valid list */
    }
  return (dcache_value (db, addr));
}

/* Write the word at ADDR both in the data cache and in the remote machine.  */
static void
dcache_poke (addr, data)
     CORE_ADDR addr;
     int data;
{
  register struct dcache_block *db;

  /* First make sure the word is IN the cache.  DB is its cache block.  */
  db = dcache_hit (addr);
  if (db == 0)
    {
      db = dcache_alloc ();
      immediate_quit++;
      ninMemGet(addr & ~0xf, (unsigned char *)db->data, 16);
      immediate_quit--;
      db->addr = addr & ~0xf;
      remque (db);			/* Off the free list */
      insque (db, &dcache_valid);	/* On the valid list */
    }

  /* Modify the word in the cache.  */
  db->data[(addr>>2)&3] = data;

  /* Send the changed word.  */
  immediate_quit++;
  ninMemPut(addr, (unsigned char *)&data, 4);
  immediate_quit--;
}

/* Initialize the data cache.  */
static void
dcache_init ()
{
  register i;
  register struct dcache_block *db;

  db = (struct dcache_block *) xmalloc (sizeof (struct dcache_block) * 
					DCACHE_SIZE);
  dcache_free.next = dcache_free.last = &dcache_free;
  dcache_valid.next = dcache_valid.last = &dcache_valid;
  for (i=0;i<DCACHE_SIZE;i++,db++)
    insque (db, &dcache_free);
}


static void
nindy_create_inferior (execfile, args, env)
     char *execfile;
     char *args;
     char **env;
{
  int entry_pt;
  int pid;

  if (args && *args)
    error ("Can't pass arguments to remote NINDY process");

  entry_pt = (int) bfd_get_start_address (exec_bfd);

  /* Now that we have a child process, make it our target.  */
  /* We might want to do this to rearrange nindy_ops on top of e.g. exec_ops? */
  unpush_target (&nindy_ops);
  push_target (&nindy_ops);

  pid = 42;

#ifdef CREATE_INFERIOR_HOOK
  CREATE_INFERIOR_HOOK (pid);
#endif  

/* The "process" (board) is already stopped awaiting our commands, and
   the program is already downloaded.  We just set its PC and go.  */

  inferior_pid = pid;		/* Needed for wait_for_inferior below */

  clear_proceed_status ();

#if defined (START_INFERIOR_HOOK)
  START_INFERIOR_HOOK ();
#endif

  /* Tell wait_for_inferior that we've started a new process.  */
  init_wait_for_inferior ();

  /* Set up the "saved terminal modes" of the inferior
     based on what modes we are starting it with.  */
  target_terminal_init ();

  /* Install inferior's terminal modes.  */
  target_terminal_inferior ();

  /* remote_start(args); */
  /* trap_expected = 0; */
  /* insert_step_breakpoint ();  FIXME, do we need this?  */
  proceed ((CORE_ADDR)entry_pt, -1, 0);		/* Let 'er rip... */
}

static void
reset_command(args, from_tty)
     char *args;
     int from_tty;
{
	if ( !nindy_fd ){
	    error( "No target system to reset -- use 'target nindy' command.");
	}
	if ( query("Really reset the target system?",0,0) ){
		send_break( nindy_fd );
		tty_flush( nindy_fd );
	}
}

void
nindy_kill (args, from_tty)
     char *args;
     int from_tty;
{
  return;		/* Ignore attempts to kill target system */
}

/* Clean up when a program exits.

   The program actually lives on in the remote processor's RAM, and may be
   run again without a download.  Don't leave it full of breakpoint
   instructions.  */

void
nindy_mourn_inferior ()
{
  remove_breakpoints ();
  generic_mourn_inferior ();	/* Do all the proper things now */
}

/* This routine is run as a hook, just before the main command loop is
   entered.  If gdb is configured for the i960, but has not had its
   nindy target specified yet, this will loop prompting the user to do so.

   Unlike the loop provided by Intel, we actually let the user get out
   of this with a RETURN.  This is useful when e.g. simply examining
   an i960 object file on the host system.  */

nindy_before_main_loop ()
{
  char ttyname[100];
  char *p, *p2;

  setjmp(to_top_level);
  while (current_target != &nindy_ops) { /* remote tty not specified yet */
	if ( instream == stdin ){
		printf("\nAttach /dev/ttyNN -- specify NN, or \"quit\" to quit:  ");
		fflush( stdout );
	}
	fgets( ttyname, sizeof(ttyname)-1, stdin );

	/* Strip leading and trailing whitespace */
	for ( p = ttyname; isspace(*p); p++ ){
		;
	}
	if ( *p == '\0' ){
		return;		/* User just hit spaces or return, wants out */
	}
	for ( p2= p; !isspace(*p2) && (*p2 != '\0'); p2++ ){
		;
	}
	*p2= '\0';
	if ( !strcmp("quit",p) ){
		exit(1);
	}

	nindy_open( p, 1 );

	/* Now that we have a tty open for talking to the remote machine,
	   download the executable file if one was specified.  */
	if ( !setjmp(to_top_level) && exec_bfd ) {
	      target_add_file (bfd_get_filename (exec_bfd), 1);
	}
  }
}

/* Define the target subroutine names */

struct target_ops nindy_ops = {
	"nindy", "Remote serial target in i960 NINDY-specific protocol",
	nindy_open, nindy_detach, nindy_resume, nindy_wait,
	nindy_fetch_registers, nindy_store_registers,
	nindy_prepare_to_store, 0, 0, /* conv_from, conv_to */
	nindy_xfer_inferior_memory, nindy_files_info,
	0, 0, /* insert_breakpoint, remove_breakpoint, */
	0, 0, 0, 0, 0,	/* Terminal crud */
	nindy_kill,
	nindy_load,
	call_function_by_hand,
	0, /* lookup_symbol */
	nindy_create_inferior,
	nindy_mourn_inferior,
	0, /* next */
	1, 1, 1, 1, 1,	/* all mem, mem, stack, regs, exec */
	OPS_MAGIC,		/* Always the last thing */
};

void
_initialize_nindy ()
{
  add_target (&nindy_ops);
  add_com ("reset", class_obscure, reset_command,
	   "Send a 'break' to the remote target system.\n\
Only useful if the target has been equipped with a circuit\n\
to perform a hard reset when a break is detected.");
}
