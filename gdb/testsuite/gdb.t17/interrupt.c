int
main ()
{
  char x;
  int nbytes;
  printf ("talk to me baby\n");
  while (1)
    {
      nbytes = read (0, &x, 1);
      if (nbytes < 0)
	perror ("");
      else if (nbytes == 0)
	{
	  printf ("end of file\n");
	  exit (0);
	}
      else
	write (1, &x, 1);
    }
}

int
func1 ()
{
  return 4;
}
