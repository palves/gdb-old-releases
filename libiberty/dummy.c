#ifndef __STDC__
#define void int
#endif

#define DEF(NAME, RETURN_TYPE, ARGS) extern RETURN_TYPE NAME ();
#include "functions.def"

/* Always use our: getopt.o getopt1.o obstack.o spaces.o */


main (argc, argv)
     int argc; char **argv;
{

#undef DEF
#define DEF(NAME, RETURN_TYPE, ARGS) NAME ();
#include "functions.def"
}
