/* chew
   Copyright (C) 1990-1991 Free Software Foundation, Inc.
   Contributed by steve chamberlain @cygnus

This file is part of BFD, the Binary File Descriptor library.

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

/*
 Yet another way of extracting documentation from source.
 No, I haven't finished it yet, but I hope you people like it better
 that the old way
  
 sac
*/



#include <ansidecl.h>
#include "sysdep.h"

#define DEF_SIZE 5000

char *EXFUN(malloc,(int));
char *EXFUN(realloc,(char *,int));

int internal_wanted;
int internal_mode;



/* Here is a string type ... */

typedef struct buffer 
{
    char *ptr;
    unsigned int write_idx;
    unsigned int size;
} string_type;






static void DEFUN(remchar,(buffer),
	   string_type *buffer)
{
    buffer->write_idx--;    
}

static void DEFUN(init_string_with_size,(buffer, size),
	   string_type *buffer AND
	   unsigned int size )
{
    buffer->write_idx = 0;
    buffer->size = size;
    buffer->ptr = malloc(size);
}

static void DEFUN(init_string,(buffer),
	   string_type *buffer)
{
    init_string_with_size(buffer, DEF_SIZE);

}

static int DEFUN(find, (str, what),
	  string_type *str AND
	  char *what)
{
    unsigned int i;
    char *p;
    p = what;
    for (i = 0; i < str->write_idx && *p; i++) 
    {
	if (*p == str->ptr[i])
	 p++;
	else
	 p = what;
    }
    return (*p == 0);
    
}

static void DEFUN(write_buffer,(buffer),
	   string_type *buffer)
{
    fwrite(buffer->ptr, buffer->write_idx, 1, stdout);
}


static void DEFUN(delete_string,(buffer),
	   string_type *buffer)
{
    free(buffer->ptr);
}


static char *DEFUN(addr, (buffer, idx),
	    string_type *buffer AND
	    unsigned int idx)
{
    return buffer->ptr + idx;
}

static char DEFUN(at,(buffer, pos),
	   string_type *buffer AND
	   unsigned int pos) 
{
    if ( pos >= buffer->write_idx) 
    {
	return 0;
    }
    return buffer->ptr[pos];
}

static void DEFUN(catchar,(buffer, ch), 
	   string_type *buffer AND
	   char ch)
{
    if (buffer->write_idx == buffer->size) 
    {
	buffer->size *=2;
	buffer->ptr = realloc(buffer->ptr, buffer->size);
    }

    buffer->ptr[buffer->write_idx ++ ] = ch;
}


static void DEFUN(overwrite_string,(dst,   src),
	   string_type *dst AND
	   string_type *src)
{
    free(dst->ptr);
    dst->size = src->size;
    dst->write_idx = src->write_idx;
    dst->ptr = src->ptr;
}

static void DEFUN(catstr,(dst, src),
	   string_type *dst AND
	   string_type *src)
{
    unsigned int i;
    for (i = 0; i < src->write_idx; i++) 
    {
	catchar(dst, src->ptr[i]);
    }
}


static void DEFUN(cattext,(buffer, string),
	   string_type *buffer AND
	   char *string)
{
    
    while (*string) 
    {
	catchar(buffer, *string);
	string++;
    }
}

static void DEFUN(catbuf,(buffer, buf, len),
	   string_type *buffer AND
	   char *buf AND
	   unsigned int len)
{
    
    while (len--) 
    {
	catchar(buffer, *buf);
	buf++;
    }
}



static unsigned int 
DEFUN(skip_white_and_stars,(src, idx),
      string_type *src AND
      unsigned int idx)
{
    while (isspace(at(src,idx)) 
	   || (at(src,idx) == '*' && at(src,idx +1) !='/')) 
     idx++;
    return idx;
    

}
/***********************************************************************/


string_type stack[20];
string_type *tos;


   
/* This function removes everything not inside comments starting on
   the first char of the line from the  string, also when copying
   comments, removes blank space and leading *'s 
   Blank lines are turned into one blank line
 */

static void 
DEFUN(remove_noncomments,(src,dst),
	   string_type *src AND
	   string_type *dst)
{
    unsigned int idx = 0;
    
    while (at(src,idx)) 
    {
	/* Now see if we have a comment at the start of the line */
	if (at(src,idx) == '\n' 
	    && at(src,idx+1) ==  '/' 
	    && at(src,idx+2) == '*') 
	{
	    idx+=3;
	    
	    idx = skip_white_and_stars(src,idx);
	    /* Copy to the end of the line, or till the end of the
	       comment */
	    while (at(src, idx))
	    {
		if (at(src, idx) == '\n') 
		{
		    /* end of line, echo and scrape of leading blanks  */
		    if (at(src,idx +1) == '\n')
		     catchar(dst,'\n');
		    catchar(dst,'\n');
		    idx++;
		    idx =   skip_white_and_stars(src, idx);
		}
		else if (at(src, idx) == '*' && at(src,idx+1) == '/') 
		{
		    idx +=2 ;
		    cattext(dst,"\nENDDD\n");
		    break;
		}
		else 
		{
		    catchar(dst, at(src, idx));
		    idx++;
		}
	    }
	}
	else idx++;
    }
    
}

#define PUSH_TEXT (char *)1
#define	SKIP_PAST_NEWLINE (char *)2
#define	CATSTR (char *)3
#define	COPY_PAST_NEWLINE (char *)4
#define DUP (char *)5
#define REMCHAR (char *)6
#define GET_STUFF_IN_COMMAND (char *)7
#define	DO_FANCY_STUFF (char *)8
#define	BULLETIZE (char *)9
#define	COURIERIZE (char *)10
#define	EXIT (char *)11
#define	SWAP (char *)12
#define OUTPUTDOTS (char *)13
#define EXFUNSTUFF (char *)14
#define MAYBECATSTR (char *)15
#define INTERNALMODE (char *)16
#define TRANSLATECOMMENTS (char *)17
typedef char * command_type;

/* turn foobar name(stuff); into foobar EXFUN(name,(stuff));

 */

static void
DEFUN_VOID(insert_exfun)
{
    unsigned int openp;
    unsigned int fname;
    unsigned int idx;
    string_type out;
    init_string(&out);
    

    /* make sure that it's not already exfuned */
    if(find(tos,"EXFUN") || find(tos,"PROTO")) {
	    catstr(&out,tos);
	}
    else 
    {
	
	/*Find the open paren*/
	for (openp = 0; at(tos, openp) != '('  && at(tos,openp); openp++)
	 ;
	fname = openp;
	/* Step back to the fname */
	fname--;
	while (fname && isspace(at(tos, fname)))
	 fname --;
	while (fname && !isspace(at(tos,fname)) && at(tos,fname) != '*')
	 fname--;

	fname++;
	
	for (idx = 0; idx < fname; idx++) 
	{
	    catchar(&out, at(tos,idx));
	}
    
	cattext(&out,"EXFUN(");
	for (idx = fname; idx < openp; idx++) 
	{
	    catchar(&out, at(tos,idx));
	}
	cattext(&out,", ");
	while (at(tos,idx) && at(tos,idx) !=';') 
	{
	    catchar(&out, at(tos, idx));
	    idx++;
	}
	cattext(&out,");\n");
    }
overwrite_string(tos, &out);    
}



/* turn {*
   and *} into comments */

static void 
DEFUN_VOID(translate_comments)
{
    unsigned int idx = 0;
    string_type out;
    init_string(&out);
    
    while (at(tos, idx)) 
    {
	if (at(tos,idx) == '{' && at(tos,idx+1) =='*') 
	{
	    cattext(&out,"	/*");
	    idx+=2;
	}
	else if (at(tos,idx) == '*' && at(tos,idx+1) =='}') 
	{
	    cattext(&out,"*/");
	    idx+=2;
	}
	else  
	{
	    catchar(&out, at(tos, idx));
	    idx++;
	}
    }


    overwrite_string(tos, &out);
    

}

/* Mod tos so that only lines with leading dots remain */
static void
DEFUN_VOID(outputdots)
{
    unsigned int idx = 0;
    string_type out;
    init_string(&out);
    
    while (at(tos, idx)) 
    {
	if (at(tos, idx) == '\n' && at(tos, idx+1) == '.') 
	{
	    idx += 2;
	    
	    while (at(tos, idx) && at(tos, idx)!='\n')
	    {
		if (at(tos,idx) == '{' && at(tos,idx+1) =='*') 
		{
		    cattext(&out," /*");
		    idx+=2;
		}
		else if (at(tos,idx) == '*' && at(tos,idx+1) =='}') 
		{
		    cattext(&out,"*/");
		    idx+=2;
		}
		else  
		{
		    catchar(&out, at(tos, idx));
		    idx++;
		}
	    }
	    catchar(&out,'\n');
	}
	else 
	{
	    idx++;
	}
    }	

    overwrite_string(tos, &out);
}

/* Find lines starting with . and put example around them on tos */
static void
DEFUN_VOID(courierize)
{
    string_type out;
    unsigned int idx = 0;
    
    init_string(&out);
    
    while (at(tos, idx)) 
    {
	if (at(tos, idx) == '\n' && at(tos, idx +1 ) == '.') 
	{
	    cattext(&out,"\n@example\n");
	    do 
	    {
		idx += 2;
		
		while (at(tos, idx) && at(tos, idx)!='\n')
		{
		    if (at(tos,idx)=='{' && at(tos,idx+1) =='*') 
		    {
			cattext(&out," /*");
			idx+=2;
		    }
		    else if (at(tos,idx)=='*' && at(tos,idx+1) =='}') 
		    {
			cattext(&out,"*/");
			idx+=2;
		    }
	            else if (at(tos,idx) == '{')
		    {
			cattext(&out,"@{");
			idx++;
		    }
	            else if (at(tos,idx) == '}')
		    {
			cattext(&out,"@}");
			idx++;
		    }
		    else 
		    {
			catchar(&out, at(tos, idx));
			idx++;
		    }
		    
		}
		catchar(&out,'\n');
	    }  
	    while (at(tos, idx) == '\n' 
		   && at(tos, idx+1) == '.') ;
	    cattext(&out,"@end example");
	}
	else 
	{    
	    catchar(&out, at(tos, idx));
	    idx++;
	}
    }    

    overwrite_string(tos, &out);

}

/* Finds any lines starting with "o ", if there are any, then turns
   on @itemize @bullet, and @items each of them. Then ends with @end
   itemize, inplace at TOS*/

static void
DEFUN_VOID(bulletize)
{
    unsigned int idx = 0;
    int on = 0;
    string_type out;
    init_string(&out);
    
    while (at(tos, idx)) {
	    if (at(tos, idx) == '\n' &&
		at(tos, idx+1) == 'o' &&
		isspace(at(tos, idx +2)))
	    {
		if (!on) 
		{
		    cattext(&out,"\n@itemize @bullet\n");
		    on = 1;
		    
		}
		cattext(&out,"@item ");
		idx+=3;
	    }
	    else 
	    {
		catchar(&out, at(tos, idx));
		idx++;
		
	    }
	}
    if (on) 
    {
	cattext(&out,"@end itemize\n");
    }	

    delete_string(tos);
    *tos = out;
}

/* Turn <<foo>> into @code{foo} in place at TOS*/
   
static void
DEFUN_VOID(do_fancy_stuff)
{
    unsigned int idx = 0;
    string_type out;
    init_string(&out);
    while (at(tos, idx)) 
    {
	if (at(tos, idx) == '<' 
	    && at(tos, idx+1) == '<'
	    && !isspace(at(tos,idx + 2))) 
	{
	    /* This qualifies as a << startup */
	    idx +=2;
	    cattext(&out,"@code{");
	    while(at(tos,idx) &&
		  at(tos,idx) != '>' )
	    {
		catchar(&out, at(tos, idx));
		idx++;
		
	    }
	    cattext(&out,"}");
	    idx+=2;
	}
	else 
	{
	    catchar(&out, at(tos, idx));
	    idx++;
	}
    }
    delete_string(tos);
    *tos = out;
}
/* A command is all upper case,and alone on a line */
static int 
DEFUN( iscommand,(ptr, idx),
      string_type *ptr AND
      unsigned int idx)
{
    unsigned int len = 0;
    while (at(ptr,idx)) {
	    if (isupper(at(ptr,idx)) || at(ptr,idx) == ' ') 
	    {
	     len++;
	     idx++;
	 }
	    else if(at(ptr,idx) == '\n')
	    {
		if (len >4) return 1;
		return 0;
	    }
	    else return 0;
	}
    return 0;

}

static unsigned int
DEFUN(copy_past_newline,(ptr, idx, dst),
      string_type *ptr AND
      unsigned int idx AND
      string_type *dst)
{
    while (at(ptr, idx) && at(ptr, idx) != '\n') 
    {
	catchar(dst, at(ptr, idx));
	idx++;
	
    }    
    catchar(dst, at(ptr, idx));
    idx++;
    return idx;
    

}

static unsigned int 
DEFUN(get_stuff_in_command,(str, idx, dst),
      string_type *str AND
      unsigned int idx AND
      string_type *dst)
{
    while (at(str, idx)) {
	    if (iscommand(str, idx))  break;
	    idx =   copy_past_newline(str, idx, dst);
	}
    return idx;
    
}

static void
DEFUN_VOID(swap)
{
    string_type t;
    
    t = tos[0];
    tos[0] = tos[-1];
    tos[-1] =t; 
}

static void
DEFUN_VOID(dup)
{
    tos++;
    init_string(tos);
    catstr(tos, tos-1);
}



typedef struct 
{
    char *command;    
    int length;
    command_type *list;
}list_type;




static unsigned int
DEFUN(skip_past_newline,(ptr, idx), 
      string_type *ptr AND
      unsigned int idx)
{
    while (at(ptr,idx) 
	   && at(ptr,idx) != '\n')
     idx++;
    idx++;
    return idx;
}


static unsigned int 
DEFUN(execute_function,(str, idx, list),
      string_type *str AND
      unsigned int idx AND
     command_type *list)
{
    tos = stack;

    
    while (*list)  
    {
	switch ((int)(*list))
	{
	  case TRANSLATECOMMENTS:
	    translate_comments();
	    break;

	  case INTERNALMODE:
	    
	    list++;
	    internal_mode = (int)*list;
	    break;
	    
	  case PUSH_TEXT:
	    tos++;
	    init_string(tos);
	    list++;
	    cattext(tos,*((char **)list));
	    break;
	  case SKIP_PAST_NEWLINE:
	    idx = skip_past_newline(str, idx);
	    break;
	  case OUTPUTDOTS:
	    outputdots();
	    break;
	    
	  case CATSTR:
	    catstr(tos-1, tos);
	    delete_string(tos);
	    tos--;
	    break;
	  case 	COPY_PAST_NEWLINE :
	    tos++;
	    init_string(tos);
	    idx = copy_past_newline(str, idx, tos);
	    break;
	  case SWAP:
	    
	    swap();
	    break;
	  case EXFUNSTUFF:
	    insert_exfun();
	    break;
	  case MAYBECATSTR:
	    /* if in internal mode, and -i then  cat */
	    if (internal_wanted == internal_mode) 
	    {
		catstr(tos-1, tos);
	    }
	    delete_string(tos);
	    tos--;
	    
	    break;		
	   
	  case DUP:
	    dup();
	    break;	    
	  case REMCHAR:
	    remchar(tos);
	    break;
	  case GET_STUFF_IN_COMMAND:
	    tos++;
	    init_string(tos);
	    idx = get_stuff_in_command(str, idx, tos);
	    break;
	  case	DO_FANCY_STUFF:
	    do_fancy_stuff();
	    break;
	  case	BULLETIZE:
	    bulletize();
	    break;
	  case	COURIERIZE:
	    courierize();
	    break;
	    

	}
	list++;	    
	    
    }    
    if (tos != stack) abort();

    return idx;
    
}

static void DEFUN(perform,( list, ptr),
	   list_type *list AND
	   string_type *ptr)
{
    unsigned int idx = 0;
    
    while (at(ptr, idx)) {
	    /* It's worth looking through the command list */
	    if (iscommand(ptr, idx))
	    {
		unsigned int i;
		int found = 0;
		for(i = 0; list[i].command && !found; i++)
		{
		    if (strncmp(list[i].command,
				addr(ptr, idx) ,
				list[i].length)==0) 
		    {
			idx = execute_function(ptr, idx, list[i].list);

			found = 1;
		    }
		}

		if (!found) 
		{
		    char buf[100];
		    unsigned int i;
		    for (i = 0; i < sizeof(buf)-1; i++)
		    {
			if (at(ptr, idx+i) == '\n'
			    ||  at(ptr, idx+i) == 0) break;
			buf[i] = at(ptr, idx+i);
		    }
		    buf[i] = 0;
		    fprintf(stderr,"warning, %s is not recognised\n",  buf);
		    idx = skip_past_newline(ptr, idx);
		}
		
	    }
	    else idx = skip_past_newline(ptr, idx);
	}
}





		

static char * section_list[] =
{
    SKIP_PAST_NEWLINE,
    PUSH_TEXT,
    "@section ",
    CATSTR,
    COPY_PAST_NEWLINE,
    CATSTR,
    0,
 };
static char * subsection_list[] =
{
    SKIP_PAST_NEWLINE,
    PUSH_TEXT,
    "@subsection ",
    CATSTR,
    COPY_PAST_NEWLINE,
    CATSTR,
    0,
 };
static char * subsubsection_list[] =
{
    SKIP_PAST_NEWLINE,
    PUSH_TEXT,
    "@subsubsection ",
    CATSTR,
    COPY_PAST_NEWLINE,
    CATSTR,
    0,
 };
/* turn X into a X b X z */
/* FUNCTION foo->@findex foo @subsubsection {foo} */
  
static char *function_list[] =
{
    PUSH_TEXT,			/* a */
    (command_type)    "@findex ",
    SKIP_PAST_NEWLINE,  
    COPY_PAST_NEWLINE,		
    DUP,			/* a X X */
    PUSH_TEXT,
    (command_type) "@subsubsection @code{", /* a X X b */
    SWAP,			/* a X b X */
    REMCHAR,
    PUSH_TEXT,
    (command_type)    "}\n",	/* a X b X c */
    CATSTR,			/* a X b Xc*/
    CATSTR,			/* a X bXc */
    CATSTR,			/* a XbXc*/
    CATSTR,			/* aXbXc*/
    CATSTR,			/* put onto output */
    0,
};


static char * description_list[] =
{
    SKIP_PAST_NEWLINE,
    GET_STUFF_IN_COMMAND,
    DO_FANCY_STUFF,
    BULLETIZE,
    COURIERIZE,
    CATSTR,
    0,
}   ;

#define TEXT(x) PUSH_TEXT,x
static command_type synopsis_list[] =
{
    SKIP_PAST_NEWLINE,
    TEXT("@example\n"),
    CATSTR,
    GET_STUFF_IN_COMMAND,	/* @example x */
    CATSTR,
    TEXT("@end example\n"),
    CATSTR,
0,
};



command_type enddd_list[] = 
{
SKIP_PAST_NEWLINE,
INTERNALMODE,0,
    0,
};

command_type internal_mode_list[] = 
{
SKIP_PAST_NEWLINE,
INTERNALMODE,(char *)1,
    0,
};

command_type example_list[] =
{
    SKIP_PAST_NEWLINE,
    TEXT("@example\n"),
    CATSTR,
    GET_STUFF_IN_COMMAND,
    TRANSLATECOMMENTS,
    CATSTR,
    TEXT("@end example\n"),
    CATSTR,

    0,
};


list_type doc_stuff[] =
{
    "SECTION",7,section_list,
    "TYPEDEF",7,function_list,
    "SUBSECTION",10,subsection_list,
    "SUBSUBSECTION",13,subsubsection_list,
    "FUNCTION",8,function_list,
    "INTERNAL FUNCTION",17, function_list,
    "DESCRIPTION",11,description_list,
    "EXAMPLE",7,example_list,
    "INTERNAL",8,description_list,
    "RETURNS",7,description_list,
    "SYNOPSIS",8,synopsis_list,
    "ENDDD",5,enddd_list,
    0,
}
;


command_type internal_list[] =
{
    INTERNALMODE,(char*)1,
    SKIP_PAST_NEWLINE,
    GET_STUFF_IN_COMMAND,
    OUTPUTDOTS,
    MAYBECATSTR,

    0,
};

command_type internal_synopsis_list[] =
{
SKIP_PAST_NEWLINE,
GET_STUFF_IN_COMMAND,
EXFUNSTUFF,
MAYBECATSTR,
0
};

command_type internal_description_list[] =
{
SKIP_PAST_NEWLINE,
GET_STUFF_IN_COMMAND,
OUTPUTDOTS,
MAYBECATSTR,
0,
};


list_type internal_stuff[] =
{
    "SECTION",7,enddd_list,
    "SUBSECTION",10,enddd_list,
    "SUBSUBSECTION",13,enddd_list,
    "FUNCTION",8,enddd_list,
    "EXAMPLE",7,enddd_list,
    "DESCRIPTION",11,internal_description_list,
    "INTERNAL FUNCTION",17, internal_mode_list,
    "INTERNAL",8,internal_list,
    "RETURNS",7,enddd_list,
    "TYPEDEF",7,enddd_list,
    "SYNOPSIS",8,internal_synopsis_list,
    "SYNOPSIS",8,internal_synopsis_list,
    "ENDDD",5,enddd_list,
    0,
};





static void DEFUN(read_in, (str), 
	   string_type *str)
{
    char buff[10000];    
    unsigned int r;
    do 
    {
	r = fread(buff, 1, sizeof(buff), stdin);
	catbuf(str, buff, r);
    }
    while (r);
}

static void DEFUN_VOID(usage)
{
    fprintf(stderr,"usage: -[d|i|g] <file >file\n");
    exit(33);    
}

int DEFUN(main,(ac,av),
int ac AND
char *av[])
{

    char *malloc();

    string_type buffer;
    string_type dst;

    init_string(&buffer);
    init_string(&dst);
    init_string(stack+0);
    tos=stack+1;
    
    /* Put a nl at the start */
    catchar(&buffer,'\n');

    read_in(&buffer); 
    remove_noncomments(&buffer, &dst);
    if(ac == 2 && av[1][0] == '-') 
    {
	switch (av[1][1]) 
	{
	  case 'd':
	    perform(doc_stuff, &dst);
	    break;
	  case 'i':
	    internal_wanted = 1;
	    perform(internal_stuff, &dst);
	    break;
	  case 'g':
	    internal_wanted =0;
	    perform(internal_stuff, &dst);
	    break;
	  default:
	    usage();
	}
    }
    else 
    {
	usage();
      
    }
    write_buffer(stack+0);
    return 0;
}



