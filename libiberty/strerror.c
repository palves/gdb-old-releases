/* Many systems don't have strerror, but some do.  This simulates
   it for systems that have sys_errlist instead.  */
 
extern int sys_nerr;
extern char *sys_errlist[];

char *
strerror (code)
     int code;
{
  return (((code < 0) || (code >= sys_nerr)) ? "(unknown error)" :
          sys_errlist [code]);
}
