
/*  A Bison parser, made from ./c-exp.y  */

#define	INT	258
#define	CHAR	259
#define	UINT	260
#define	FLOAT	261
#define	STRING	262
#define	NAME	263
#define	TYPENAME	264
#define	NAME_OR_INT	265
#define	NAME_OR_UINT	266
#define	STRUCT	267
#define	UNION	268
#define	ENUM	269
#define	SIZEOF	270
#define	UNSIGNED	271
#define	COLONCOLON	272
#define	ERROR	273
#define	SIGNED	274
#define	LONG	275
#define	SHORT	276
#define	INT_KEYWORD	277
#define	LAST	278
#define	REGNAME	279
#define	VARIABLE	280
#define	ASSIGN_MODIFY	281
#define	THIS	282
#define	ABOVE_COMMA	283
#define	OR	284
#define	AND	285
#define	EQUAL	286
#define	NOTEQUAL	287
#define	LEQ	288
#define	GEQ	289
#define	LSH	290
#define	RSH	291
#define	UNARY	292
#define	INCREMENT	293
#define	DECREMENT	294
#define	ARROW	295
#define	BLOCKNAME	296

#line 29 "./c-exp.y"


#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "param.h"
#include "symtab.h"
#include "frame.h"
#include "expression.h"
#include "parser-defs.h"
#include "value.h"
#include "language.h"

/* These MUST be included in any grammar file!!!! 
   Please choose unique names! */
#define	yyparse	c_parse
#define	yylex	c_lex
#define	yyerror	c_error
#define	yylval	c_lval
#define	yychar	c_char
#define	yydebug	c_debug
#define	yypact	c_pact	
#define	yyr1	c_r1			
#define	yyr2	c_r2			
#define	yydef	c_def		
#define	yychk	c_chk		
#define	yypgo	c_pgo		
#define	yyact	c_act		
#define	yyexca	c_exca
#define yyerrflag c_errflag
#define yynerrs	c_nerrs
#define	yyps	c_ps
#define	yypv	c_pv
#define	yys	c_s
#define	yystate	c_state
#define	yytmp	c_tmp
#define	yyv	c_v
#define	yyval	c_val
#define	yylloc	c_lloc

/* Forward decls */
void yyerror ();
static int parse_number ();
int yyparse ();

/* #define	YYDEBUG	1 */


#line 82 "./c-exp.y"
typedef union
  {
    LONGEST lval;
    unsigned LONGEST ulval;
    double dval;
    struct symbol *sym;
    struct type *tval;
    struct stoken sval;
    struct ttype tsym;
    struct symtoken ssym;
    int voidval;
    struct block *bval;
    enum exp_opcode opcode;
    struct internalvar *ivar;

    struct type **tvec;
    int *ivec;
  } YYSTYPE;

#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype
#endif

#include <stdio.h>

#ifndef __STDC__
#define const
#endif



#define	YYFINAL		199
#define	YYFLAG		-32768
#define	YYNTBASE	66

#define YYTRANSLATE(x) ((unsigned)(x) <= 296 ? yytranslate[x] : 85)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    59,     2,     2,     2,    50,    36,     2,    57,
    62,    48,    46,    28,    47,    55,    49,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    65,     2,    39,
    30,    40,    31,    45,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    56,     2,    61,    35,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    63,    34,    64,    60,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    29,    32,    33,    37,    38,    41,    42,    43,
    44,    51,    52,    53,    54,    58
};

static const short yyprhs[] = {     0,
     0,     2,     4,     6,     8,    12,    15,    18,    21,    24,
    27,    30,    33,    36,    39,    42,    46,    51,    55,    60,
    65,    66,    72,    73,    75,    79,    84,    89,    93,    97,
   101,   105,   109,   113,   117,   121,   125,   129,   133,   137,
   141,   145,   149,   153,   157,   161,   165,   169,   175,   179,
   183,   185,   187,   189,   191,   193,   195,   197,   199,   201,
   203,   208,   210,   212,   214,   218,   222,   226,   231,   234,
   236,   238,   241,   243,   246,   248,   251,   253,   257,   260,
   262,   265,   267,   270,   274,   277,   279,   283,   290,   299,
   309,   311,   313,   315,   317,   320,   324,   327,   331,   335,
   340,   343,   347,   350,   353,   356,   359,   361,   364,   366,
   368,   370,   372,   374,   376,   380,   382,   384,   386,   388,
   390,   392
};

static const short yyrhs[] = {    68,
     0,    67,     0,    79,     0,    69,     0,    68,    28,    69,
     0,    48,    69,     0,    36,    69,     0,    47,    69,     0,
    59,    69,     0,    60,    69,     0,    52,    69,     0,    53,
    69,     0,    69,    52,     0,    69,    53,     0,    15,    69,
     0,    69,    54,    83,     0,    69,    54,    48,    69,     0,
    69,    55,    83,     0,    69,    55,    48,    69,     0,    69,
    56,    68,    61,     0,     0,    69,    57,    70,    71,    62,
     0,     0,    69,     0,    71,    28,    69,     0,    63,    79,
    64,    69,     0,    57,    79,    62,    69,     0,    57,    68,
    62,     0,    69,    45,    69,     0,    69,    48,    69,     0,
    69,    49,    69,     0,    69,    50,    69,     0,    69,    46,
    69,     0,    69,    47,    69,     0,    69,    43,    69,     0,
    69,    44,    69,     0,    69,    37,    69,     0,    69,    38,
    69,     0,    69,    41,    69,     0,    69,    42,    69,     0,
    69,    39,    69,     0,    69,    40,    69,     0,    69,    36,
    69,     0,    69,    35,    69,     0,    69,    34,    69,     0,
    69,    33,    69,     0,    69,    32,    69,     0,    69,    31,
    69,    65,    69,     0,    69,    30,    69,     0,    69,    26,
    69,     0,     3,     0,    10,     0,     5,     0,    11,     0,
     4,     0,     6,     0,    73,     0,    23,     0,    24,     0,
    25,     0,    15,    57,    79,    62,     0,     7,     0,    27,
     0,    58,     0,    72,    17,    83,     0,    72,    17,    83,
     0,    80,    17,    83,     0,    80,    17,    60,    83,     0,
    17,    83,     0,    84,     0,    80,     0,    80,    75,     0,
    48,     0,    48,    75,     0,    36,     0,    36,    75,     0,
    76,     0,    57,    75,    62,     0,    76,    77,     0,    77,
     0,    76,    78,     0,    78,     0,    56,    61,     0,    56,
     3,    61,     0,    57,    62,     0,    74,     0,    80,    17,
    48,     0,    79,    57,    80,    17,    48,    62,     0,    79,
    57,    80,    17,    48,    62,    57,    62,     0,    79,    57,
    80,    17,    48,    62,    57,    82,    62,     0,     9,     0,
    22,     0,    20,     0,    21,     0,    20,    22,     0,    16,
    20,    22,     0,    20,    20,     0,    20,    20,    22,     0,
    16,    20,    20,     0,    16,    20,    20,    22,     0,    21,
    22,     0,    16,    21,    22,     0,    12,    83,     0,    13,
    83,     0,    14,    83,     0,    16,    81,     0,    16,     0,
    19,    81,     0,    19,     0,     9,     0,    22,     0,    20,
     0,    21,     0,    79,     0,    82,    28,    79,     0,     8,
     0,    58,     0,     9,     0,    10,     0,    11,     0,     8,
     0,    58,     0
};

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   176,   177,   180,   187,   188,   193,   196,   199,   203,   207,
   211,   215,   219,   223,   227,   231,   237,   241,   247,   251,
   255,   259,   265,   268,   272,   276,   282,   288,   294,   298,
   302,   306,   310,   314,   318,   322,   326,   330,   334,   338,
   342,   346,   350,   354,   358,   362,   366,   370,   374,   378,
   384,   394,   407,   419,   432,   439,   446,   449,   455,   461,
   467,   474,   481,   488,   506,   516,   529,   542,   560,   606,
   700,   701,   736,   738,   740,   742,   744,   747,   749,   754,
   760,   762,   766,   768,   772,   776,   777,   779,   781,   784,
   791,   793,   795,   797,   799,   801,   803,   805,   807,   809,
   811,   813,   815,   818,   821,   824,   826,   828,   830,   834,
   835,   841,   847,   856,   861,   868,   869,   870,   871,   872,
   875,   876
};

static const char * const yytname[] = {   "$",
"error","$illegal.","INT","CHAR","UINT","FLOAT","STRING","NAME","TYPENAME","NAME_OR_INT",
"NAME_OR_UINT","STRUCT","UNION","ENUM","SIZEOF","UNSIGNED","COLONCOLON","ERROR","SIGNED","LONG",
"SHORT","INT_KEYWORD","LAST","REGNAME","VARIABLE","ASSIGN_MODIFY","THIS","','","ABOVE_COMMA","'='",
"'?'","OR","AND","'|'","'^'","'&'","EQUAL","NOTEQUAL","'<'","'>'",
"LEQ","GEQ","LSH","RSH","'@'","'+'","'-'","'*'","'/'","'%'",
"UNARY","INCREMENT","DECREMENT","ARROW","'.'","'['","'('","BLOCKNAME","'!'","'~'",
"']'","')'","'{'","'}'","':'","start","type_exp","exp1","exp","@1",
"arglist","block","variable","ptype","abs_decl","direct_abs_decl","array_mod","func_mod","type","typebase",
"typename","nonempty_typelist","name","name_not_typename",""
};
#endif

static const short yyr1[] = {     0,
    66,    66,    67,    68,    68,    69,    69,    69,    69,    69,
    69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
    70,    69,    71,    71,    71,    69,    69,    69,    69,    69,
    69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
    69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
    69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
    69,    69,    69,    72,    72,    73,    73,    73,    73,    73,
    74,    74,    75,    75,    75,    75,    75,    76,    76,    76,
    76,    76,    77,    77,    78,    79,    79,    79,    79,    79,
    80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
    80,    80,    80,    80,    80,    80,    80,    80,    80,    81,
    81,    81,    81,    82,    82,    83,    83,    83,    83,    83,
    84,    84
};

static const short yyr2[] = {     0,
     1,     1,     1,     1,     3,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     3,     4,     3,     4,     4,
     0,     5,     0,     1,     3,     4,     4,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     5,     3,     3,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     4,     1,     1,     1,     3,     3,     3,     4,     2,     1,
     1,     2,     1,     2,     1,     2,     1,     3,     2,     1,
     2,     1,     2,     3,     2,     1,     3,     6,     8,     9,
     1,     1,     1,     1,     2,     3,     2,     3,     3,     4,
     2,     3,     2,     2,     2,     2,     1,     2,     1,     1,
     1,     1,     1,     1,     3,     1,     1,     1,     1,     1,
     1,     1
};

static const short yydefact[] = {     0,
    51,    55,    53,    56,    62,   121,    91,    52,    54,     0,
     0,     0,     0,   107,     0,   109,    93,    94,    92,    58,
    59,    60,    63,     0,     0,     0,     0,     0,     0,   122,
     0,     0,     0,     2,     1,     4,     0,    57,    86,     3,
    71,    70,   116,   118,   119,   120,   117,   103,   104,   105,
     0,    15,     0,   110,   112,   113,   111,   106,    69,   112,
   113,   108,    97,    95,   101,     7,     8,     6,    11,    12,
     0,     0,     9,    10,     0,    71,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
    14,     0,     0,     0,    21,     0,     0,     0,    75,    73,
     0,     0,    72,    77,    80,    82,     0,     0,    99,    96,
   102,    98,    28,     0,     0,     0,     5,    50,    49,     0,
    47,    46,    45,    44,    43,    37,    38,    41,    42,    39,
    40,    35,    36,    29,    33,    34,    30,    31,    32,     0,
    16,     0,    18,     0,    23,    66,     0,    87,     0,    67,
    76,    74,     0,    83,    85,     0,     0,    79,    81,    61,
   100,    27,    26,     0,    17,    19,    20,    24,     0,     0,
    68,    84,    78,    48,     0,    22,     0,    25,    88,     0,
    89,   114,     0,     0,    90,   115,     0,     0,     0
};

static const short yydefgoto[] = {   197,
    34,    71,    36,   155,   179,    37,    38,    39,   113,   114,
   115,   116,    40,    53,    58,   193,   160,    42
};

static const short yypact[] = {   112,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   168,
   168,   168,   192,     1,   168,   133,    71,    -5,-32768,-32768,
-32768,-32768,-32768,   112,   112,   112,   112,   112,   112,    11,
   112,   112,   273,-32768,     6,   335,    22,-32768,-32768,   -11,
   266,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   112,   132,    72,-32768,    81,    83,-32768,-32768,-32768,-32768,
-32768,-32768,    87,-32768,-32768,   132,   132,   132,   132,   132,
   -19,   -31,   132,   132,   -39,   271,   112,   112,   112,   112,
   112,   112,   112,   112,   112,   112,   112,   112,   112,   112,
   112,   112,   112,   112,   112,   112,   112,   112,   112,-32768,
-32768,    52,   172,   112,-32768,   168,   273,    44,   243,   243,
    27,   185,-32768,    88,-32768,-32768,   -30,    48,    92,-32768,
-32768,-32768,-32768,   112,   112,    46,   335,   335,   335,   299,
   387,   411,   434,   456,   477,   496,   496,   511,   511,   511,
   511,   524,   524,   536,   546,   546,   132,   132,   132,   112,
-32768,   112,-32768,    69,   112,   124,   129,-32768,   168,-32768,
-32768,-32768,    86,-32768,-32768,    89,    94,-32768,-32768,   253,
-32768,   132,   132,   112,   132,   132,-32768,   335,   -17,   101,
-32768,-32768,-32768,   362,   112,-32768,    95,   335,    93,    28,
-32768,   -11,    36,   273,-32768,   -11,   161,   166,-32768
};

static const short yypgoto[] = {-32768,
-32768,     7,   -12,-32768,-32768,-32768,-32768,-32768,   -74,-32768,
    54,    60,   -27,     0,   175,-32768,    -7,-32768
};


#define	YYLAST		603


static const short yytable[] = {    41,
    52,    72,    48,    49,    50,    75,    35,    59,    77,    54,
   185,    66,    67,    68,    69,    70,    65,   107,    73,    74,
    55,    56,    57,   117,   125,   107,   107,   -64,    41,   163,
   124,   170,    76,    77,   161,   162,     7,   166,   106,    10,
    11,    12,   123,    14,   186,   107,    16,    17,    18,    19,
    41,    43,    44,    45,    46,    43,    44,    45,    46,    43,
    44,    45,    46,   194,   127,   128,   129,   130,   131,   132,
   133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
   143,   144,   145,   146,   147,   148,   149,   164,   118,   191,
    63,   158,    64,   158,   151,   153,    77,   195,   156,   150,
   119,    47,   120,   159,   121,    47,   157,   159,   122,    47,
   154,   172,   173,   171,     1,     2,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,   177,
    16,    17,    18,    19,    20,    21,    22,   175,    23,   176,
   -65,    54,   178,   111,   167,   180,   182,    24,   187,   190,
   183,   181,    60,    61,    57,   165,   189,   172,    25,    26,
   198,   184,   192,    27,    28,   199,   196,   168,    29,    30,
    31,    32,   188,   169,    33,    43,    44,    45,    46,    43,
    44,    45,    46,   100,   101,   102,   103,   104,   105,    76,
    62,     0,     0,    76,     1,     2,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,     0,
    16,    17,    18,    19,    20,    21,    22,     0,    23,   152,
   109,     0,     0,     0,     0,    47,     0,    24,     0,    47,
     0,     0,   110,     0,     0,     0,     0,     0,    25,    26,
   111,   112,     0,    27,    28,     0,   165,     0,    51,    30,
    31,    32,     0,     0,    33,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
     0,    16,    17,    18,    19,    20,    21,    22,   109,    23,
     0,     7,   108,     0,    10,    11,    12,   126,    14,     0,
   110,    16,    17,    18,    19,     0,     0,     0,   111,   112,
     0,   109,     0,     0,    27,    28,   109,     0,     0,    29,
    30,    31,    32,   110,     0,    33,     0,     0,   110,     0,
     0,   111,   112,     0,    78,     0,   111,   112,    79,    80,
    81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
    91,    92,    93,    94,    95,    96,    97,    98,    99,     0,
   100,   101,   102,   103,   104,   105,     0,     0,     0,     0,
    78,     0,     0,   174,    79,    80,    81,    82,    83,    84,
    85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
    95,    96,    97,    98,    99,     0,   100,   101,   102,   103,
   104,   105,    80,    81,    82,    83,    84,    85,    86,    87,
    88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
    98,    99,     0,   100,   101,   102,   103,   104,   105,    82,
    83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
    93,    94,    95,    96,    97,    98,    99,     0,   100,   101,
   102,   103,   104,   105,    83,    84,    85,    86,    87,    88,
    89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
    99,     0,   100,   101,   102,   103,   104,   105,    84,    85,
    86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
    96,    97,    98,    99,     0,   100,   101,   102,   103,   104,
   105,    85,    86,    87,    88,    89,    90,    91,    92,    93,
    94,    95,    96,    97,    98,    99,     0,   100,   101,   102,
   103,   104,   105,    86,    87,    88,    89,    90,    91,    92,
    93,    94,    95,    96,    97,    98,    99,     0,   100,   101,
   102,   103,   104,   105,    88,    89,    90,    91,    92,    93,
    94,    95,    96,    97,    98,    99,     0,   100,   101,   102,
   103,   104,   105,    92,    93,    94,    95,    96,    97,    98,
    99,     0,   100,   101,   102,   103,   104,   105,    94,    95,
    96,    97,    98,    99,     0,   100,   101,   102,   103,   104,
   105,    95,    96,    97,    98,    99,     0,   100,   101,   102,
   103,   104,   105,    97,    98,    99,     0,   100,   101,   102,
   103,   104,   105
};

static const short yycheck[] = {     0,
    13,    29,    10,    11,    12,    33,     0,    15,    28,     9,
    28,    24,    25,    26,    27,    28,    22,    57,    31,    32,
    20,    21,    22,    51,    64,    57,    57,    17,    29,     3,
    62,    62,    33,    28,   109,   110,     9,   112,    17,    12,
    13,    14,    62,    16,    62,    57,    19,    20,    21,    22,
    51,     8,     9,    10,    11,     8,     9,    10,    11,     8,
     9,    10,    11,    28,    77,    78,    79,    80,    81,    82,
    83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
    93,    94,    95,    96,    97,    98,    99,    61,    17,    62,
    20,    48,    22,    48,   102,   103,    28,    62,   106,    48,
    20,    58,    22,    60,    22,    58,   107,    60,    22,    58,
   104,   124,   125,    22,     3,     4,     5,     6,     7,     8,
     9,    10,    11,    12,    13,    14,    15,    16,    17,    61,
    19,    20,    21,    22,    23,    24,    25,   150,    27,   152,
    17,     9,   155,    56,    57,    17,    61,    36,    48,    57,
    62,   159,    20,    21,    22,    62,    62,   170,    47,    48,
     0,   174,   190,    52,    53,     0,   194,   114,    57,    58,
    59,    60,   185,   114,    63,     8,     9,    10,    11,     8,
     9,    10,    11,    52,    53,    54,    55,    56,    57,   190,
    16,    -1,    -1,   194,     3,     4,     5,     6,     7,     8,
     9,    10,    11,    12,    13,    14,    15,    16,    17,    -1,
    19,    20,    21,    22,    23,    24,    25,    -1,    27,    48,
    36,    -1,    -1,    -1,    -1,    58,    -1,    36,    -1,    58,
    -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,    47,    48,
    56,    57,    -1,    52,    53,    -1,    62,    -1,    57,    58,
    59,    60,    -1,    -1,    63,     3,     4,     5,     6,     7,
     8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
    -1,    19,    20,    21,    22,    23,    24,    25,    36,    27,
    -1,     9,    17,    -1,    12,    13,    14,    17,    16,    -1,
    48,    19,    20,    21,    22,    -1,    -1,    -1,    56,    57,
    -1,    36,    -1,    -1,    52,    53,    36,    -1,    -1,    57,
    58,    59,    60,    48,    -1,    63,    -1,    -1,    48,    -1,
    -1,    56,    57,    -1,    26,    -1,    56,    57,    30,    31,
    32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
    42,    43,    44,    45,    46,    47,    48,    49,    50,    -1,
    52,    53,    54,    55,    56,    57,    -1,    -1,    -1,    -1,
    26,    -1,    -1,    65,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
    46,    47,    48,    49,    50,    -1,    52,    53,    54,    55,
    56,    57,    31,    32,    33,    34,    35,    36,    37,    38,
    39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
    49,    50,    -1,    52,    53,    54,    55,    56,    57,    33,
    34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
    44,    45,    46,    47,    48,    49,    50,    -1,    52,    53,
    54,    55,    56,    57,    34,    35,    36,    37,    38,    39,
    40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
    50,    -1,    52,    53,    54,    55,    56,    57,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49,    50,    -1,    52,    53,    54,    55,    56,
    57,    36,    37,    38,    39,    40,    41,    42,    43,    44,
    45,    46,    47,    48,    49,    50,    -1,    52,    53,    54,
    55,    56,    57,    37,    38,    39,    40,    41,    42,    43,
    44,    45,    46,    47,    48,    49,    50,    -1,    52,    53,
    54,    55,    56,    57,    39,    40,    41,    42,    43,    44,
    45,    46,    47,    48,    49,    50,    -1,    52,    53,    54,
    55,    56,    57,    43,    44,    45,    46,    47,    48,    49,
    50,    -1,    52,    53,    54,    55,    56,    57,    45,    46,
    47,    48,    49,    50,    -1,    52,    53,    54,    55,    56,
    57,    46,    47,    48,    49,    50,    -1,    52,    53,    54,
    55,    56,    57,    48,    49,    50,    -1,    52,    53,    54,
    55,    56,    57
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/latest/lib/bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Bob Corbett and Richard Stallman

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */


#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* Not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__)
#include <alloca.h>
#else /* Not sparc */
#ifdef MSDOS
#include <malloc.h>
#endif /* MSDOS */
#endif /* Not sparc.  */
#endif /* Not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#define YYLEX		yylex(&yylval, &yylloc)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_bcopy (from, to, count)
     char *from;
     char *to;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_bcopy (char *from, char *to, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif

#line 160 "/usr/latest/lib/bison.simple"
int
yyparse()
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/

#define YYPOPSTACK   (yyvsp--, yysp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yysp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
#ifdef YYLSP_NEEDED
		 &yyls1, size * sizeof (*yylsp),
#endif
		 &yystacksize);

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Next token is %d (%s)\n", yychar, yytname[yychar1]);
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symboles being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 3:
#line 181 "./c-exp.y"
{ write_exp_elt_opcode(OP_TYPE);
			  write_exp_elt_type(yyvsp[0].tval);
			  write_exp_elt_opcode(OP_TYPE);;
    break;}
case 5:
#line 189 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_COMMA); ;
    break;}
case 6:
#line 194 "./c-exp.y"
{ write_exp_elt_opcode (UNOP_IND); ;
    break;}
case 7:
#line 197 "./c-exp.y"
{ write_exp_elt_opcode (UNOP_ADDR); ;
    break;}
case 8:
#line 200 "./c-exp.y"
{ write_exp_elt_opcode (UNOP_NEG); ;
    break;}
case 9:
#line 204 "./c-exp.y"
{ write_exp_elt_opcode (UNOP_ZEROP); ;
    break;}
case 10:
#line 208 "./c-exp.y"
{ write_exp_elt_opcode (UNOP_LOGNOT); ;
    break;}
case 11:
#line 212 "./c-exp.y"
{ write_exp_elt_opcode (UNOP_PREINCREMENT); ;
    break;}
case 12:
#line 216 "./c-exp.y"
{ write_exp_elt_opcode (UNOP_PREDECREMENT); ;
    break;}
case 13:
#line 220 "./c-exp.y"
{ write_exp_elt_opcode (UNOP_POSTINCREMENT); ;
    break;}
case 14:
#line 224 "./c-exp.y"
{ write_exp_elt_opcode (UNOP_POSTDECREMENT); ;
    break;}
case 15:
#line 228 "./c-exp.y"
{ write_exp_elt_opcode (UNOP_SIZEOF); ;
    break;}
case 16:
#line 232 "./c-exp.y"
{ write_exp_elt_opcode (STRUCTOP_PTR);
			  write_exp_string (yyvsp[0].sval);
			  write_exp_elt_opcode (STRUCTOP_PTR); ;
    break;}
case 17:
#line 238 "./c-exp.y"
{ write_exp_elt_opcode (STRUCTOP_MPTR); ;
    break;}
case 18:
#line 242 "./c-exp.y"
{ write_exp_elt_opcode (STRUCTOP_STRUCT);
			  write_exp_string (yyvsp[0].sval);
			  write_exp_elt_opcode (STRUCTOP_STRUCT); ;
    break;}
case 19:
#line 248 "./c-exp.y"
{ write_exp_elt_opcode (STRUCTOP_MEMBER); ;
    break;}
case 20:
#line 252 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_SUBSCRIPT); ;
    break;}
case 21:
#line 258 "./c-exp.y"
{ start_arglist (); ;
    break;}
case 22:
#line 260 "./c-exp.y"
{ write_exp_elt_opcode (OP_FUNCALL);
			  write_exp_elt_longcst ((LONGEST) end_arglist ());
			  write_exp_elt_opcode (OP_FUNCALL); ;
    break;}
case 24:
#line 269 "./c-exp.y"
{ arglist_len = 1; ;
    break;}
case 25:
#line 273 "./c-exp.y"
{ arglist_len++; ;
    break;}
case 26:
#line 277 "./c-exp.y"
{ write_exp_elt_opcode (UNOP_MEMVAL);
			  write_exp_elt_type (yyvsp[-2].tval);
			  write_exp_elt_opcode (UNOP_MEMVAL); ;
    break;}
case 27:
#line 283 "./c-exp.y"
{ write_exp_elt_opcode (UNOP_CAST);
			  write_exp_elt_type (yyvsp[-2].tval);
			  write_exp_elt_opcode (UNOP_CAST); ;
    break;}
case 28:
#line 289 "./c-exp.y"
{ ;
    break;}
case 29:
#line 295 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_REPEAT); ;
    break;}
case 30:
#line 299 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_MUL); ;
    break;}
case 31:
#line 303 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_DIV); ;
    break;}
case 32:
#line 307 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_REM); ;
    break;}
case 33:
#line 311 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_ADD); ;
    break;}
case 34:
#line 315 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_SUB); ;
    break;}
case 35:
#line 319 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_LSH); ;
    break;}
case 36:
#line 323 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_RSH); ;
    break;}
case 37:
#line 327 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_EQUAL); ;
    break;}
case 38:
#line 331 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_NOTEQUAL); ;
    break;}
case 39:
#line 335 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_LEQ); ;
    break;}
case 40:
#line 339 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_GEQ); ;
    break;}
case 41:
#line 343 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_LESS); ;
    break;}
case 42:
#line 347 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_GTR); ;
    break;}
case 43:
#line 351 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_LOGAND); ;
    break;}
case 44:
#line 355 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_LOGXOR); ;
    break;}
case 45:
#line 359 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_LOGIOR); ;
    break;}
case 46:
#line 363 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_AND); ;
    break;}
case 47:
#line 367 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_OR); ;
    break;}
case 48:
#line 371 "./c-exp.y"
{ write_exp_elt_opcode (TERNOP_COND); ;
    break;}
case 49:
#line 375 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_ASSIGN); ;
    break;}
case 50:
#line 379 "./c-exp.y"
{ write_exp_elt_opcode (BINOP_ASSIGN_MODIFY);
			  write_exp_elt_opcode (yyvsp[-1].opcode);
			  write_exp_elt_opcode (BINOP_ASSIGN_MODIFY); ;
    break;}
case 51:
#line 385 "./c-exp.y"
{ write_exp_elt_opcode (OP_LONG);
			  if (yyvsp[0].lval == (int) yyvsp[0].lval || yyvsp[0].lval == (unsigned int) yyvsp[0].lval)
			    write_exp_elt_type (builtin_type_int);
			  else
			    write_exp_elt_type (BUILTIN_TYPE_LONGEST);
			  write_exp_elt_longcst ((LONGEST) yyvsp[0].lval);
			  write_exp_elt_opcode (OP_LONG); ;
    break;}
case 52:
#line 395 "./c-exp.y"
{ YYSTYPE val;
			  parse_number (yyvsp[0].ssym.stoken.ptr, yyvsp[0].ssym.stoken.length, 0, &val);
			  write_exp_elt_opcode (OP_LONG);
			  if (val.lval == (int) val.lval ||
			      val.lval == (unsigned int) val.lval)
			    write_exp_elt_type (builtin_type_int);
			  else
			    write_exp_elt_type (BUILTIN_TYPE_LONGEST);
			  write_exp_elt_longcst (val.lval);
			  write_exp_elt_opcode (OP_LONG); ;
    break;}
case 53:
#line 408 "./c-exp.y"
{
			  write_exp_elt_opcode (OP_LONG);
			  if (yyvsp[0].ulval == (unsigned int) yyvsp[0].ulval)
			    write_exp_elt_type (builtin_type_unsigned_int);
			  else
			    write_exp_elt_type (BUILTIN_TYPE_UNSIGNED_LONGEST);
			  write_exp_elt_longcst ((LONGEST) yyvsp[0].ulval);
			  write_exp_elt_opcode (OP_LONG);
			;
    break;}
case 54:
#line 420 "./c-exp.y"
{ YYSTYPE val;
			  parse_number (yyvsp[0].ssym.stoken.ptr, yyvsp[0].ssym.stoken.length, 0, &val);
			  write_exp_elt_opcode (OP_LONG);
			  if (val.ulval == (unsigned int) val.ulval)
			    write_exp_elt_type (builtin_type_unsigned_int);
			  else
			    write_exp_elt_type (BUILTIN_TYPE_UNSIGNED_LONGEST);
			  write_exp_elt_longcst ((LONGEST)val.ulval);
			  write_exp_elt_opcode (OP_LONG);
			;
    break;}
case 55:
#line 433 "./c-exp.y"
{ write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (builtin_type_char);
			  write_exp_elt_longcst ((LONGEST) yyvsp[0].lval);
			  write_exp_elt_opcode (OP_LONG); ;
    break;}
case 56:
#line 440 "./c-exp.y"
{ write_exp_elt_opcode (OP_DOUBLE);
			  write_exp_elt_type (builtin_type_double);
			  write_exp_elt_dblcst (yyvsp[0].dval);
			  write_exp_elt_opcode (OP_DOUBLE); ;
    break;}
case 58:
#line 450 "./c-exp.y"
{ write_exp_elt_opcode (OP_LAST);
			  write_exp_elt_longcst ((LONGEST) yyvsp[0].lval);
			  write_exp_elt_opcode (OP_LAST); ;
    break;}
case 59:
#line 456 "./c-exp.y"
{ write_exp_elt_opcode (OP_REGISTER);
			  write_exp_elt_longcst ((LONGEST) yyvsp[0].lval);
			  write_exp_elt_opcode (OP_REGISTER); ;
    break;}
case 60:
#line 462 "./c-exp.y"
{ write_exp_elt_opcode (OP_INTERNALVAR);
			  write_exp_elt_intern (yyvsp[0].ivar);
			  write_exp_elt_opcode (OP_INTERNALVAR); ;
    break;}
case 61:
#line 468 "./c-exp.y"
{ write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (builtin_type_int);
			  write_exp_elt_longcst ((LONGEST) TYPE_LENGTH (yyvsp[-1].tval));
			  write_exp_elt_opcode (OP_LONG); ;
    break;}
case 62:
#line 475 "./c-exp.y"
{ write_exp_elt_opcode (OP_STRING);
			  write_exp_string (yyvsp[0].sval);
			  write_exp_elt_opcode (OP_STRING); ;
    break;}
case 63:
#line 482 "./c-exp.y"
{ write_exp_elt_opcode (OP_THIS);
			  write_exp_elt_opcode (OP_THIS); ;
    break;}
case 64:
#line 489 "./c-exp.y"
{
			  if (yyvsp[0].ssym.sym != 0)
			      yyval.bval = SYMBOL_BLOCK_VALUE (yyvsp[0].ssym.sym);
			  else
			    {
			      struct symtab *tem =
				  lookup_symtab (copy_name (yyvsp[0].ssym.stoken));
			      if (tem)
				yyval.bval = BLOCKVECTOR_BLOCK
					 (BLOCKVECTOR (tem), STATIC_BLOCK);
			      else
				error ("No file or function \"%s\".",
				       copy_name (yyvsp[0].ssym.stoken));
			    }
			;
    break;}
case 65:
#line 507 "./c-exp.y"
{ struct symbol *tem
			    = lookup_symbol (copy_name (yyvsp[0].sval), yyvsp[-2].bval,
					     VAR_NAMESPACE, 0, NULL);
			  if (!tem || SYMBOL_CLASS (tem) != LOC_BLOCK)
			    error ("No function \"%s\" in specified context.",
				   copy_name (yyvsp[0].sval));
			  yyval.bval = SYMBOL_BLOCK_VALUE (tem); ;
    break;}
case 66:
#line 517 "./c-exp.y"
{ struct symbol *sym;
			  sym = lookup_symbol (copy_name (yyvsp[0].sval), yyvsp[-2].bval,
					       VAR_NAMESPACE, 0, NULL);
			  if (sym == 0)
			    error ("No symbol \"%s\" in specified context.",
				   copy_name (yyvsp[0].sval));

			  write_exp_elt_opcode (OP_VAR_VALUE);
			  write_exp_elt_sym (sym);
			  write_exp_elt_opcode (OP_VAR_VALUE); ;
    break;}
case 67:
#line 530 "./c-exp.y"
{
			  struct type *type = yyvsp[-2].tval;
			  if (TYPE_CODE (type) != TYPE_CODE_STRUCT
			      && TYPE_CODE (type) != TYPE_CODE_UNION)
			    error ("`%s' is not defined as an aggregate type.",
				   TYPE_NAME (type));

			  write_exp_elt_opcode (OP_SCOPE);
			  write_exp_elt_type (type);
			  write_exp_string (yyvsp[0].sval);
			  write_exp_elt_opcode (OP_SCOPE);
			;
    break;}
case 68:
#line 543 "./c-exp.y"
{
			  struct type *type = yyvsp[-3].tval;
			  if (TYPE_CODE (type) != TYPE_CODE_STRUCT
			      && TYPE_CODE (type) != TYPE_CODE_UNION)
			    error ("`%s' is not defined as an aggregate type.",
				   TYPE_NAME (type));

			  if (strcmp (type_name_no_tag (type), yyvsp[0].sval.ptr))
			    error ("invalid destructor `%s::~%s'",
				   type_name_no_tag (type), yyvsp[0].sval.ptr);

			  write_exp_elt_opcode (OP_SCOPE);
			  write_exp_elt_type (type);
			  write_exp_string (yyvsp[0].sval);
			  write_exp_elt_opcode (OP_SCOPE);
			  write_exp_elt_opcode (UNOP_LOGNOT);
			;
    break;}
case 69:
#line 561 "./c-exp.y"
{
			  char *name = copy_name (yyvsp[0].sval);
			  struct symbol *sym;
			  int i;

			  sym =
			    lookup_symbol (name, 0, VAR_NAMESPACE, 0, NULL);
			  if (sym)
			    {
			      write_exp_elt_opcode (OP_VAR_VALUE);
			      write_exp_elt_sym (sym);
			      write_exp_elt_opcode (OP_VAR_VALUE);
			      break;
			    }
			  for (i = 0; i < misc_function_count; i++)
			    if (!strcmp (misc_function_vector[i].name, name))
			      break;

			  if (i < misc_function_count)
			    {
			      enum misc_function_type mft =
				  misc_function_vector[i].type;
			      
			      write_exp_elt_opcode (OP_LONG);
			      write_exp_elt_type (builtin_type_int);
			      write_exp_elt_longcst ((LONGEST) misc_function_vector[i].address);
			      write_exp_elt_opcode (OP_LONG);
			      write_exp_elt_opcode (UNOP_MEMVAL);
			      if (mft == mf_data || mft == mf_bss)
				write_exp_elt_type (builtin_type_int);
			      else if (mft == mf_text)
				write_exp_elt_type (lookup_function_type (builtin_type_int));
			      else
				write_exp_elt_type (builtin_type_char);
			      write_exp_elt_opcode (UNOP_MEMVAL);
			    }
			  else
			    if (symtab_list == 0
				&& partial_symtab_list == 0)
			      error ("No symbol table is loaded.  Use the \"file\" command.");
			    else
			      error ("No symbol \"%s\" in current context.", name);
			;
    break;}
case 70:
#line 607 "./c-exp.y"
{ struct symbol *sym = yyvsp[0].ssym.sym;

			  if (sym)
			    {
			      switch (SYMBOL_CLASS (sym))
				{
				case LOC_REGISTER:
				case LOC_ARG:
				case LOC_REF_ARG:
				case LOC_REGPARM:
				case LOC_LOCAL:
				case LOC_LOCAL_ARG:
				  if (innermost_block == 0 ||
				      contained_in (block_found, 
						    innermost_block))
				    innermost_block = block_found;
				case LOC_UNDEF:
				case LOC_CONST:
				case LOC_STATIC:
				case LOC_TYPEDEF:
				case LOC_LABEL:
				case LOC_BLOCK:
				case LOC_CONST_BYTES:

				  /* In this case the expression can
				     be evaluated regardless of what
				     frame we are in, so there is no
				     need to check for the
				     innermost_block.  These cases are
				     listed so that gcc -Wall will
				     report types that may not have
				     been considered.  */

				  break;
				}
			      write_exp_elt_opcode (OP_VAR_VALUE);
			      write_exp_elt_sym (sym);
			      write_exp_elt_opcode (OP_VAR_VALUE);
			    }
			  else if (yyvsp[0].ssym.is_a_field_of_this)
			    {
			      /* C++: it hangs off of `this'.  Must
			         not inadvertently convert from a method call
				 to data ref.  */
			      if (innermost_block == 0 || 
				  contained_in (block_found, innermost_block))
				innermost_block = block_found;
			      write_exp_elt_opcode (OP_THIS);
			      write_exp_elt_opcode (OP_THIS);
			      write_exp_elt_opcode (STRUCTOP_PTR);
			      write_exp_string (yyvsp[0].ssym.stoken);
			      write_exp_elt_opcode (STRUCTOP_PTR);
			    }
			  else
			    {
			      register int i;
			      register char *arg = copy_name (yyvsp[0].ssym.stoken);

				/* FIXME, this search is linear!  At least
				   optimize the strcmp with a 1-char cmp... */
			      for (i = 0; i < misc_function_count; i++)
				if (!strcmp (misc_function_vector[i].name, arg))
				  break;

			      if (i < misc_function_count)
				{
				  enum misc_function_type mft =
				      misc_function_vector[i].type;
				  
				  write_exp_elt_opcode (OP_LONG);
				  write_exp_elt_type (builtin_type_int);
				  write_exp_elt_longcst ((LONGEST) misc_function_vector[i].address);
				  write_exp_elt_opcode (OP_LONG);
				  write_exp_elt_opcode (UNOP_MEMVAL);
				  if (mft == mf_data || mft == mf_bss)
				    write_exp_elt_type (builtin_type_int);
				  else if (mft == mf_text)
				    write_exp_elt_type (lookup_function_type (builtin_type_int));
				  else
				    write_exp_elt_type (builtin_type_char);
				  write_exp_elt_opcode (UNOP_MEMVAL);
				}
			      else if (symtab_list == 0
				       && partial_symtab_list == 0)
				error ("No symbol table is loaded.  Use the \"file\" command.");
			      else
				error ("No symbol \"%s\" in current context.",
				       copy_name (yyvsp[0].ssym.stoken));
			    }
			;
    break;}
case 72:
#line 702 "./c-exp.y"
{
		  /* This is where the interesting stuff happens.  */
		  int done = 0;
		  int array_size;
		  struct type *follow_type = yyvsp[-1].tval;
		  
		  while (!done)
		    switch (pop_type ())
		      {
		      case tp_end:
			done = 1;
			break;
		      case tp_pointer:
			follow_type = lookup_pointer_type (follow_type);
			break;
		      case tp_reference:
			follow_type = lookup_reference_type (follow_type);
			break;
		      case tp_array:
			array_size = pop_type_int ();
			if (array_size != -1)
			  follow_type = create_array_type (follow_type,
							   array_size);
			else
			  follow_type = lookup_pointer_type (follow_type);
			break;
		      case tp_function:
			follow_type = lookup_function_type (follow_type);
			break;
		      }
		  yyval.tval = follow_type;
		;
    break;}
case 73:
#line 737 "./c-exp.y"
{ push_type (tp_pointer); yyval.voidval = 0; ;
    break;}
case 74:
#line 739 "./c-exp.y"
{ push_type (tp_pointer); yyval.voidval = yyvsp[0].voidval; ;
    break;}
case 75:
#line 741 "./c-exp.y"
{ push_type (tp_reference); yyval.voidval = 0; ;
    break;}
case 76:
#line 743 "./c-exp.y"
{ push_type (tp_reference); yyval.voidval = yyvsp[0].voidval; ;
    break;}
case 78:
#line 748 "./c-exp.y"
{ yyval.voidval = yyvsp[-1].voidval; ;
    break;}
case 79:
#line 750 "./c-exp.y"
{
			  push_type_int (yyvsp[0].lval);
			  push_type (tp_array);
			;
    break;}
case 80:
#line 755 "./c-exp.y"
{
			  push_type_int (yyvsp[0].lval);
			  push_type (tp_array);
			  yyval.voidval = 0;
			;
    break;}
case 81:
#line 761 "./c-exp.y"
{ push_type (tp_function); ;
    break;}
case 82:
#line 763 "./c-exp.y"
{ push_type (tp_function); ;
    break;}
case 83:
#line 767 "./c-exp.y"
{ yyval.lval = -1; ;
    break;}
case 84:
#line 769 "./c-exp.y"
{ yyval.lval = yyvsp[-1].lval; ;
    break;}
case 85:
#line 773 "./c-exp.y"
{ yyval.voidval = 0; ;
    break;}
case 87:
#line 778 "./c-exp.y"
{ yyval.tval = lookup_member_type (builtin_type_int, yyvsp[-2].tval); ;
    break;}
case 88:
#line 780 "./c-exp.y"
{ yyval.tval = lookup_member_type (yyvsp[-5].tval, yyvsp[-3].tval); ;
    break;}
case 89:
#line 782 "./c-exp.y"
{ yyval.tval = lookup_member_type
			    (lookup_function_type (yyvsp[-7].tval), yyvsp[-5].tval); ;
    break;}
case 90:
#line 785 "./c-exp.y"
{ yyval.tval = lookup_member_type
			    (lookup_function_type (yyvsp[-8].tval), yyvsp[-6].tval);
			  free (yyvsp[-1].tvec); ;
    break;}
case 91:
#line 792 "./c-exp.y"
{ yyval.tval = yyvsp[0].tsym.type; ;
    break;}
case 92:
#line 794 "./c-exp.y"
{ yyval.tval = builtin_type_int; ;
    break;}
case 93:
#line 796 "./c-exp.y"
{ yyval.tval = builtin_type_long; ;
    break;}
case 94:
#line 798 "./c-exp.y"
{ yyval.tval = builtin_type_short; ;
    break;}
case 95:
#line 800 "./c-exp.y"
{ yyval.tval = builtin_type_long; ;
    break;}
case 96:
#line 802 "./c-exp.y"
{ yyval.tval = builtin_type_unsigned_long; ;
    break;}
case 97:
#line 804 "./c-exp.y"
{ yyval.tval = builtin_type_long_long; ;
    break;}
case 98:
#line 806 "./c-exp.y"
{ yyval.tval = builtin_type_long_long; ;
    break;}
case 99:
#line 808 "./c-exp.y"
{ yyval.tval = builtin_type_unsigned_long_long; ;
    break;}
case 100:
#line 810 "./c-exp.y"
{ yyval.tval = builtin_type_unsigned_long_long; ;
    break;}
case 101:
#line 812 "./c-exp.y"
{ yyval.tval = builtin_type_short; ;
    break;}
case 102:
#line 814 "./c-exp.y"
{ yyval.tval = builtin_type_unsigned_short; ;
    break;}
case 103:
#line 816 "./c-exp.y"
{ yyval.tval = lookup_struct (copy_name (yyvsp[0].sval),
					      expression_context_block); ;
    break;}
case 104:
#line 819 "./c-exp.y"
{ yyval.tval = lookup_union (copy_name (yyvsp[0].sval),
					     expression_context_block); ;
    break;}
case 105:
#line 822 "./c-exp.y"
{ yyval.tval = lookup_enum (copy_name (yyvsp[0].sval),
					    expression_context_block); ;
    break;}
case 106:
#line 825 "./c-exp.y"
{ yyval.tval = lookup_unsigned_typename (TYPE_NAME(yyvsp[0].tsym.type)); ;
    break;}
case 107:
#line 827 "./c-exp.y"
{ yyval.tval = builtin_type_unsigned_int; ;
    break;}
case 108:
#line 829 "./c-exp.y"
{ yyval.tval = yyvsp[0].tsym.type; ;
    break;}
case 109:
#line 831 "./c-exp.y"
{ yyval.tval = builtin_type_int; ;
    break;}
case 111:
#line 836 "./c-exp.y"
{
		  yyval.tsym.stoken.ptr = "int";
		  yyval.tsym.stoken.length = 3;
		  yyval.tsym.type = builtin_type_int;
		;
    break;}
case 112:
#line 842 "./c-exp.y"
{
		  yyval.tsym.stoken.ptr = "long";
		  yyval.tsym.stoken.length = 4;
		  yyval.tsym.type = builtin_type_long;
		;
    break;}
case 113:
#line 848 "./c-exp.y"
{
		  yyval.tsym.stoken.ptr = "short";
		  yyval.tsym.stoken.length = 5;
		  yyval.tsym.type = builtin_type_short;
		;
    break;}
case 114:
#line 857 "./c-exp.y"
{ yyval.tvec = (struct type **)xmalloc (sizeof (struct type *) * 2);
		  yyval.tvec[0] = (struct type *)0;
		  yyval.tvec[1] = yyvsp[0].tval;
		;
    break;}
case 115:
#line 862 "./c-exp.y"
{ int len = sizeof (struct type *) * ++(yyvsp[-2].ivec[0]);
		  yyval.tvec = (struct type **)xrealloc (yyvsp[-2].tvec, len);
		  yyval.tvec[yyval.ivec[0]] = yyvsp[0].tval;
		;
    break;}
case 116:
#line 868 "./c-exp.y"
{ yyval.sval = yyvsp[0].ssym.stoken; ;
    break;}
case 117:
#line 869 "./c-exp.y"
{ yyval.sval = yyvsp[0].ssym.stoken; ;
    break;}
case 118:
#line 870 "./c-exp.y"
{ yyval.sval = yyvsp[0].tsym.stoken; ;
    break;}
case 119:
#line 871 "./c-exp.y"
{ yyval.sval = yyvsp[0].ssym.stoken; ;
    break;}
case 120:
#line 872 "./c-exp.y"
{ yyval.sval = yyvsp[0].ssym.stoken; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 423 "/usr/latest/lib/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  for (x = 0; x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) xmalloc(size + 15);
	  strcpy(msg, "parse error");

	  if (count < 5)
	    {
	      count = 0;
	      for (x = 0; x < (sizeof(yytname) / sizeof(char *)); x++)
		if (yycheck[x + yyn] == x)
		  {
		    strcat(msg, count == 0 ? ", expecting `" : " or `");
		    strcat(msg, yytname[x]);
		    strcat(msg, "'");
		    count++;
		  }
	    }
	  yyerror(msg);
	  free(msg);
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 887 "./c-exp.y"


/* Take care of parsing a number (anything that starts with a digit).
   Set yylval and return the token type; update lexptr.
   LEN is the number of characters in it.  */

/*** Needs some error checking for the float case ***/

static int
parse_number (p, len, parsed_float, putithere)
     register char *p;
     register int len;
     int parsed_float;
     YYSTYPE *putithere;
{
  register LONGEST n = 0;
  register LONGEST prevn = 0;
  register int i;
  register int c;
  register int base = input_radix;
  int unsigned_p = 0;

  extern double atof ();

  if (parsed_float)
    {
      /* It's a float since it contains a point or an exponent.  */
      putithere->dval = atof (p);
      return FLOAT;
    }

  /* Handle base-switching prefixes 0x, 0t, 0d, 0 */
  if (p[0] == '0')
    switch (p[1])
      {
      case 'x':
      case 'X':
	if (len >= 3)
	  {
	    p += 2;
	    base = 16;
	    len -= 2;
	  }
	break;

      case 't':
      case 'T':
      case 'd':
      case 'D':
	if (len >= 3)
	  {
	    p += 2;
	    base = 10;
	    len -= 2;
	  }
	break;

      default:
	base = 8;
	break;
      }

  while (len-- > 0)
    {
      c = *p++;
      if (c >= 'A' && c <= 'Z')
	c += 'a' - 'A';
      if (c != 'l' && c != 'u')
	n *= base;
      if (c >= '0' && c <= '9')
	n += i = c - '0';
      else
	{
	  if (base > 10 && c >= 'a' && c <= 'f')
	    n += i = c - 'a' + 10;
	  else if (len == 0 && c == 'l')
	    ;
	  else if (len == 0 && c == 'u')
	    unsigned_p = 1;
	  else
	    return ERROR;	/* Char not a digit */
	}
      if (i >= base)
	return ERROR;		/* Invalid digit in this base */
      if(!unsigned_p && (prevn >= n))
	 unsigned_p=1;		/* Try something unsigned */
      /* Don't do the range check if n==i and i==0, since that special
	 case will give an overflow error. */
      if(RANGE_CHECK && n!=0)
      {	
	 if((unsigned_p && (unsigned)prevn >= (unsigned)n))
	    range_error("Overflow on numeric constant.");	 
      }
      prevn=n;
    }

  if (unsigned_p)
    {
      putithere->ulval = n;
      return UINT;
    }
  else
    {
      putithere->lval = n;
      return INT;
    }
}

struct token
{
  char *operator;
  int token;
  enum exp_opcode opcode;
};

const static struct token tokentab3[] =
  {
    {">>=", ASSIGN_MODIFY, BINOP_RSH},
    {"<<=", ASSIGN_MODIFY, BINOP_LSH}
  };

const static struct token tokentab2[] =
  {
    {"+=", ASSIGN_MODIFY, BINOP_ADD},
    {"-=", ASSIGN_MODIFY, BINOP_SUB},
    {"*=", ASSIGN_MODIFY, BINOP_MUL},
    {"/=", ASSIGN_MODIFY, BINOP_DIV},
    {"%=", ASSIGN_MODIFY, BINOP_REM},
    {"|=", ASSIGN_MODIFY, BINOP_LOGIOR},
    {"&=", ASSIGN_MODIFY, BINOP_LOGAND},
    {"^=", ASSIGN_MODIFY, BINOP_LOGXOR},
    {"++", INCREMENT, BINOP_END},
    {"--", DECREMENT, BINOP_END},
    {"->", ARROW, BINOP_END},
    {"&&", AND, BINOP_END},
    {"||", OR, BINOP_END},
    {"::", COLONCOLON, BINOP_END},
    {"<<", LSH, BINOP_END},
    {">>", RSH, BINOP_END},
    {"==", EQUAL, BINOP_END},
    {"!=", NOTEQUAL, BINOP_END},
    {"<=", LEQ, BINOP_END},
    {">=", GEQ, BINOP_END}
  };

/* Read one token, getting characters through lexptr.  */

int
yylex ()
{
  register int c;
  register int namelen;
  register unsigned i;
  register char *tokstart;

 retry:

  tokstart = lexptr;
  /* See if it is a special token of length 3.  */
  for (i = 0; i < sizeof tokentab3 / sizeof tokentab3[0]; i++)
    if (!strncmp (tokstart, tokentab3[i].operator, 3))
      {
	lexptr += 3;
	yylval.opcode = tokentab3[i].opcode;
	return tokentab3[i].token;
      }

  /* See if it is a special token of length 2.  */
  for (i = 0; i < sizeof tokentab2 / sizeof tokentab2[0]; i++)
    if (!strncmp (tokstart, tokentab2[i].operator, 2))
      {
	lexptr += 2;
	yylval.opcode = tokentab2[i].opcode;
	return tokentab2[i].token;
      }

  switch (c = *tokstart)
    {
    case 0:
      return 0;

    case ' ':
    case '\t':
    case '\n':
      lexptr++;
      goto retry;

    case '\'':
      lexptr++;
      c = *lexptr++;
      if (c == '\\')
	c = parse_escape (&lexptr);
      yylval.lval = c;
      c = *lexptr++;
      if (c != '\'')
	error ("Invalid character constant.");
      return CHAR;

    case '(':
      paren_depth++;
      lexptr++;
      return c;

    case ')':
      if (paren_depth == 0)
	return 0;
      paren_depth--;
      lexptr++;
      return c;

    case ',':
      if (comma_terminates && paren_depth == 0)
	return 0;
      lexptr++;
      return c;

    case '.':
      /* Might be a floating point number.  */
      if (lexptr[1] < '0' || lexptr[1] > '9')
	goto symbol;		/* Nope, must be a symbol. */
      /* FALL THRU into number case.  */

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      {
	/* It's a number.  */
	int got_dot = 0, got_e = 0, toktype;
	register char *p = tokstart;
	int hex = input_radix > 10;

	if (c == '0' && (p[1] == 'x' || p[1] == 'X'))
	  {
	    p += 2;
	    hex = 1;
	  }
	else if (c == '0' && (p[1]=='t' || p[1]=='T' || p[1]=='d' || p[1]=='D'))
	  {
	    p += 2;
	    hex = 0;
	  }

	for (;; ++p)
	  {
	    if (!hex && !got_e && (*p == 'e' || *p == 'E'))
	      got_dot = got_e = 1;
	    else if (!hex && !got_dot && *p == '.')
	      got_dot = 1;
	    else if (got_e && (p[-1] == 'e' || p[-1] == 'E')
		     && (*p == '-' || *p == '+'))
	      /* This is the sign of the exponent, not the end of the
		 number.  */
	      continue;
	    /* We will take any letters or digits.  parse_number will
	       complain if past the radix, or if L or U are not final.  */
	    else if ((*p < '0' || *p > '9')
		     && ((*p < 'a' || *p > 'z')
				  && (*p < 'A' || *p > 'Z')))
	      break;
	  }
	toktype = parse_number (tokstart, p - tokstart, got_dot|got_e, &yylval);
        if (toktype == ERROR)
	  {
	    char *err_copy = (char *) alloca (p - tokstart + 1);

	    bcopy (tokstart, err_copy, p - tokstart);
	    err_copy[p - tokstart] = 0;
	    error ("Invalid number \"%s\".", err_copy);
	  }
	lexptr = p;
	return toktype;
      }

    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '|':
    case '&':
    case '^':
    case '~':
    case '!':
    case '@':
    case '<':
    case '>':
    case '[':
    case ']':
    case '?':
    case ':':
    case '=':
    case '{':
    case '}':
    symbol:
      lexptr++;
      return c;

    case '"':
      for (namelen = 1; (c = tokstart[namelen]) != '"'; namelen++)
	if (c == '\\')
	  {
	    c = tokstart[++namelen];
	    if (c >= '0' && c <= '9')
	      {
		c = tokstart[++namelen];
		if (c >= '0' && c <= '9')
		  c = tokstart[++namelen];
	      }
	  }
      yylval.sval.ptr = tokstart + 1;
      yylval.sval.length = namelen - 1;
      lexptr += namelen + 1;
      return STRING;
    }

  if (!(c == '_' || c == '$'
	|| (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
    /* We must have come across a bad character (e.g. ';').  */
    error ("Invalid character '%c' in expression.", c);

  /* It's a name.  See how long it is.  */
  namelen = 0;
  for (c = tokstart[namelen];
       (c == '_' || c == '$' || (c >= '0' && c <= '9')
	|| (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
       c = tokstart[++namelen])
    ;

  /* The token "if" terminates the expression and is NOT 
     removed from the input stream.  */
  if (namelen == 2 && tokstart[0] == 'i' && tokstart[1] == 'f')
    {
      return 0;
    }

  lexptr += namelen;

  /* Handle the tokens $digits; also $ (short for $0) and $$ (short for $$1)
     and $$digits (equivalent to $<-digits> if you could type that).
     Make token type LAST, and put the number (the digits) in yylval.  */

  if (*tokstart == '$')
    {
      register int negate = 0;
      c = 1;
      /* Double dollar means negate the number and add -1 as well.
	 Thus $$ alone means -1.  */
      if (namelen >= 2 && tokstart[1] == '$')
	{
	  negate = 1;
	  c = 2;
	}
      if (c == namelen)
	{
	  /* Just dollars (one or two) */
	  yylval.lval = - negate;
	  return LAST;
	}
      /* Is the rest of the token digits?  */
      for (; c < namelen; c++)
	if (!(tokstart[c] >= '0' && tokstart[c] <= '9'))
	  break;
      if (c == namelen)
	{
	  yylval.lval = atoi (tokstart + 1 + negate);
	  if (negate)
	    yylval.lval = - yylval.lval;
	  return LAST;
	}
    }

  /* Handle tokens that refer to machine registers:
     $ followed by a register name.  */

  if (*tokstart == '$') {
    for (c = 0; c < NUM_REGS; c++)
      if (namelen - 1 == strlen (reg_names[c])
	  && !strncmp (tokstart + 1, reg_names[c], namelen - 1))
	{
	  yylval.lval = c;
	  return REGNAME;
	}
    for (c = 0; c < num_std_regs; c++)
     if (namelen - 1 == strlen (std_regs[c].name)
	 && !strncmp (tokstart + 1, std_regs[c].name, namelen - 1))
       {
	 yylval.lval = std_regs[c].regnum;
	 return REGNAME;
       }
  }
  /* Catch specific keywords.  Should be done with a data structure.  */
  switch (namelen)
    {
    case 8:
      if (!strncmp (tokstart, "unsigned", 8))
	return UNSIGNED;
      break;
    case 6:
      if (!strncmp (tokstart, "struct", 6))
	return STRUCT;
      if (!strncmp (tokstart, "signed", 6))
	return SIGNED;
      if (!strncmp (tokstart, "sizeof", 6))      
	return SIZEOF;
      break;
    case 5:
      if (!strncmp (tokstart, "union", 5))
	return UNION;
      if (!strncmp (tokstart, "short", 5))
	return SHORT;
      break;
    case 4:
      if (!strncmp (tokstart, "enum", 4))
	return ENUM;
      if (!strncmp (tokstart, "long", 4))
	return LONG;
      if (!strncmp (tokstart, "this", 4))
	{
	  static const char this_name[] =
				 { CPLUS_MARKER, 't', 'h', 'i', 's', '\0' };

	  if (lookup_symbol (this_name, expression_context_block,
			     VAR_NAMESPACE, 0, NULL))
	    return THIS;
	}
      break;
    case 3:
      if (!strncmp (tokstart, "int", 3))
	return INT_KEYWORD;
      break;
    default:
      break;
    }

  yylval.sval.ptr = tokstart;
  yylval.sval.length = namelen;

  /* Any other names starting in $ are debugger internal variables.  */

  if (*tokstart == '$')
    {
      yylval.ivar =  lookup_internalvar (copy_name (yylval.sval) + 1);
      return VARIABLE;
    }

  /* Use token-type BLOCKNAME for symbols that happen to be defined as
     functions or symtabs.  If this is not so, then ...
     Use token-type TYPENAME for symbols that happen to be defined
     currently as names of types; NAME for other symbols.
     The caller is not constrained to care about the distinction.  */
  {
    char *tmp = copy_name (yylval.sval);
    struct symbol *sym;
    int is_a_field_of_this = 0;
    int hextype;

    sym = lookup_symbol (tmp, expression_context_block,
			 VAR_NAMESPACE, &is_a_field_of_this, NULL);
    if ((sym && SYMBOL_CLASS (sym) == LOC_BLOCK) ||
        lookup_partial_symtab (tmp))
      {
	yylval.ssym.sym = sym;
	yylval.ssym.is_a_field_of_this = is_a_field_of_this;
	return BLOCKNAME;
      }
    if (sym && SYMBOL_CLASS (sym) == LOC_TYPEDEF)
        {
	  yylval.tsym.type = SYMBOL_TYPE (sym);
	  return TYPENAME;
        }
    if ((yylval.tsym.type = lookup_primitive_typename (tmp)) != 0)
	return TYPENAME;

    /* Input names that aren't symbols but ARE valid hex numbers,
       when the input radix permits them, can be names or numbers
       depending on the parse.  Note we support radixes > 16 here.  */
    if (!sym && 
        ((tokstart[0] >= 'a' && tokstart[0] < 'a' + input_radix - 10) ||
         (tokstart[0] >= 'A' && tokstart[0] < 'A' + input_radix - 10)))
      {
 	YYSTYPE newlval;	/* Its value is ignored.  */
	hextype = parse_number (tokstart, namelen, 0, &newlval);
	if (hextype == INT)
	  {
	    yylval.ssym.sym = sym;
	    yylval.ssym.is_a_field_of_this = is_a_field_of_this;
	    return NAME_OR_INT;
	  }
	if (hextype == UINT)
	  {
	    yylval.ssym.sym = sym;
	    yylval.ssym.is_a_field_of_this = is_a_field_of_this;
	    return NAME_OR_UINT;
	  }
      }

    /* Any other kind of symbol */
    yylval.ssym.sym = sym;
    yylval.ssym.is_a_field_of_this = is_a_field_of_this;
    return NAME;
  }
}

void
yyerror (msg)
     char *msg;
{
  error ("Invalid syntax in expression.");
}

/* Table mapping opcodes into strings for printing operators
   and precedences of the operators.  */

const static struct op_print c_op_print_tab[] =
  {
    {",",  BINOP_COMMA, PREC_COMMA, 0},
    {"=",  BINOP_ASSIGN, PREC_ASSIGN, 1},
    {"||", BINOP_OR, PREC_OR, 0},
    {"&&", BINOP_AND, PREC_AND, 0},
    {"|",  BINOP_LOGIOR, PREC_LOGIOR, 0},
    {"&",  BINOP_LOGAND, PREC_LOGAND, 0},
    {"^",  BINOP_LOGXOR, PREC_LOGXOR, 0},
    {"==", BINOP_EQUAL, PREC_EQUAL, 0},
    {"!=", BINOP_NOTEQUAL, PREC_EQUAL, 0},
    {"<=", BINOP_LEQ, PREC_ORDER, 0},
    {">=", BINOP_GEQ, PREC_ORDER, 0},
    {">",  BINOP_GTR, PREC_ORDER, 0},
    {"<",  BINOP_LESS, PREC_ORDER, 0},
    {">>", BINOP_RSH, PREC_SHIFT, 0},
    {"<<", BINOP_LSH, PREC_SHIFT, 0},
    {"+",  BINOP_ADD, PREC_ADD, 0},
    {"-",  BINOP_SUB, PREC_ADD, 0},
    {"*",  BINOP_MUL, PREC_MUL, 0},
    {"/",  BINOP_DIV, PREC_MUL, 0},
    {"%",  BINOP_REM, PREC_MUL, 0},
    {"@",  BINOP_REPEAT, PREC_REPEAT, 0},
    {"-",  UNOP_NEG, PREC_PREFIX, 0},
    {"!",  UNOP_ZEROP, PREC_PREFIX, 0},
    {"~",  UNOP_LOGNOT, PREC_PREFIX, 0},
    {"*",  UNOP_IND, PREC_PREFIX, 0},
    {"&",  UNOP_ADDR, PREC_PREFIX, 0},
    {"sizeof ", UNOP_SIZEOF, PREC_PREFIX, 0},
    {"++", UNOP_PREINCREMENT, PREC_PREFIX, 0},
    {"--", UNOP_PREDECREMENT, PREC_PREFIX, 0},
    /* C++  */
    {"::", BINOP_SCOPE, PREC_PREFIX, 0},
};

/* These variables point to the objects
   representing the predefined C data types.  */

struct type *builtin_type_void;
struct type *builtin_type_char;
struct type *builtin_type_short;
struct type *builtin_type_int;
struct type *builtin_type_long;
struct type *builtin_type_long_long;
struct type *builtin_type_unsigned_char;
struct type *builtin_type_unsigned_short;
struct type *builtin_type_unsigned_int;
struct type *builtin_type_unsigned_long;
struct type *builtin_type_unsigned_long_long;
struct type *builtin_type_float;
struct type *builtin_type_double;

struct type ** const (c_builtin_types[]) = 
{
  &builtin_type_int,
  &builtin_type_long,
  &builtin_type_short,
  &builtin_type_char,
  &builtin_type_float,
  &builtin_type_double,
  &builtin_type_void,
  &builtin_type_long_long,
  &builtin_type_unsigned_char,
  &builtin_type_unsigned_short,
  &builtin_type_unsigned_int,
  &builtin_type_unsigned_long,
  &builtin_type_unsigned_long_long,
  0
};

/* FIXME:  Eventually do a separate defintion for C++.  */

const struct language_defn c_language_defn = {
  "c",				/* Language name */
  language_c,
  c_builtin_types,
  range_check_off,
  type_check_off,
  c_parse,
  c_error,
  &BUILTIN_TYPE_LONGEST,	 /* longest signed   integral type */
  &BUILTIN_TYPE_UNSIGNED_LONGEST,/* longest unsigned integral type */
  &builtin_type_double,		/* longest floating point type */
  "0x%x", "0x%", "x",		/* Hex   format, prefix, suffix */
  "0%o",  "0%",  "o",		/* Octal format, prefix, suffix */
  c_op_print_tab,		/* expression operators for printing */
  LANG_MAGIC
};

void
_initialize_c_exp ()
{
  /* FIXME:  The code below assumes that the sizes of the basic data
     types are the same on the host and target machines!!!  */

  builtin_type_void = init_type (TYPE_CODE_VOID, 1, 0, "void");

  builtin_type_float = init_type (TYPE_CODE_FLT, sizeof (float), 0, "float");
  builtin_type_double = init_type (TYPE_CODE_FLT, sizeof (double), 0, "double");

  builtin_type_char = init_type (TYPE_CODE_INT, sizeof (char), 0, "char");
  builtin_type_short = init_type (TYPE_CODE_INT, sizeof (short), 0, "short");
  builtin_type_long = init_type (TYPE_CODE_INT, sizeof (long), 0, "long");
  builtin_type_int = init_type (TYPE_CODE_INT, sizeof (int), 0, "int");

  builtin_type_unsigned_char = init_type (TYPE_CODE_INT, sizeof (char), 1, "unsigned char");
  builtin_type_unsigned_short = init_type (TYPE_CODE_INT, sizeof (short), 1, "unsigned short");
  builtin_type_unsigned_long = init_type (TYPE_CODE_INT, sizeof (long), 1, "unsigned long");
  builtin_type_unsigned_int = init_type (TYPE_CODE_INT, sizeof (int), 1, "unsigned int");

  builtin_type_long_long =
    init_type (TYPE_CODE_INT, TARGET_LONG_LONG_BIT / TARGET_CHAR_BIT,
	       0, "long long");
  builtin_type_unsigned_long_long = 
    init_type (TYPE_CODE_INT, TARGET_LONG_LONG_BIT / TARGET_CHAR_BIT,
	       1, "unsigned long long");

  add_language (&c_language_defn);
  set_language (language_c);		/* Make C the default language */
}
