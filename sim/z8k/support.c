/* support routines for interpreted instructions
   Copyright (C) 1992, 1993 Free Software Foundation, Inc.

This file is part of Z8KSIM

Z8KSIM is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

Z8KSIM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Z8KSIM; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <signal.h>

#include "sysdep.h"
#include "tm.h"
#include "sim.h"
#include "mem.h"
#include <sys/times.h>
#include <sys/param.h>
#include "remote-sim.h"
#include "syscall.h"

static int get_now PARAMS ((void));
static int now_persec PARAMS ((void));
static int put_long PARAMS ((sim_state_type * context, int ptr, int value));
static int put_short PARAMS ((sim_state_type * context, int ptr, int value));

int sim_z8001_mode;

static int
get_now ()
{
  struct tms b;

  times (&b);
  return b.tms_utime + b.tms_stime;
}

static int
now_persec ()
{
  return 50;
}


/* #define LOG /* define this to print instruction use counts */

#ifdef __GNUC__
#define INLINE __inline__
#include "inlines.h"
#else
#include "inlines.h"
#endif

/* This holds the entire cpu context */
static sim_state_type the_state;

int
fail (context, dummy)
     sim_state_type *context;
     int dummy;
{
  context->exception = SIM_BAD_INST;
  return 1;
}

void
sfop_bad1 (context)
     sim_state_type *context;
{
  context->exception
    = SIM_BAD_INST;
}

void
bfop_bad1 (context)
     sim_state_type *context;
{
  context->exception
    = SIM_BAD_INST;
}

void
fop_bad (context)
     sim_state_type *context;
{
  context->exception =
    SIM_BAD_INST;
}

/* Table of bit counts for all byte values */

char the_parity[256] =
{
  0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3,
  4, 2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4,
  4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2,
  3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5,
  4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4,
  5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3,
  3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2,
  3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6,
  4, 5, 5, 6, 5, 6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5,
  6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5,
  5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6,
  7, 7, 8};


int read ();
int write ();
int open ();
int close ();
int open ();
int close ();
int creat ();
int link ();
int fstat ();

static int
put_short (context, ptr, value)
     sim_state_type *context;
     int ptr;
     int value;
{
  put_word_mem_da (context, ptr, value);
  return ptr + 2;
}

static int
put_long (context, ptr, value)
     sim_state_type *context;
     int
       ptr;
     int value;
{
  put_long_mem_da (context, ptr, value);
  return ptr + 4;
}

#define aptr(x) ((sitoptr(x)) + (char *)(context->memory))

static int args[3];
static int arg_index;		/* Translate a z8k system call into a host system call */
void
support_call (context, sc)
     sim_state_type *context;
     int sc;
{
  extern int errno;
  int ret;
  int retnext = 0;
  int fd;

  int olderrno = errno;
  errno = 0;
  switch (sc)
    {
    case SYS_ARG:
      args[arg_index++] = context->regs[0].word << 16 | context->regs[1].word;
      break;
    case SYS_exit:
      context->exception = SIM_DONE;
      arg_index = 0;
      break;
    case SYS_close:
      ret = close ((int) (args[0]));
      arg_index = 0;
      break;
    case SYS_creat:
      ret = creat (aptr (args[0]), args[1]);
      arg_index = 0;
      break;
    case SYS_isatty:
      ret =
	isatty (args[0]);
      arg_index = 0;
      break;
    case SYS_open:
      ret = open (aptr (args[0]), args[1], args[2]);
      arg_index = 0;
      break;
      case
    SYS_lseek:
      ret = lseek (args[0], (off_t) args[1], args[2]);
      arg_index =
	0;
      break;
    case SYS_read:
      ret = read (args[0], aptr (args[1]), args[2]);
      arg_index = 0;
      break;
    case SYS_write:
      ret = write (args[0],aptr (args[1]), args[2]);
      arg_index = 0;
      break;
    case SYS_time:
      {
	int
	  dst = args[0];

	ret = time (0);
	if (dst)
	  {
	    put_long_mem_da (context,
			     sitoptr (dst), ret);
	  }
	retnext = ret;
	ret = retnext >> 16;
	arg_index =
	  0;
      }
      break;
    case SYS_fstat:
      {
	int buf;
	struct stat host_stat;
	fd =
	  args[0];
	buf = sitoptr (args[1]);
	ret = fstat (fd, &host_stat);
	buf = put_short (context, buf, host_stat.st_dev);
	buf = put_short (context, buf, host_stat.st_ino);
	buf = put_short (context, buf, host_stat.st_mode);
	buf = put_short (context, buf, host_stat.st_nlink);
	buf = put_short (context, buf, host_stat.st_uid);
	buf = put_short (context, buf, host_stat.st_uid);
	buf = put_short (context, buf, host_stat.st_rdev);
	buf = put_long (context, buf, host_stat.st_size);
	buf = put_long (context, buf, host_stat.st_atime);
	arg_index = 0;
      } break;
    default:
    case SYS_link:
      context->exception = SIM_BAD_SYSCALL;
      arg_index = 0;
      break;
    }
  context->regs[2].word = ret;
  context->regs[3].word = retnext;
  context->regs[5].word = errno;
  errno = olderrno;
}

#undef get_word_mem_da

int
get_word_mem_da (context, addr)
     sim_state_type *context;
     int addr;
{
  return (get_byte_mem_da (context, addr) << 8) | (get_byte_mem_da (context, addr + 1));

}

#undef get_word_reg
int 
get_word_reg (context, reg) sim_state_type
* context;
     int reg;
{
  return context->regs[reg].word;
}

#ifdef LOG
int log[64 * 1024];

#endif

void
tm_store_register (regno, value)
     int regno;
     int value;
{
  switch
    (regno)
    {
    case REG_PC:
      the_state.sometimes_pc = value;
      break;

    default:
      put_word_reg (&the_state, regno, value);
    }
}

void
swap_long (buf, val)
     char *buf;
     int val;
{
  buf[0] = val >> 24;
  buf[1] = val >> 16;
  buf[2] = val >> 8;
  buf[3] = val >> 0;
}

void
swap_word (buf, val)
     char *buf;
     int val;
{
  buf[0] = val >> 8;
  buf[1] = val >> 0;
}

void
tm_fetch_register (regno, buf)
     int regno;
     char *buf;
{
  switch
    (regno)
    {
    case REG_CYCLES:
      swap_long (buf, the_state.cycles);
      break;
    case REG_INSTS:
      swap_long (buf, the_state.insts);
      break;
      case
    REG_TIME:
      swap_long (buf, the_state.ticks);
      break;
    case REG_PC:
      swap_long (buf, the_state.sometimes_pc);
      break;
    case REG_SP:
      {
	if
	  (sim_z8001_mode)
	  {
	    swap_long (buf, get_long_reg (&the_state, 14));
	  }
	else
	  {
	    swap_long (buf, get_word_reg (&the_state, 15));
	  }
      }
      break;
      case
    REG_FP:
      {
	if (sim_z8001_mode)
	  {
	    swap_long (buf, get_long_reg
		       (&the_state, 10));
	  }
	else
	  {
	    swap_long (buf,
		       get_word_reg (&the_state, 10));
	  }
      }
      break;
    default:
      {
	swap_word (buf,
		   get_word_reg (&the_state, regno));
      }
    }
}

void
tm_resume (step)
     int step;
{
  int now = get_now ();
  struct op_info
   *p;
  int word;
  int pc;
  extern int (*(sfop_table[])) ();
  extern int (*(bfop_table[])) ();
  int (*((*table))) ();
  sim_state_type *context = &the_state;

  if (step)
    {
      context->exception = SIM_SINGLE_STEP;
    }
  else
    {
      context->exception = 0;
    }

  pc = context->sometimes_pc;
  if (sim_z8001_mode)
    {
      table = bfop_table;
      pc = MAP_PHYSICAL_TO_LOGICAL (pc);
    }
  else
    {
      table = sfop_table;
    }


  do
    {
      word = get_word_mem_da (context, pc);
      p = op_info_table + word;

#ifdef LOG
      log[word]++;
#endif
      pc = table[p->exec] (context, pc, word);
      context->insts++;

    }
  while (!context->exception);



  context->sometimes_pc = MAP_LOGICAL_TO_PHYSICAL (pc);
  context->ticks += get_now () - now;
}

int
tm_signal ()
{
  return the_state.exception;
}

void
tm_info_print (x)
     sim_state_type *x;
{
  double timetaken = (double) x->ticks / (double) now_persec ();
  double virttime = x->cycles / 4.0e6;

  printf ("instructions executed            : %9d\n", x->insts);
  printf ("cycles counted                   : %9d \n", x->cycles);
  printf ("cycles / inst                    : %9.1f \n", (double) x->cycles / (double) x->insts);
  printf ("virtual time taked (at 4 Mhz)    : %9.1f \n", virttime);
  printf ("real time taken                  : %9.1f \n", timetaken);

  if (timetaken)
    {
      printf ("virtual instructions per second  : %9.1f\n", x->insts / timetaken);
      printf ("emulation speed                  : %9.1f%%\n", virttime / timetaken * 100.0);
    }
#ifdef LOG
  {
    extern int quick[];

    for (i = 0; quick[i]; i++)
      {
	log[quick[i]] += 100000;
      }
  }

  for (i = 0; i < 64 * 1024; i++)
    {
      if (log[i])
	{
	  printf ("			/*%7d*/ 0x%x,\n", log[i], i);
	}
    }
#endif

}

int
sim_trace ()
{
  int i;
  char buffer[10];
  int r;

  printf ("\n");
  for (r = 0; r < 16; r++)
    {
      int m;

      printf ("r%2d", r);
      printf ("=%04x ", get_word_reg (&the_state,
				      r));
      for (m = -4; m < 8; m++)
	{
	  if (m == 0)
	    printf (">");
	  printf ("%04x ",
		  get_word_mem_da (&the_state, (0xfffe & get_word_reg (&the_state, r)) + m * 2));
	}
      printf ("\n");
    }

  printf ("\n");
  printf ("%9d %9d %08x ", the_state.cycles,
	  the_state.insts, the_state.sometimes_pc);

  for (i = 0; i < 6; i++)
    {
      buffer[i] = get_byte_mem_da (&the_state,
				   the_state.sometimes_pc + i);
    }

  print_insn_z8001 (the_state.sometimes_pc, buffer, stdout);
  printf
    ("\n");
  tm_resume (1);
  if (the_state.exception != SIM_SINGLE_STEP)
    return 1;
  return 0;
}

void
tm_state (x)
     sim_state_type *x;
{
  *x = the_state;
}

void
tm_exception (x)
     int x;
{
  the_state.exception = x;
}

int
tm_read_byte (x)
     int x;
{
  x &= 0x3f00ffff;
  return sim_read_byte (&the_state, x);
}

void
tm_write_byte (x, y)
     int x, y;
{
  x &= 0x3f00ffff;
  sim_write_byte (&the_state, x, y);
}

int
sim_open (name)
     char *name;
{
  return 0;
}

int 
sim_set_args (argv, env)
     char **argv;
     char **env;
{
  return 0;
}


int
sim_kill ()
{
  return 0;
}
