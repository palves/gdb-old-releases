/* Main code for remote server for GDB.
   Copyright (C) 1989, 1993 Free Software Foundation, Inc.

This file is part of GDB.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "server.h"

int cont_thread;
int general_thread;
int thread_from_wait;
int old_thread_from_wait;

int
main (argc, argv)
     int argc;
     char *argv[];
{
  char ch, status, own_buf[2000], mem_buf[2000];
  int i = 0;
  unsigned char signal;
  unsigned int mem_addr, len;

  if (setjmp(toplevel))
    {
      fprintf(stderr, "Exiting\n");
      exit(1);
    }

  if (argc < 3)
    error("Usage: gdbserver tty prog [args ...]");

  inferior_pid = create_inferior (argv[2], &argv[2]);
  fprintf (stderr, "Process %s created; pid = %d\n", argv[2], inferior_pid);

  signal = mywait (&status);	/* Wait till we are at 1st instr in prog */

  /* We are now stopped at the first instruction of the target process */

  while (1)
    {
      remote_open (argv[1]);

      setjmp(toplevel);
      while (getpkt (own_buf) > 0)
	{
	  unsigned char sig;
	  i = 0;
	  ch = own_buf[i++];
	  switch (ch)
	    {
	    case '?':
	      prepare_resume_reply (own_buf, status, signal);
	      break;
	    case 'H':
	      switch (own_buf[1])
		{
		case 'g':
		  general_thread = strtol (&own_buf[2], NULL, 16);
		  write_ok (own_buf);
		  fetch_inferior_registers (0);
		  break;
		case 'c':
		  cont_thread = strtol (&own_buf[2], NULL, 16);
		  write_ok (own_buf);
		  break;
		default:
		  /* Silently ignore it so that gdb can extend the protocol
		     without compatibility headaches.  */
		  own_buf[0] = '\0';
		  break;
		}
	      break;
	    case 'g':
	      convert_int_to_ascii (registers, own_buf, REGISTER_BYTES);
	      break;
	    case 'G':
	      convert_ascii_to_int (&own_buf[1], registers, REGISTER_BYTES);
	      store_inferior_registers (-1);
	      write_ok (own_buf);
	      break;
	    case 'm':
	      decode_m_packet (&own_buf[1], &mem_addr, &len);
	      read_inferior_memory (mem_addr, mem_buf, len);
	      convert_int_to_ascii (mem_buf, own_buf, len);
	      break;
	    case 'M':
	      decode_M_packet (&own_buf[1], &mem_addr, &len, mem_buf);
	      if (write_inferior_memory (mem_addr, mem_buf, len) == 0)
		write_ok (own_buf);
	      else
		write_enn (own_buf);
	      break;
	    case 'C':
	      convert_ascii_to_int (own_buf + 1, &sig, 1);
	      myresume (0, sig);
	      signal = mywait (&status);
	      prepare_resume_reply (own_buf, status, signal);
	      break;
	    case 'S':
	      convert_ascii_to_int (own_buf + 1, &sig, 1);
	      myresume (1, sig);
	      signal = mywait (&status);
	      prepare_resume_reply (own_buf, status, signal);
	      break;
	    case 'c':
	      myresume (0, 0);
	      signal = mywait (&status);
	      prepare_resume_reply (own_buf, status, signal);
	      break;
	    case 's':
	      myresume (1, 0);
	      signal = mywait (&status);
	      prepare_resume_reply (own_buf, status, signal);
	      break;
	    case 'k':
	      fprintf (stderr, "Killing inferior\n");
	      kill_inferior ();
	      fprintf (stderr, "GDBserver exiting\n");
	      exit (0);
	      break;
	    default:
	      /* It is a request we don't understand.  Respond with an
		 empty packet so that gdb knows that we don't support this
		 request.  */
	      own_buf[0] = '\0';
	      break;
	    }

	  putpkt (own_buf);

	  if (status == 'W')
	    fprintf (stderr,
		     "\nChild exited with status %d\n", sig);
	  if (status == 'X')
	    fprintf (stderr, "\nChild terminated with signal = 0x%x\n", sig);
	  if (status == 'W' || status == 'X')
	    {
	      fprintf (stderr, "GDBserver exiting\n");
	      exit (0);
	    }
	}

      /* We come here when getpkt fails.  Close the connection, and re-open it
         at the top of the loop.  */

      fprintf (stderr, "Remote side has terminated connection.  GDBserver will reopen the connection.\n");

      remote_close ();
    }
}
