/* Low level interface to ptrace, for GDB when running under Unix.
   Copyright (C) 1986, 1987, 1989 Free Software Foundation, Inc.

This file is part of GDB.

GDB is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GDB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GDB; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifdef __GNUC__
/* Bad implement execle(3). It's depend for "/bin/cc".

   main()
   {
     printf("execle:\n");
     execle(FILE, ARGS, envp);
     exit(1);
   }

   GCC:
   link a6,#0
   pea LC5	; call printf
   jbsr _printf
   ;		; (not popd stack)
   pea _envp	; call execle
   clrl sp@-
   pea LC4
   pea LC4
   pea LC4
   pea LC3
   pea LC6
   jbsr _execle
   addw #32,sp	; delayed pop !!

   /bin/cc:
   link.l	fp,#L23
   movem.l	#L24,(sp)
   pea	L26		; call printf
   jbsr	_printf
   addq.l	#4,sp	; <--- popd stack !!
   pea	_envp		; call execle
   clr.l	-(sp)
   pea	L32
   
   */

execle(name, args)
     char *name, *args;
{
  register char	**env = &args;
  while (*env++)
    ;
  execve(name, (char **)&args, (char **)*env);
}
#endif
