/* front end to the simulator.

   Written by Steve Chamberlain of Cygnus Support.
   sac@cygnus.com

   This file is part of H8/300 sim


		THIS SOFTWARE IS NOT COPYRIGHTED

   Cygnus offers the following for use in the public domain.  Cygnus
   makes no warranty with regard to the software or it's performance
   and the user accepts the software "AS IS" with all faults.

   CYGNUS DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD TO
   THIS SOFTWARE INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

*/

#include "bfd.h"
#include "sysdep.h"

int
main(ac,av)
int ac;
char **av;
{
  bfd *abfd;
  bfd_vma start_address;
  asection *s;
  int i;
  int verbose = 0;
  int trace = 0;
  char *name = "";
  for (i = 1; i < ac; i++)
  {
    if (strcmp(av[i],"-v") == 0) 
    {
      verbose = 1;
    }
    else if (strcmp(av[i],"-t") == 0) 
    {
      trace = 1;
    }

    else 
    {
      name = av[i];
    }
  }
  if (verbose)
  {
    printf("run %s\n", name);
  }
  abfd = bfd_openr(name,"coff-h8300");
  if (abfd) {
      
    if (bfd_check_format(abfd, bfd_object)) 
    {

      for (s = abfd->sections; s; s=s->next) 
      {
	char *buffer = malloc(bfd_section_size(abfd,s));
	bfd_get_section_contents(abfd, s, buffer, 0, bfd_section_size(abfd,s));
	sim_write(s->vma, buffer, bfd_section_size(abfd,s));
      }

      start_address = bfd_get_start_address(abfd);
      sim_store_register(
			 9,start_address);
      sim_resume(0,0);
      return 0;
    }
  }

  return 1;
}
