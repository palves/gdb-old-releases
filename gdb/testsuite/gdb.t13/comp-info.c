/* This needs to be kept in sync with gdbme.c.  If this proves to end up
   being hairy, we could use a common header file.  */
#if defined (__STDC__)
set signed_keyword_not_used 0
#else
set signed_keyword_not_used 1
#endif

#if defined (__GNUC__)
set gcc_compiled __GNUC__
#else
set gcc_compiled 0
#endif

return 0
