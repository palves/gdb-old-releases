/*  This file is part of the program psim.

    Copyright (C) 1994-1995, Andrew Cagney <cagney@highland.com.au>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 
    */


#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

#include "config.h"
#include "misc.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#else
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#endif

void
error (char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  vprintf(msg, ap);
  va_end(ap);
  exit (1);
}

void *
zalloc(long size)
{
  void *memory = malloc(size);
  if (memory == NULL)
    error("zalloc failed\n");
  memset(memory, 0, size);
  return memory;
}

void
dumpf (int indent, char *msg, ...)
{
  va_list ap;
  for (; indent > 0; indent--)
    printf(" ");
  va_start(ap, msg);
  vprintf(msg, ap);
  va_end(ap);
}


int
it_is(const char *flag,
      const char *flags)
{
  int flag_len = strlen(flag);
  while (*flags != '\0') {
    if (!strncmp(flags, flag, flag_len)
	&& (flags[flag_len] == ',' || flags[flag_len] == '\0'))
      return 1;
    while (*flags != ',') {
      if (*flags == '\0')
	return 0;
      flags++;
    }
    flags++;
  }
  return 0;
}


unsigned
a2i(const char *a)
{
  int neg = 0;
  int base = 10;
  unsigned num = 0;
  int looping;

  while (isspace (*a))
    a++;

  if (*a == '-') {
    neg = 1;
    a++;
  }

  if (*a == '0') {
    if (a[1] == 'x' || a[1] == 'X') {
      a += 2;
      base = 16;
    }
    else
      base = 8;
  }

  looping = 1;
  while (looping) {
    int ch = *a++;

    switch (base) {
    default:
      looping = 0;
      break;

    case 10:
      if (ch >= '0' && ch <= '9') {
	num = (num * 10) + (ch - '0');
      } else {
	looping = 0;
      }
      break;

    case 8:
      if (ch >= '0' && ch <= '7') {
	num = (num * 8) + (ch - '0');
      } else {
	looping = 0;
      }
      break;

    case 16:
      if (ch >= '0' && ch <= '9') {
	num = (num * 16) + (ch - '0');
      } else if (ch >= 'a' && ch <= 'f') {
	num = (num * 16) + (ch - 'a' + 10);
      } else if (ch >= 'A' && ch <= 'F') {
	num = (num * 16) + (ch - 'A' + 10);
      } else {
	looping = 0;
      }
      break;
    }
  }

  if (neg)
    num = - num;

  return num;
}

unsigned
target_a2i(int ms_bit_nr,
	   const char *a)
{
  if (ms_bit_nr)
    return (ms_bit_nr - a2i(a));
  else
    return a2i(a);
}

unsigned
i2target(int ms_bit_nr,
	 unsigned bit)
{
  if (ms_bit_nr)
    return ms_bit_nr - bit;
  else
    return bit;
}


