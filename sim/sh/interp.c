/* Simulator for the Hitachi SH architecture.

   Written by Steve Chamberlain of Cygnus Support.
   sac@cygnus.com

   This file is part of SH sim


		THIS SOFTWARE IS NOT COPYRIGHTED

   Cygnus offers the following for use in the public domain.  Cygnus
   makes no warranty with regard to the software or it's performance
   and the user accepts the software "AS IS" with all faults.

   CYGNUS DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD TO
   THIS SOFTWARE INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

*/
#include <signal.h>
#include "sysdep.h"
#include <sys/times.h>
#include <sys/param.h>
#include "bfd.h"
#define O_RECOMPILE 85
#define DEFINE_TABLE

#define DISASSEMBLER_TABLE

#define SBIT(x) ((x)&sbit)
#define R0 	saved_state.asregs.regs[0]
#define Rn 	saved_state.asregs.regs[n]
#define Rm 	saved_state.asregs.regs[m]
#define UR0 	(unsigned int)(saved_state.asregs.regs[0])
#define UR 	(unsigned int)R
#define UR 	(unsigned int)R
#define SR0 	saved_state.asregs.regs[0]
#define GBR 	saved_state.asregs.gbr
#define VBR 	saved_state.asregs.vbr
#define MACH 	saved_state.asregs.mach
#define MACL 	saved_state.asregs.macl
#define M 	saved_state.asregs.sr.bits.m
#define Q 	saved_state.asregs.sr.bits.q

#define GET_SR() (saved_state.asregs.sr.bits.t = T, saved_state.asregs.sr.word)
#define SET_SR(x) {saved_state.asregs.sr.word = (x); T =saved_state.asregs.sr.bits.t;}

#define PC pc
#define C cycles

#ifdef TARGET_BIG_ENDIAN
#define LMEM(x) 	*((long *)(memory+((x)&maskl)))
#define BMEM(x) 	*((char *)(memory+((x)&maskb)))
#define UWMEM(x) 	*((unsigned short *)(memory+((x)&maskw)))
#define SWMEM(x) 	*((short *)(memory+((x)&maskw)))
#define WLAT(x,value)  	(LMEM(x) = value)
#define RLAT(x)  	(LMEM(x))
#define WWAT(x,value)  	(UWMEM(x) = value)
#define RSWAT(x)  	(SWMEM(x))
#define RUWAT(x)  	(UWMEM(x))
#define WBAT(x,value)  	(BMEM(x) = value)
#define RBAT(x)  	(BMEM(x))
#else
/* For little endian or unknown host machines */
#define WLAT(x,value)\
{ int v = value; unsigned char *p = memory + ((x) & maskl);\
  p[0] =v>>24;p[1] = v>>16;p[2]=v>>8;p[3]=v; }

#define WWAT(x,value)\
{ int v = value; unsigned char *p = memory + (x & maskw);p[0] =v>>8;p[1] = v ;}

#define WBAT(x,value)\
{ unsigned char *p = memory + (x & maskb);p[0] =value;}

#define RLAT(x)\
  ((memory[x&maskl]<<24)|(memory[(x&maskl)+1]<<16)|(memory[(x&maskl)+2]<<8)| (memory[(x&maskl)+3]))

#define RWAT(x)\
  ((memory[x&maskw]<<8)|(memory[(x&maskw)+1]))

#define RBAT(x)\
  ((memory[x&maskb]))

#define RUWAT(x)  (RWAT(x) & 0xffff)
#define RSWAT(x)  ((short)(RWAT(x)))
#define RSBAT(x)  (SEXT(RBAT(x)))

#endif



#define SEXT(x)     	(((x&0xff) ^ (~0x7f))+0x80)
#define SEXTW(y)    	((int)((short)y))

#define SL(TEMPPC)  	iword= RUWAT(TEMPPC); goto top;


#define L(x)   thislock = x;
#define TL(x)  if ((x) == prevlock) stalls++;
#define TB(x,y)  if ((x) == prevlock || (y)==prevlock) stalls++;
int sim_memory_size = 19;
static int sim_profile_size = 17;
static int nsamples;
static int sim_timeout;

typedef union
{

  struct
    {

      int regs[16];
      int pc;
      int pr;

      int gbr;
      int vbr;
      int mach;
      int macl;


      union
	{
	  struct
	    {
	      unsigned int d0:22;
	      unsigned int m:1;
	      unsigned int q:1;
	      unsigned int i:4;
	      unsigned int d1:2;
	      unsigned int s:1;
	      unsigned int t:1;
	    }
	  bits;
	  int word;
	}
      sr;
      int ticks;
      int stalls;
      int cycles;
      int insts;


      int prevlock;
      int thislock;
      int exception;
      int msize;
#define PROFILE_FREQ 1
#define PROFILE_SHIFT 2
      int profile;
      unsigned short *profile_hist;
      unsigned char *memory;

    }
  asregs;
  int asints[28];

}

saved_state_type;
saved_state_type saved_state;

static int
get_now ()
{
  return time((char*)0);
}

static int
now_persec ()
{
  return 1;
}



static FILE *profile_file;

static void 
swap (b, n)
     unsigned char *b;
     int n;
{
  b[0] = n >> 24;
  b[1] = n >> 16;
  b[2] = n >> 8;
  b[3] = n >> 0;
}
static void 
swap16 (b, n)
     unsigned char *b;
     int n;
{
  b[0] = n >> 8;
  b[1] = n >> 0;
}

static void
swapout (n)
     int n;
{
  if (profile_file)
    {
      char b[4];
      swap (b, n);
      fwrite (b, 4, 1, profile_file);
    }
}

static void
swapout16 (n)
     int n;
{
  char b[4];
  swap16 (b, n);
  fwrite (b, 2, 1, profile_file);
}


/* Turn a pointer in a register into a pointer into real memory. */

static char *
ptr (x)
     int x;
{
  return (char *) (x + saved_state.asregs.memory);
}

/* Simulate a monitor trap.  */

static void
trap (i, regs)
     int i;
     int *regs;
{
  switch (i)
    {
    case 1:
      printf ("%c", regs[0]);
      break;
    case 2:
      saved_state.asregs.exception = SIGQUIT;
      break;
    case 3:
      {
	extern int errno;
	int perrno = errno;
	errno = 0;

	switch (regs[4])
	  {
	  case 3:
	    regs[4] = read (regs[5], ptr (regs[6]), regs[7]);
	    break;
	  case 4:
	    regs[4] = write (regs[5], ptr (regs[6]), regs[7]);
	    break;
	  case 19:
	    regs[4] = lseek (regs[5], regs[6], regs[7]);
	    break;
	  case 6:
	    regs[4] = close (regs[5]);
	    break;
	  case 5:
	    regs[4] = open (ptr (regs[5]), regs[6]);
	    break;
	  case 1:
	    /* EXIT */
	    saved_state.asregs.exception = SIGQUIT;
	    errno = regs[5];
	    break;
	    
	  default:
	    abort ();
	  }
	regs[0] = errno;
	errno = perrno;
      }

      break;

    case 255:
      saved_state.asregs.exception = SIGTRAP;
      break;
    }

}
void
control_c (sig, code, scp, addr)
     int sig;
     int code;
     char *scp;
     char *addr;
{
  saved_state.asregs.exception = SIGINT;
}


static int
div1 (R, iRn2, iRn1, T)
     int *R;
     int iRn1;
     int iRn2;
     int T;
{
  unsigned long tmp0;
  unsigned char old_q, tmp1;

  old_q = Q;
  Q = (unsigned char) ((0x80000000 & R[iRn1]) != 0);
  R[iRn1] <<= 1;
  R[iRn1] |= (unsigned long) T;

  switch (old_q)
    {
    case 0:
      switch (M)
	{
	case 0:
	  tmp0 = R[iRn1];
	  R[iRn1] -= R[iRn2];
	  tmp1 = (R[iRn1] > tmp0);
	  switch (Q)
	    {
	    case 0:
	      Q = tmp1;
	      break;
	    case 1:
	      Q = (unsigned char) (tmp1 == 0);
	      break;
	    }
	  break;
	case 1:
	  tmp0 = R[iRn1];
	  R[iRn1] += R[iRn2];
	  tmp1 = (R[iRn1] < tmp0);
	  switch (Q)
	    {
	    case 0:
	      Q = (unsigned char) (tmp1 == 0);
	      break;
	    case 1:
	      Q = tmp1;
	      break;
	    }
	  break;
	}
      break;
    case 1:
      switch (M)
	{
	case 0:
	  tmp0 = R[iRn1];
	  R[iRn1] += R[iRn2];
	  tmp1 = (R[iRn1] < tmp0);
	  switch (Q)
	    {
	    case 0:
	      Q = tmp1;
	      break;
	    case 1:
	      Q = (unsigned char) (tmp1 == 0);
	      break;
	    }
	  break;
	case 1:
	  tmp0 = R[iRn1];
	  R[iRn1] -= R[iRn2];
	  tmp1 = (R[iRn1] > tmp0);
	  switch (Q)
	    {
	    case 0:
	      Q = (unsigned char) (tmp1 == 0);
	      break;
	    case 1:
	      Q = tmp1;
	      break;
	    }
	  break;
	}
      break;
    }
  T = (Q == M);
  return T;
}


static void 
dmul (sign, rm, rn)
     int sign;
     unsigned int rm;
     unsigned int rn;
{
  unsigned long RnL, RnH;
  unsigned long RmL, RmH;
  unsigned long temp0, temp1, temp2, temp3;
  unsigned long Res2, Res1, Res0;


  if (!sign)
    {

      RnL = rn & 0xffff;
      RnH = (rn >> 16) & 0xffff;
      RmL = rm & 0xffff;
      RmH = (rm >> 16) & 0xffff;
      temp0 = RmL * RnL;
      temp1 = RmH * RnL;
      temp2 = RmL * RnH;
      temp3 = RmH * RnH;
      Res2 = 0;
      Res1 = temp1 + temp2;
      if (Res1 < temp1)
	Res2 += 0x00010000;
      temp1 = (Res1 << 16) & 0xffff0000;
      Res0 = temp0 + temp1;
      if (Res0 < temp0)
	Res2 += 1;
      Res2 += ((Res1 >> 16) & 0xffff) + temp3;
      MACH = Res2;
      MACL = Res0;

    }

  else
    {
      abort ();
    }

}


/* Set the memory size to the power of two provided. */

void
sim_size (power)
     int power;

{
  saved_state.asregs.msize = 1 << power;

  sim_memory_size = power;


  if (saved_state.asregs.memory)
    {
      free (saved_state.asregs.memory);
    }

  saved_state.asregs.memory =
    (unsigned char *) calloc (64, saved_state.asregs.msize / 64);

  if (!saved_state.asregs.memory)
    {
      fprintf (stderr,
	       "Not enough VM for simuation of %d bytes of RAM\n",
	       saved_state.asregs.msize);

      saved_state.asregs.msize = 1;
      saved_state.asregs.memory = (unsigned char *) calloc (1, 1);
    }
}



static
void
init_pointers ()
{
  if (saved_state.asregs.msize != 1 << sim_memory_size)
    {
      sim_size (sim_memory_size);
    }

  if (saved_state.asregs.profile && !profile_file)
    {
      profile_file = fopen ("gmon.out", "wb");
      /* Seek to where to put the call arc data */
      nsamples = (1 << sim_profile_size);

      fseek (profile_file, nsamples * 2 + 12, 0);

      if (!profile_file)
	{
	  fprintf (stderr, "Can't open gmon.out\n");
	}
      else
	{
	  saved_state.asregs.profile_hist =
	    (unsigned short *) calloc (64, (nsamples * sizeof (short) / 64));
	}
    }
}

static void
dump_profile ()
{
  unsigned int minpc;
  unsigned int maxpc;
  unsigned short *p;

  int thisshift;

  unsigned short *first;

  int i;
  p = saved_state.asregs.profile_hist;
  minpc = 0;
  maxpc = (1 << sim_profile_size);

  fseek (profile_file, 0L, 0);
  swapout (minpc << PROFILE_SHIFT);
  swapout (maxpc << PROFILE_SHIFT);
  swapout (nsamples * 2 + 12);
  for (i = 0; i < nsamples; i++)
    swapout16 (saved_state.asregs.profile_hist[i]);

}

static int 
gotcall (from, to)
     int from;
     int to;
{
  swapout (from);
  swapout (to);
  swapout (1);
}

#define MMASKB ((saved_state.asregs.msize -1) & ~0)
void
sim_resume (step)
     int step;
{
  register unsigned int pc;
  register int cycles = 0;
  register int stalls = 0;
  register int insts = 0;
  register int prevlock;
  register int thislock;
  register unsigned int doprofile;

  int tick_start = get_now ();
  void (*prev) ();
  extern unsigned char sh_jump_table0[];

  register unsigned char *jump_table = sh_jump_table0;

  register int *R = &(saved_state.asregs.regs[0]);
  register int T;
  register int PR;

  register int maskb = ((saved_state.asregs.msize - 1) & ~0);
  register int maskw = ((saved_state.asregs.msize - 1) & ~1);
  register int maskl = ((saved_state.asregs.msize - 1) & ~3);
  register unsigned char *memory ;
  register unsigned int sbit = (1 << 31);

  prev = signal (SIGINT, control_c);

  init_pointers ();

  memory = saved_state.asregs.memory;

  if (step)
    {
      saved_state.asregs.exception = SIGTRAP;
    }
  else
    {
      saved_state.asregs.exception = 0;
    }

  pc = saved_state.asregs.pc;
  PR = saved_state.asregs.pr;
  T = saved_state.asregs.sr.bits.t;
  prevlock = saved_state.asregs.prevlock;
  thislock = saved_state.asregs.thislock;
  doprofile = saved_state.asregs.profile;

  /* If profiling not enabled, disable it by asking for
     profiles infrequently. */
  if (doprofile == 0)
    doprofile = ~0;

  do
    {
      register unsigned int iword = RUWAT (pc);
      register unsigned int ult;

      insts++;
    top:

#include "code.c"


      pc += 2;
      prevlock = thislock;
      thislock = 30;
      cycles++;

      if (cycles >= doprofile)
	{
	  if (cycles > sim_timeout)
	    saved_state.asregs.exception = SIGQUIT;
	  
	  saved_state.asregs.cycles += doprofile;
	  cycles -= doprofile;
	  if (saved_state.asregs.profile_hist)
	    {
	      int n = pc >> PROFILE_SHIFT;
	      if (n < nsamples)
		{
		  int i = saved_state.asregs.profile_hist[n];
		  if (i < 65000)
		    saved_state.asregs.profile_hist[n] = i + 1;
		}

	    }
	}
    }
  while (!saved_state.asregs.exception);

  if (saved_state.asregs.exception == SIGILL)
    {
      pc -= 2;
    }

  saved_state.asregs.ticks += get_now () - tick_start;
  saved_state.asregs.cycles += cycles;
  saved_state.asregs.stalls += stalls;
  saved_state.asregs.insts += insts;
  saved_state.asregs.pc = pc;
  saved_state.asregs.sr.bits.t = T;
  saved_state.asregs.pr = PR;

  saved_state.asregs.prevlock = prevlock;
  saved_state.asregs.thislock = thislock;


  if (profile_file)
    {
      dump_profile ();
    }

  signal (SIGINT, prev);
}




int
sim_write (addr, buffer, size)
     long int addr;
     unsigned char *buffer;
     int size;
{
  int i;
  init_pointers ();

  for (i = 0; i < size; i++)
    {
      saved_state.asregs.memory[MMASKB & (addr + i)] = buffer[i];
    }
return size;
}

int
sim_read (addr, buffer, size)
     long int addr;
     char *buffer;
     int size;
{
  int i;

  init_pointers ();

  for (i = 0; i < size; i++)
    {
      buffer[i] = saved_state.asregs.memory[MMASKB & (addr + i)];
    }
return size;
}


void
sim_store_register (rn, value)
     int rn;
     unsigned char value[4];
{
  saved_state.asregs.regs[rn] = (value[0] << 24) | (value[1] << 16) | (value[2] << 8) | (value[3]);
}

void
sim_fetch_register (rn, buf)
     int rn;
     char *buf;
{
  int value = ((int *) (&saved_state))[rn];

  swap (buf, value);
}


int
sim_trace ()
{
  return 0;
}

int
sim_stop_signal ()
{
  return saved_state.asregs.exception;
}

void
sim_set_pc (x)
     int x;
{
  saved_state.asregs.pc = x;
}


void
sim_info ()
{
  double timetaken = (double) saved_state.asregs.ticks / (double) now_persec ();
  double virttime = saved_state.asregs.cycles / 36.0e6;

  printf ("\n\n# instructions executed  %10d\n", saved_state.asregs.insts);
  printf ("# cycles                 %10d\n", saved_state.asregs.cycles);
  printf ("# pipeline stalls        %10d\n", saved_state.asregs.stalls);
  printf ("# real time taken        %10.4f\n", timetaken);
  printf ("# virtual time taken     %10.4f\n", virttime);
  printf ("# profiling size         %10d\n", sim_profile_size);
  printf ("# profiling frequency    %10d\n", saved_state.asregs.profile);
  printf ("# profile maxpc          %10x\n", (1 << sim_profile_size) << PROFILE_SHIFT);

  if (timetaken != 0)
    {
      printf ("# cycles/second          %10d\n", (int) (saved_state.asregs.cycles / timetaken));
      printf ("# simulation ratio       %10.4f\n", virttime / timetaken);
    }
}


void
sim_set_profile (n)
     int n;
{
  saved_state.asregs.profile = n;
}

void
sim_set_profile_size (n)
     int n;
{
  sim_profile_size = n;
}


void
sim_kill()
{

}

int
sim_open()
{
  return 0;
}
int sim_set_args()
{
  return 0;
}


