
/*  A Bison parser, made from ./ch-exp.y with Bison version GNU Bison version 1.24
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	INTEGER_LITERAL	258
#define	BOOLEAN_LITERAL	259
#define	CHARACTER_LITERAL	260
#define	FLOAT_LITERAL	261
#define	GENERAL_PROCEDURE_NAME	262
#define	LOCATION_NAME	263
#define	EMPTINESS_LITERAL	264
#define	CHARACTER_STRING_LITERAL	265
#define	BIT_STRING_LITERAL	266
#define	TYPENAME	267
#define	FIELD_NAME	268
#define	CASE	269
#define	OF	270
#define	ESAC	271
#define	LOGIOR	272
#define	ORIF	273
#define	LOGXOR	274
#define	LOGAND	275
#define	ANDIF	276
#define	NOTEQUAL	277
#define	GTR	278
#define	LEQ	279
#define	IN	280
#define	SLASH_SLASH	281
#define	MOD	282
#define	REM	283
#define	NOT	284
#define	POINTER	285
#define	RECEIVE	286
#define	UP	287
#define	IF	288
#define	THEN	289
#define	ELSE	290
#define	FI	291
#define	ELSIF	292
#define	ILLEGAL_TOKEN	293
#define	NUM	294
#define	PRED	295
#define	SUCC	296
#define	ABS	297
#define	CARD	298
#define	MAX_TOKEN	299
#define	MIN_TOKEN	300
#define	ADDR_TOKEN	301
#define	SIZE	302
#define	UPPER	303
#define	LOWER	304
#define	LENGTH	305
#define	ARRAY	306
#define	GDB_REGNAME	307
#define	GDB_LAST	308
#define	GDB_VARIABLE	309
#define	GDB_ASSIGNMENT	310

#line 54 "./ch-exp.y"


#include "defs.h"
#include <string.h>
#include <ctype.h>
#include "expression.h"
#include "language.h"
#include "value.h"
#include "parser-defs.h"
#include "ch-lang.h"
#include "bfd.h" /* Required by objfiles.h.  */
#include "symfile.h" /* Required by objfiles.h.  */
#include "objfiles.h" /* For have_full_symbols and have_partial_symbols */

/* Remap normal yacc parser interface names (yyparse, yylex, yyerror, etc),
   as well as gratuitiously global symbol names, so we can have multiple
   yacc generated parsers in gdb.  Note that these are only the variables
   produced by yacc.  If other parser generators (bison, byacc, etc) produce
   additional global names that conflict at link time, then those parser
   generators need to be fixed instead of adding those names to this list. */

#define	yymaxdepth chill_maxdepth
#define	yyparse	chill_parse
#define	yylex	chill_lex
#define	yyerror	chill_error
#define	yylval	chill_lval
#define	yychar	chill_char
#define	yydebug	chill_debug
#define	yypact	chill_pact
#define	yyr1	chill_r1
#define	yyr2	chill_r2
#define	yydef	chill_def
#define	yychk	chill_chk
#define	yypgo	chill_pgo
#define	yyact	chill_act
#define	yyexca	chill_exca
#define	yyerrflag chill_errflag
#define	yynerrs	chill_nerrs
#define	yyps	chill_ps
#define	yypv	chill_pv
#define	yys	chill_s
#define	yy_yys	chill_yys
#define	yystate	chill_state
#define	yytmp	chill_tmp
#define	yyv	chill_v
#define	yy_yyv	chill_yyv
#define	yyval	chill_val
#define	yylloc	chill_lloc
#define	yyreds	chill_reds		/* With YYDEBUG defined */
#define	yytoks	chill_toks		/* With YYDEBUG defined */
#define yylhs	chill_yylhs
#define yylen	chill_yylen
#define yydefred chill_yydefred
#define yydgoto	chill_yydgoto
#define yysindex chill_yysindex
#define yyrindex chill_yyrindex
#define yygindex chill_yygindex
#define yytable	 chill_yytable
#define yycheck	 chill_yycheck

#ifndef YYDEBUG
#define	YYDEBUG	0		/* Default to no yydebug support */
#endif

static void
write_lower_upper_value PARAMS ((enum exp_opcode, struct type *type));

int
yyparse PARAMS ((void));

static int
yylex PARAMS ((void));

void
yyerror PARAMS ((char *));


#line 136 "./ch-exp.y"
typedef union
  {
    LONGEST lval;
    unsigned LONGEST ulval;
    struct {
      LONGEST val;
      struct type *type;
    } typed_val;
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

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		203
#define	YYFLAG		-32768
#define	YYNTBASE	71

#define YYTRANSLATE(x) ((unsigned)(x) <= 310 ? yytranslate[x] : 106)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    44,
    45,    34,    32,    70,    33,    14,    35,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    16,    15,    29,
    25,    27,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    42,     2,    43,     2,     2,     2,     2,     2,     2,     2,
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
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    17,    18,
    19,    20,    21,    22,    23,    24,    26,    28,    30,    31,
    36,    37,    38,    39,    40,    41,    46,    47,    48,    49,
    50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
    60,    61,    62,    63,    64,    65,    66,    67,    68,    69
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     4,     6,     8,    10,    12,    14,    16,    20,
    21,    23,    26,    28,    30,    34,    37,    40,    44,    46,
    48,    50,    52,    54,    56,    58,    60,    62,    64,    66,
    68,    70,    72,    74,    76,    78,    82,    86,    88,    92,
    94,    95,    96,   101,   102,   108,   114,   120,   123,   129,
   133,   135,   137,   139,   145,   148,   151,   156,   158,   162,
   166,   170,   172,   176,   180,   182,   186,   190,   194,   198,
   202,   206,   210,   212,   216,   220,   224,   226,   230,   234,
   238,   242,   244,   247,   250,   253,   256,   259,   261,   265,
   270,   275,   280,   285,   290,   295,   300,   305,   310,   315,
   320,   325,   330,   335,   340,   342
};

static const short yyrhs[] = {    72,
     0,   105,     0,    91,     0,     8,     0,    67,     0,    66,
     0,    68,     0,    91,     0,    74,    70,    91,     0,     0,
    74,     0,    78,    44,     0,    45,     0,    73,     0,    76,
    75,    77,     0,    78,    13,     0,    78,    40,     0,    78,
    40,   105,     0,    79,     0,    80,     0,    85,     0,    88,
     0,    89,     0,   103,     0,    90,     0,     7,     0,     3,
     0,     4,     0,     5,     0,     6,     0,     9,     0,    10,
     0,    11,     0,    91,     0,    82,     0,    13,    70,    82,
     0,    13,    16,    91,     0,    81,     0,    83,    70,    81,
     0,    83,     0,     0,     0,    42,    86,    84,    43,     0,
     0,   105,    42,    87,    84,    43,     0,    76,    91,    16,
    91,    77,     0,    76,    91,    46,    91,    77,     0,   105,
    90,     0,    65,    44,    45,   105,    90,     0,    44,    91,
    45,     0,    95,     0,   102,     0,    92,     0,    47,    91,
    93,    94,    50,     0,    48,    91,     0,    49,    91,     0,
    51,    91,    93,    94,     0,    96,     0,    95,    20,    96,
     0,    95,    21,    96,     0,    95,    22,    96,     0,    97,
     0,    96,    23,    97,     0,    96,    24,    97,     0,    98,
     0,    97,    25,    98,     0,    97,    26,    98,     0,    97,
    27,    98,     0,    97,    28,    98,     0,    97,    29,    98,
     0,    97,    30,    98,     0,    97,    31,    98,     0,    99,
     0,    98,    32,    99,     0,    98,    33,    99,     0,    98,
    36,    99,     0,   100,     0,    99,    34,   100,     0,    99,
    35,   100,     0,    99,    37,   100,     0,    99,    38,   100,
     0,   101,     0,    33,   101,     0,    39,   101,     0,    90,
    80,     0,    40,    78,     0,    41,    91,     0,    78,     0,
    78,    69,    72,     0,    53,    44,    91,    45,     0,    54,
    44,    91,    45,     0,    55,    44,    91,    45,     0,    60,
    44,    91,    45,     0,    56,    44,    91,    45,     0,    57,
    44,    91,    45,     0,    58,    44,    91,    45,     0,    59,
    44,    91,    45,     0,    61,    44,    91,    45,     0,    61,
    44,   104,    45,     0,    63,    44,   104,    45,     0,    62,
    44,   104,    45,     0,    63,    44,    91,    45,     0,    62,
    44,    91,    45,     0,    64,    44,    91,    45,     0,   105,
     0,    12,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   271,   272,   278,   287,   294,   300,   306,   316,   320,   326,
   330,   336,   342,   346,   348,   354,   359,   363,   370,   371,
   372,   373,   374,   375,   382,   387,   398,   405,   411,   418,
   425,   434,   440,   450,   451,   454,   459,   466,   470,   476,
   477,   480,   482,   489,   492,   509,   513,   521,   527,   547,
   552,   553,   554,   557,   565,   568,   569,   575,   576,   580,
   584,   592,   593,   597,   605,   606,   610,   614,   618,   622,
   626,   630,   639,   640,   644,   648,   656,   657,   661,   665,
   669,   677,   678,   682,   686,   696,   700,   702,   708,   717,
   724,   726,   728,   730,   732,   734,   736,   738,   740,   745,
   747,   749,   751,   753,   757,   771
};

static const char * const yytname[] = {   "$","error","$undefined.","INTEGER_LITERAL",
"BOOLEAN_LITERAL","CHARACTER_LITERAL","FLOAT_LITERAL","GENERAL_PROCEDURE_NAME",
"LOCATION_NAME","EMPTINESS_LITERAL","CHARACTER_STRING_LITERAL","BIT_STRING_LITERAL",
"TYPENAME","FIELD_NAME","'.'","';'","':'","CASE","OF","ESAC","LOGIOR","ORIF",
"LOGXOR","LOGAND","ANDIF","'='","NOTEQUAL","'>'","GTR","'<'","LEQ","IN","'+'",
"'-'","'*'","'/'","SLASH_SLASH","MOD","REM","NOT","POINTER","RECEIVE","'['",
"']'","'('","')'","UP","IF","THEN","ELSE","FI","ELSIF","ILLEGAL_TOKEN","NUM",
"PRED","SUCC","ABS","CARD","MAX_TOKEN","MIN_TOKEN","ADDR_TOKEN","SIZE","UPPER",
"LOWER","LENGTH","ARRAY","GDB_REGNAME","GDB_LAST","GDB_VARIABLE","GDB_ASSIGNMENT",
"','","start","value","access_name","expression_list","maybe_expression_list",
"primitive_value_lparen","rparen","primitive_value","value_name","literal","tuple_element",
"named_record_element","tuple_elements","maybe_tuple_elements","tuple","@1",
"@2","slice","expression_conversion","parenthesised_expression","expression",
"conditional_expression","then_alternative","else_alternative","operand_0","operand_1",
"operand_2","operand_3","operand_4","operand_5","operand_6","single_assignment_action",
"value_built_in_routine_call","mode_argument","mode_name",""
};
#endif

static const short yyr1[] = {     0,
    71,    71,    72,    73,    73,    73,    73,    74,    74,    75,
    75,    76,    77,    78,    78,    78,    78,    78,    78,    78,
    78,    78,    78,    78,    78,    79,    80,    80,    80,    80,
    80,    80,    80,    81,    81,    82,    82,    83,    83,    84,
    84,    86,    85,    87,    85,    88,    88,    89,    89,    90,
    91,    91,    91,    92,    93,    94,    94,    95,    95,    95,
    95,    96,    96,    96,    97,    97,    97,    97,    97,    97,
    97,    97,    98,    98,    98,    98,    99,    99,    99,    99,
    99,   100,   100,   100,   100,   101,   101,   101,   102,   103,
   103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
   103,   103,   103,   103,   104,   105
};

static const short yyr2[] = {     0,
     1,     1,     1,     1,     1,     1,     1,     1,     3,     0,
     1,     2,     1,     1,     3,     2,     2,     3,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     3,     3,     1,     3,     1,
     0,     0,     4,     0,     5,     5,     5,     2,     5,     3,
     1,     1,     1,     5,     2,     2,     4,     1,     3,     3,
     3,     1,     3,     3,     1,     3,     3,     3,     3,     3,
     3,     3,     1,     3,     3,     3,     1,     3,     3,     3,
     3,     1,     2,     2,     2,     2,     2,     1,     3,     4,
     4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
     4,     4,     4,     4,     1,     1
};

static const short yydefact[] = {     0,
    27,    28,    29,    30,    26,     4,    31,    32,    33,   106,
     0,     0,     0,     0,    42,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     6,     5,     7,     1,    14,    10,    88,    19,    20,    21,
    22,    23,    25,     3,    53,    51,    58,    62,    65,    73,
    77,    82,    52,    24,     2,    88,    25,    83,     0,    84,
    86,    87,    41,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    11,     0,
     8,    16,    17,    12,     0,    85,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    44,    48,     0,    38,    35,
    40,     0,    34,    50,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,   105,     0,     0,     0,
     0,     0,     0,     0,    13,    15,     0,     0,    18,    89,
    59,    60,    61,    63,    64,    66,    67,    68,    69,    70,
    71,    72,    74,    75,    76,    78,    79,    80,    81,    41,
     0,     0,     0,    43,    55,     0,     0,     0,    90,    91,
    92,    94,    95,    96,    97,    93,    98,    99,   103,   101,
   102,   100,   104,     0,     9,     0,     0,     0,    37,    36,
    39,    56,     0,    54,    49,    46,    47,    45,     0,    57,
     0,     0,     0
};

static const short yydefgoto[] = {   201,
    34,    35,    79,    80,    36,   136,    37,    38,    39,   109,
   110,   111,   112,    40,    63,   160,    41,    42,    43,   113,
    45,   116,   168,    46,    47,    48,    49,    50,    51,    52,
    53,    54,   126,    59
};

static const short yypact[] = {   187,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   319,   319,   346,   187,-32768,   187,   187,   -31,   -12,    24,
    49,    50,    52,    73,    79,    91,    92,    94,    95,   106,
-32768,-32768,-32768,-32768,-32768,   187,    -9,-32768,-32768,-32768,
-32768,-32768,    46,-32768,-32768,    21,    30,    -1,    31,   -23,
-32768,-32768,-32768,-32768,   -37,    -7,-32768,-32768,   -37,-32768,
    -7,-32768,   121,    16,   -14,   187,   187,   187,   187,   187,
   187,   187,   187,   187,   187,   187,   187,   107,   -34,   108,
    -6,-32768,    26,-32768,   187,-32768,   253,   253,   253,   253,
   253,   253,   253,   253,   253,   253,   253,   253,   253,   253,
   253,   253,   253,   253,   253,-32768,-32768,    -8,-32768,-32768,
    25,   112,-32768,-32768,   187,   -33,   111,   113,   114,   122,
   126,   127,   155,   156,   157,   158,   -37,   159,   160,   161,
   162,   163,    26,   187,-32768,-32768,   187,   187,-32768,-32768,
    30,    30,    30,    -1,    -1,    31,    31,    31,    31,    31,
    31,    31,   -23,   -23,   -23,-32768,-32768,-32768,-32768,   121,
   187,   138,   121,-32768,-32768,   187,   187,   116,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   165,-32768,   108,   108,   167,-32768,-32768,
-32768,-32768,   -14,-32768,-32768,-32768,-32768,-32768,   -33,-32768,
   211,   212,-32768
};

static const short yypgoto[] = {-32768,
    72,-32768,-32768,-32768,-32768,  -128,    10,-32768,   170,    53,
    55,-32768,    54,-32768,-32768,-32768,-32768,-32768,   -11,     3,
-32768,    22,    19,-32768,   -42,   -25,    51,   -10,    17,    70,
-32768,-32768,    11,     9
};


#define	YYLAST		414


static const short yytable[] = {    57,
    57,    57,    44,    82,   106,    82,    16,   161,    55,   137,
   102,   103,    66,   104,   105,   166,    62,   167,    64,    65,
    56,    56,    61,    92,    93,    94,    95,    96,    97,    98,
    83,    67,    83,   115,    84,   134,    84,    10,    81,   138,
    87,    88,    89,   107,   141,   142,   143,   107,     1,     2,
     3,     4,    90,    91,     7,     8,     9,   196,   197,    85,
   114,   162,    99,   100,   144,   145,   101,    68,   117,   118,
   119,   120,   121,   122,   123,   124,   125,   128,   130,   132,
    58,    60,   127,   127,   127,   129,   131,    44,   153,   154,
   155,   139,    69,    70,   163,    71,    56,    56,    56,    56,
    56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
    56,    56,    56,    56,    56,   107,    72,   165,   156,   157,
   158,   159,    73,     1,     2,     3,     4,     5,     6,     7,
     8,     9,    10,   108,    74,    75,   185,    76,    77,   186,
   187,   184,   146,   147,   148,   149,   150,   151,   152,    78,
   108,   133,   135,    11,   164,   169,   140,   170,   171,    12,
    13,    14,    15,   189,    16,   194,   172,    17,   192,   193,
   173,   174,   195,    18,    19,    20,    21,    22,    23,    24,
    25,    26,    27,    28,    29,    30,    31,    32,    33,     1,
     2,     3,     4,     5,     6,     7,     8,     9,    10,   175,
   176,   177,   178,   179,   180,   181,   182,   183,    16,   198,
   202,   203,    86,   188,   199,   191,   190,   200,     0,    11,
     0,     0,     0,     0,     0,    12,    13,    14,    15,     0,
    16,     0,     0,    17,     0,     0,     0,     0,     0,    18,
    19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
    29,    30,    31,    32,    33,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    11,     0,     0,     0,     0,
     0,    12,    13,    14,    15,     0,    16,     0,     0,     0,
     0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
    23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
    33,     1,     2,     3,     4,     5,     6,     7,     8,     9,
    10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     1,     2,
     3,     4,     5,     6,     7,     8,     9,    10,    13,    14,
    15,     0,    16,     0,     0,     0,     0,     0,     0,     0,
     0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32,    33,    15,     0,    16,
     0,     0,     0,     0,     0,     0,     0,     0,    18,    19,
    20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
    30,    31,    32,    33
};

static const short yycheck[] = {    11,
    12,    13,     0,    13,    42,    13,    44,    16,     0,    16,
    34,    35,    44,    37,    38,    49,    14,    51,    16,    17,
    11,    12,    13,    25,    26,    27,    28,    29,    30,    31,
    40,    44,    40,    48,    44,    70,    44,    12,    36,    46,
    20,    21,    22,    55,    87,    88,    89,    59,     3,     4,
     5,     6,    23,    24,     9,    10,    11,   186,   187,    69,
    45,    70,    32,    33,    90,    91,    36,    44,    66,    67,
    68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
    11,    12,    74,    75,    76,    75,    76,    85,    99,   100,
   101,    83,    44,    44,    70,    44,    87,    88,    89,    90,
    91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
   101,   102,   103,   104,   105,   127,    44,   115,   102,   103,
   104,   105,    44,     3,     4,     5,     6,     7,     8,     9,
    10,    11,    12,    13,    44,    44,   134,    44,    44,   137,
   138,   133,    92,    93,    94,    95,    96,    97,    98,    44,
    13,    45,    45,    33,    43,    45,    85,    45,    45,    39,
    40,    41,    42,   161,    44,    50,    45,    47,   166,   167,
    45,    45,   184,    53,    54,    55,    56,    57,    58,    59,
    60,    61,    62,    63,    64,    65,    66,    67,    68,     3,
     4,     5,     6,     7,     8,     9,    10,    11,    12,    45,
    45,    45,    45,    45,    45,    45,    45,    45,    44,    43,
     0,     0,    43,   160,   193,   163,   162,   199,    -1,    33,
    -1,    -1,    -1,    -1,    -1,    39,    40,    41,    42,    -1,
    44,    -1,    -1,    47,    -1,    -1,    -1,    -1,    -1,    53,
    54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
    64,    65,    66,    67,    68,     3,     4,     5,     6,     7,
     8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,
    -1,    39,    40,    41,    42,    -1,    44,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    53,    54,    55,    56,    57,
    58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
    68,     3,     4,     5,     6,     7,     8,     9,    10,    11,
    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
     5,     6,     7,     8,     9,    10,    11,    12,    40,    41,
    42,    -1,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    53,    54,    55,    56,    57,    58,    59,    60,    61,
    62,    63,    64,    65,    66,    67,    68,    42,    -1,    44,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
    55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
    65,    66,    67,    68
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/unsupported/share/bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
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
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
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

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(FROM,TO,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (from, to, count)
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
__yy_memcpy (char *from, char *to, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 192 "/usr/unsupported/share/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#else
#define YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#endif

int
yyparse(YYPARSE_PARAM)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to xreallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
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
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

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
      /* Give user a chance to xreallocate the stack */
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
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

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
      __yy_memcpy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
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

  goto yybackup;
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
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
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
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 271 "./ch-exp.y"
{ ;
    break;}
case 2:
#line 273 "./ch-exp.y"
{ write_exp_elt_opcode(OP_TYPE);
			  write_exp_elt_type(yyvsp[0].tsym.type);
			  write_exp_elt_opcode(OP_TYPE);;
    break;}
case 4:
#line 288 "./ch-exp.y"
{
			  write_exp_elt_opcode (OP_VAR_VALUE);
			  write_exp_elt_block (NULL);
			  write_exp_elt_sym (yyvsp[0].ssym.sym);
			  write_exp_elt_opcode (OP_VAR_VALUE);
			;
    break;}
case 5:
#line 295 "./ch-exp.y"
{
			  write_exp_elt_opcode (OP_LAST);
			  write_exp_elt_longcst (yyvsp[0].lval);
			  write_exp_elt_opcode (OP_LAST); 
			;
    break;}
case 6:
#line 301 "./ch-exp.y"
{
			  write_exp_elt_opcode (OP_REGISTER);
			  write_exp_elt_longcst (yyvsp[0].lval);
			  write_exp_elt_opcode (OP_REGISTER); 
			;
    break;}
case 7:
#line 307 "./ch-exp.y"
{
			  write_exp_elt_opcode (OP_INTERNALVAR);
			  write_exp_elt_intern (yyvsp[0].ivar);
			  write_exp_elt_opcode (OP_INTERNALVAR); 
			;
    break;}
case 8:
#line 317 "./ch-exp.y"
{
			  arglist_len = 1;
			;
    break;}
case 9:
#line 321 "./ch-exp.y"
{
			  arglist_len++;
			;
    break;}
case 10:
#line 327 "./ch-exp.y"
{
			  arglist_len = 0;
			;
    break;}
case 12:
#line 339 "./ch-exp.y"
{ start_arglist (); ;
    break;}
case 13:
#line 343 "./ch-exp.y"
{ yyval.lval = end_arglist (); ;
    break;}
case 15:
#line 349 "./ch-exp.y"
{
			  write_exp_elt_opcode (MULTI_SUBSCRIPT);
			  write_exp_elt_longcst (yyvsp[0].lval);
			  write_exp_elt_opcode (MULTI_SUBSCRIPT);
			;
    break;}
case 16:
#line 355 "./ch-exp.y"
{ write_exp_elt_opcode (STRUCTOP_STRUCT);
			  write_exp_string (yyvsp[0].sval);
			  write_exp_elt_opcode (STRUCTOP_STRUCT);
			;
    break;}
case 17:
#line 360 "./ch-exp.y"
{
			  write_exp_elt_opcode (UNOP_IND);
			;
    break;}
case 18:
#line 364 "./ch-exp.y"
{
			  write_exp_elt_opcode (UNOP_CAST);
			  write_exp_elt_type (lookup_pointer_type (yyvsp[0].tsym.type));
			  write_exp_elt_opcode (UNOP_CAST);
			  write_exp_elt_opcode (UNOP_IND);
			;
    break;}
case 26:
#line 388 "./ch-exp.y"
{
			  write_exp_elt_opcode (OP_VAR_VALUE);
			  write_exp_elt_block (NULL);
			  write_exp_elt_sym (yyvsp[0].ssym.sym);
			  write_exp_elt_opcode (OP_VAR_VALUE);
			;
    break;}
case 27:
#line 399 "./ch-exp.y"
{
			  write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (yyvsp[0].typed_val.type);
			  write_exp_elt_longcst ((LONGEST) (yyvsp[0].typed_val.val));
			  write_exp_elt_opcode (OP_LONG);
			;
    break;}
case 28:
#line 406 "./ch-exp.y"
{
			  write_exp_elt_opcode (OP_BOOL);
			  write_exp_elt_longcst ((LONGEST) yyvsp[0].ulval);
			  write_exp_elt_opcode (OP_BOOL);
			;
    break;}
case 29:
#line 412 "./ch-exp.y"
{
			  write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (yyvsp[0].typed_val.type);
			  write_exp_elt_longcst ((LONGEST) (yyvsp[0].typed_val.val));
			  write_exp_elt_opcode (OP_LONG);
			;
    break;}
case 30:
#line 419 "./ch-exp.y"
{
			  write_exp_elt_opcode (OP_DOUBLE);
			  write_exp_elt_type (builtin_type_double);
			  write_exp_elt_dblcst (yyvsp[0].dval);
			  write_exp_elt_opcode (OP_DOUBLE);
			;
    break;}
case 31:
#line 426 "./ch-exp.y"
{
			  struct type *void_ptr_type
			    = lookup_pointer_type (builtin_type_void);
			  write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (void_ptr_type);
			  write_exp_elt_longcst (0);
			  write_exp_elt_opcode (OP_LONG);
			;
    break;}
case 32:
#line 435 "./ch-exp.y"
{
			  write_exp_elt_opcode (OP_STRING);
			  write_exp_string (yyvsp[0].sval);
			  write_exp_elt_opcode (OP_STRING);
			;
    break;}
case 33:
#line 441 "./ch-exp.y"
{
			  write_exp_elt_opcode (OP_BITSTRING);
			  write_exp_bitstring (yyvsp[0].sval);
			  write_exp_elt_opcode (OP_BITSTRING);
			;
    break;}
case 36:
#line 455 "./ch-exp.y"
{ write_exp_elt_opcode (OP_LABELED);
			  write_exp_string (yyvsp[-2].sval);
			  write_exp_elt_opcode (OP_LABELED);
			;
    break;}
case 37:
#line 460 "./ch-exp.y"
{ write_exp_elt_opcode (OP_LABELED);
			  write_exp_string (yyvsp[-2].sval);
			  write_exp_elt_opcode (OP_LABELED);
			;
    break;}
case 38:
#line 467 "./ch-exp.y"
{
			  arglist_len = 1;
			;
    break;}
case 39:
#line 471 "./ch-exp.y"
{
			  arglist_len++;
			;
    break;}
case 42:
#line 481 "./ch-exp.y"
{ start_arglist (); ;
    break;}
case 43:
#line 483 "./ch-exp.y"
{
			  write_exp_elt_opcode (OP_ARRAY);
			  write_exp_elt_longcst ((LONGEST) 0);
			  write_exp_elt_longcst ((LONGEST) end_arglist () - 1);
			  write_exp_elt_opcode (OP_ARRAY);
			;
    break;}
case 44:
#line 491 "./ch-exp.y"
{ start_arglist (); ;
    break;}
case 45:
#line 493 "./ch-exp.y"
{
			  write_exp_elt_opcode (OP_ARRAY);
			  write_exp_elt_longcst ((LONGEST) 0);
			  write_exp_elt_longcst ((LONGEST) end_arglist () - 1);
			  write_exp_elt_opcode (OP_ARRAY);

			  write_exp_elt_opcode (UNOP_CAST);
			  write_exp_elt_type (yyvsp[-4].tsym.type);
			  write_exp_elt_opcode (UNOP_CAST);
			;
    break;}
case 46:
#line 510 "./ch-exp.y"
{
			  write_exp_elt_opcode (TERNOP_SLICE);
			;
    break;}
case 47:
#line 514 "./ch-exp.y"
{
			  write_exp_elt_opcode (TERNOP_SLICE_COUNT);
			;
    break;}
case 48:
#line 522 "./ch-exp.y"
{
			  write_exp_elt_opcode (UNOP_CAST);
			  write_exp_elt_type (yyvsp[-1].tsym.type);
			  write_exp_elt_opcode (UNOP_CAST);
			;
    break;}
case 49:
#line 530 "./ch-exp.y"
{
			  struct type *range_type
			    = create_range_type ((struct type *) NULL,
						 builtin_type_int, 0, 0);
			  struct type *array_type
			    = create_array_type ((struct type *) NULL,
						 yyvsp[-1].tsym.type, range_type);
			  TYPE_ARRAY_UPPER_BOUND_TYPE(array_type)
			    = BOUND_CANNOT_BE_DETERMINED;
			  write_exp_elt_opcode (UNOP_CAST);
			  write_exp_elt_type (array_type);
			  write_exp_elt_opcode (UNOP_CAST);
			;
    break;}
case 54:
#line 558 "./ch-exp.y"
{ write_exp_elt_opcode (TERNOP_COND); ;
    break;}
case 57:
#line 570 "./ch-exp.y"
{ write_exp_elt_opcode (TERNOP_COND); ;
    break;}
case 59:
#line 577 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_BITWISE_IOR);
			;
    break;}
case 60:
#line 581 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_LOGICAL_OR);
			;
    break;}
case 61:
#line 585 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_BITWISE_XOR);
			;
    break;}
case 63:
#line 594 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_BITWISE_AND);
			;
    break;}
case 64:
#line 598 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_LOGICAL_AND);
			;
    break;}
case 66:
#line 607 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_EQUAL);
			;
    break;}
case 67:
#line 611 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_NOTEQUAL);
			;
    break;}
case 68:
#line 615 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_GTR);
			;
    break;}
case 69:
#line 619 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_GEQ);
			;
    break;}
case 70:
#line 623 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_LESS);
			;
    break;}
case 71:
#line 627 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_LEQ);
			;
    break;}
case 72:
#line 631 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_IN);
			;
    break;}
case 74:
#line 641 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_ADD);
			;
    break;}
case 75:
#line 645 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_SUB);
			;
    break;}
case 76:
#line 649 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_CONCAT);
			;
    break;}
case 78:
#line 658 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_MUL);
			;
    break;}
case 79:
#line 662 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_DIV);
			;
    break;}
case 80:
#line 666 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_MOD);
			;
    break;}
case 81:
#line 670 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_REM);
			;
    break;}
case 83:
#line 679 "./ch-exp.y"
{
			  write_exp_elt_opcode (UNOP_NEG);
			;
    break;}
case 84:
#line 683 "./ch-exp.y"
{
			  write_exp_elt_opcode (UNOP_LOGICAL_NOT);
			;
    break;}
case 85:
#line 689 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_CONCAT);
			;
    break;}
case 86:
#line 697 "./ch-exp.y"
{
			  write_exp_elt_opcode (UNOP_ADDR);
			;
    break;}
case 87:
#line 701 "./ch-exp.y"
{ error ("not implemented:  RECEIVE expression"); ;
    break;}
case 89:
#line 710 "./ch-exp.y"
{
			  write_exp_elt_opcode (BINOP_ASSIGN);
			;
    break;}
case 90:
#line 719 "./ch-exp.y"
{
			  write_exp_elt_opcode (UNOP_CAST);
			  write_exp_elt_type (builtin_type_int);
			  write_exp_elt_opcode (UNOP_CAST);
			;
    break;}
case 91:
#line 725 "./ch-exp.y"
{ error ("not implemented:  PRED builtin function"); ;
    break;}
case 92:
#line 727 "./ch-exp.y"
{ error ("not implemented:  SUCC builtin function"); ;
    break;}
case 93:
#line 729 "./ch-exp.y"
{ write_exp_elt_opcode (UNOP_ADDR); ;
    break;}
case 94:
#line 731 "./ch-exp.y"
{ error ("not implemented:  ABS builtin function"); ;
    break;}
case 95:
#line 733 "./ch-exp.y"
{ error ("not implemented:  CARD builtin function"); ;
    break;}
case 96:
#line 735 "./ch-exp.y"
{ error ("not implemented:  MAX builtin function"); ;
    break;}
case 97:
#line 737 "./ch-exp.y"
{ error ("not implemented:  MIN builtin function"); ;
    break;}
case 98:
#line 739 "./ch-exp.y"
{ write_exp_elt_opcode (UNOP_SIZEOF); ;
    break;}
case 99:
#line 741 "./ch-exp.y"
{ write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (builtin_type_int);
			  write_exp_elt_longcst ((LONGEST) TYPE_LENGTH (yyvsp[-1].tval));
			  write_exp_elt_opcode (OP_LONG); ;
    break;}
case 100:
#line 746 "./ch-exp.y"
{ write_lower_upper_value (UNOP_LOWER, yyvsp[-1].tval); ;
    break;}
case 101:
#line 748 "./ch-exp.y"
{ write_lower_upper_value (UNOP_UPPER, yyvsp[-1].tval); ;
    break;}
case 102:
#line 750 "./ch-exp.y"
{ write_exp_elt_opcode (UNOP_LOWER); ;
    break;}
case 103:
#line 752 "./ch-exp.y"
{ write_exp_elt_opcode (UNOP_UPPER); ;
    break;}
case 104:
#line 754 "./ch-exp.y"
{ write_exp_elt_opcode (UNOP_LENGTH); ;
    break;}
case 105:
#line 758 "./ch-exp.y"
{
			  yyval.tval = yyvsp[0].tsym.type;
			;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 487 "/usr/unsupported/share/bison.simple"

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
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) xmalloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
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
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
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
#line 774 "./ch-exp.y"


/* Implementation of a dynamically expandable buffer for processing input
   characters acquired through lexptr and building a value to return in
   yylval. */

static char *tempbuf;		/* Current buffer contents */
static int tempbufsize;		/* Size of allocated buffer */
static int tempbufindex;	/* Current index into buffer */

#define GROWBY_MIN_SIZE 64	/* Minimum amount to grow buffer by */

#define CHECKBUF(size) \
  do { \
    if (tempbufindex + (size) >= tempbufsize) \
      { \
	growbuf_by_size (size); \
      } \
  } while (0);

/* Grow the static temp buffer if necessary, including allocating the first one
   on demand. */

static void
growbuf_by_size (count)
     int count;
{
  int growby;

  growby = max (count, GROWBY_MIN_SIZE);
  tempbufsize += growby;
  if (tempbuf == NULL)
    {
      tempbuf = (char *) xmalloc (tempbufsize);
    }
  else
    {
      tempbuf = (char *) xrealloc (tempbuf, tempbufsize);
    }
}

/* Try to consume a simple name string token.  If successful, returns
   a pointer to a nullbyte terminated copy of the name that can be used
   in symbol table lookups.  If not successful, returns NULL. */

static char *
match_simple_name_string ()
{
  char *tokptr = lexptr;

  if (isalpha (*tokptr) || *tokptr == '_')
    {
      char *result;
      do {
	tokptr++;
      } while (isalnum (*tokptr) || (*tokptr == '_'));
      yylval.sval.ptr = lexptr;
      yylval.sval.length = tokptr - lexptr;
      lexptr = tokptr;
      result = copy_name (yylval.sval);
      return result;
    }
  return (NULL);
}

/* Start looking for a value composed of valid digits as set by the base
   in use.  Note that '_' characters are valid anywhere, in any quantity,
   and are simply ignored.  Since we must find at least one valid digit,
   or reject this token as an integer literal, we keep track of how many
   digits we have encountered. */
  
static int
decode_integer_value (base, tokptrptr, ivalptr)
  int base;
  char **tokptrptr;
  int *ivalptr;
{
  char *tokptr = *tokptrptr;
  int temp;
  int digits = 0;

  while (*tokptr != '\0')
    {
      temp = *tokptr;
      if (isupper (temp))
        temp = tolower (temp);
      tokptr++;
      switch (temp)
	{
	case '_':
	  continue;
	case '0':  case '1':  case '2':  case '3':  case '4':
	case '5':  case '6':  case '7':  case '8':  case '9':
	  temp -= '0';
	  break;
	case 'a':  case 'b':  case 'c':  case 'd':  case 'e': case 'f':
	  temp -= 'a';
	  temp += 10;
	  break;
	default:
	  temp = base;
	  break;
	}
      if (temp < base)
	{
	  digits++;
	  *ivalptr *= base;
	  *ivalptr += temp;
	}
      else
	{
	  /* Found something not in domain for current base. */
	  tokptr--;	/* Unconsume what gave us indigestion. */
	  break;
	}
    }
  
  /* If we didn't find any digits, then we don't have a valid integer
     value, so reject the entire token.  Otherwise, update the lexical
     scan pointer, and return non-zero for success. */
  
  if (digits == 0)
    {
      return (0);
    }
  else
    {
      *tokptrptr = tokptr;
      return (1);
    }
}

static int
decode_integer_literal (valptr, tokptrptr)
  int *valptr;
  char **tokptrptr;
{
  char *tokptr = *tokptrptr;
  int base = 0;
  int ival = 0;
  int explicit_base = 0;
  
  /* Look for an explicit base specifier, which is optional. */
  
  switch (*tokptr)
    {
    case 'd':
    case 'D':
      explicit_base++;
      base = 10;
      tokptr++;
      break;
    case 'b':
    case 'B':
      explicit_base++;
      base = 2;
      tokptr++;
      break;
    case 'h':
    case 'H':
      explicit_base++;
      base = 16;
      tokptr++;
      break;
    case 'o':
    case 'O':
      explicit_base++;
      base = 8;
      tokptr++;
      break;
    default:
      base = 10;
      break;
    }
  
  /* If we found an explicit base ensure that the character after the
     explicit base is a single quote. */
  
  if (explicit_base && (*tokptr++ != '\''))
    {
      return (0);
    }
  
  /* Attempt to decode whatever follows as an integer value in the
     indicated base, updating the token pointer in the process and
     computing the value into ival.  Also, if we have an explicit
     base, then the next character must not be a single quote, or we
     have a bitstring literal, so reject the entire token in this case.
     Otherwise, update the lexical scan pointer, and return non-zero
     for success. */

  if (!decode_integer_value (base, &tokptr, &ival))
    {
      return (0);
    }
  else if (explicit_base && (*tokptr == '\''))
    {
      return (0);
    }
  else
    {
      *valptr = ival;
      *tokptrptr = tokptr;
      return (1);
    }
}

/*  If it wasn't for the fact that floating point values can contain '_'
    characters, we could just let strtod do all the hard work by letting it
    try to consume as much of the current token buffer as possible and
    find a legal conversion.  Unfortunately we need to filter out the '_'
    characters before calling strtod, which we do by copying the other
    legal chars to a local buffer to be converted.  However since we also
    need to keep track of where the last unconsumed character in the input
    buffer is, we have transfer only as many characters as may compose a
    legal floating point value. */
    
static int
match_float_literal ()
{
  char *tokptr = lexptr;
  char *buf;
  char *copy;
  double dval;
  extern double strtod ();
  
  /* Make local buffer in which to build the string to convert.  This is
     required because underscores are valid in chill floating point numbers
     but not in the string passed to strtod to convert.  The string will be
     no longer than our input string. */
     
  copy = buf = (char *) alloca (strlen (tokptr) + 1);

  /* Transfer all leading digits to the conversion buffer, discarding any
     underscores. */

  while (isdigit (*tokptr) || *tokptr == '_')
    {
      if (*tokptr != '_')
	{
	  *copy++ = *tokptr;
	}
      tokptr++;
    }

  /* Now accept either a '.', or one of [eEdD].  Dot is legal regardless
     of whether we found any leading digits, and we simply accept it and
     continue on to look for the fractional part and/or exponent.  One of
     [eEdD] is legal only if we have seen digits, and means that there
     is no fractional part.  If we find neither of these, then this is
     not a floating point number, so return failure. */

  switch (*tokptr++)
    {
      case '.':
        /* Accept and then look for fractional part and/or exponent. */
	*copy++ = '.';
	break;

      case 'e':
      case 'E':
      case 'd':
      case 'D':
	if (copy == buf)
	  {
	    return (0);
	  }
	*copy++ = 'e';
	goto collect_exponent;
	break;

      default:
	return (0);
        break;
    }

  /* We found a '.', copy any fractional digits to the conversion buffer, up
     to the first nondigit, non-underscore character. */

  while (isdigit (*tokptr) || *tokptr == '_')
    {
      if (*tokptr != '_')
	{
	  *copy++ = *tokptr;
	}
      tokptr++;
    }

  /* Look for an exponent, which must start with one of [eEdD].  If none
     is found, jump directly to trying to convert what we have collected
     so far. */

  switch (*tokptr)
    {
      case 'e':
      case 'E':
      case 'd':
      case 'D':
	*copy++ = 'e';
	tokptr++;
	break;
      default:
	goto convert_float;
	break;
    }

  /* Accept an optional '-' or '+' following one of [eEdD]. */

  collect_exponent:
  if (*tokptr == '+' || *tokptr == '-')
    {
      *copy++ = *tokptr++;
    }

  /* Now copy an exponent into the conversion buffer.  Note that at the 
     moment underscores are *not* allowed in exponents. */

  while (isdigit (*tokptr))
    {
      *copy++ = *tokptr++;
    }

  /* If we transfered any chars to the conversion buffer, try to interpret its
     contents as a floating point value.  If any characters remain, then we
     must not have a valid floating point string. */

  convert_float:
  *copy = '\0';
  if (copy != buf)
      {
        dval = strtod (buf, &copy);
        if (*copy == '\0')
	  {
	    yylval.dval = dval;
	    lexptr = tokptr;
	    return (FLOAT_LITERAL);
	  }
      }
  return (0);
}

/* Recognize a string literal.  A string literal is a sequence
   of characters enclosed in matching single or double quotes, except that
   a single character inside single quotes is a character literal, which
   we reject as a string literal.  To embed the terminator character inside
   a string, it is simply doubled (I.E. "this""is""one""string") */

static int
match_string_literal ()
{
  char *tokptr = lexptr;

  for (tempbufindex = 0, tokptr++; *tokptr != '\0'; tokptr++)
    {
      CHECKBUF (1);
      if (*tokptr == *lexptr)
	{
	  if (*(tokptr + 1) == *lexptr)
	    {
	      tokptr++;
	    }
	  else
	    {
	      break;
	    }
	}
      tempbuf[tempbufindex++] = *tokptr;
    }
  if (*tokptr == '\0'					/* no terminator */
      || (tempbufindex == 1 && *tokptr == '\''))	/* char literal */
    {
      return (0);
    }
  else
    {
      tempbuf[tempbufindex] = '\0';
      yylval.sval.ptr = tempbuf;
      yylval.sval.length = tempbufindex;
      lexptr = ++tokptr;
      return (CHARACTER_STRING_LITERAL);
    }
}

/* Recognize a character literal.  A character literal is single character
   or a control sequence, enclosed in single quotes.  A control sequence
   is a comma separated list of one or more integer literals, enclosed
   in parenthesis and introduced with a circumflex character.

   EX:  'a'  '^(7)'  '^(7,8)'

   As a GNU chill extension, the syntax C'xx' is also recognized as a 
   character literal, where xx is a hex value for the character.

   Note that more than a single character, enclosed in single quotes, is
   a string literal.

   Also note that the control sequence form is not in GNU Chill since it
   is ambiguous with the string literal form using single quotes.  I.E.
   is '^(7)' a character literal or a string literal.  In theory it it
   possible to tell by context, but GNU Chill doesn't accept the control
   sequence form, so neither do we (for now the code is disabled).

   Returns CHARACTER_LITERAL if a match is found.
   */

static int
match_character_literal ()
{
  char *tokptr = lexptr;
  int ival = 0;
  
  if ((*tokptr == 'c' || *tokptr == 'C') && (*(tokptr + 1) == '\''))
    {
      /* We have a GNU chill extension form, so skip the leading "C'",
	 decode the hex value, and then ensure that we have a trailing
	 single quote character. */
      tokptr += 2;
      if (!decode_integer_value (16, &tokptr, &ival) || (*tokptr != '\''))
	{
	  return (0);
	}
      tokptr++;
    }
  else if (*tokptr == '\'')
    {
      tokptr++;

      /* Determine which form we have, either a control sequence or the
	 single character form. */
      
      if ((*tokptr == '^') && (*(tokptr + 1) == '('))
	{
#if 0     /* Disable, see note above. -fnf */
	  /* Match and decode a control sequence.  Return zero if we don't
	     find a valid integer literal, or if the next unconsumed character
	     after the integer literal is not the trailing ')'.
	     FIXME:  We currently don't handle the multiple integer literal
	     form. */
	  tokptr += 2;
	  if (!decode_integer_literal (&ival, &tokptr) || (*tokptr++ != ')'))
	    {
	      return (0);
	    }
#else
	  return (0);
#endif
	}
      else
	{
	  ival = *tokptr++;
	}
      
      /* The trailing quote has not yet been consumed.  If we don't find
	 it, then we have no match. */
      
      if (*tokptr++ != '\'')
	{
	  return (0);
	}
    }
  else
    {
      /* Not a character literal. */
      return (0);
    }
  yylval.typed_val.val = ival;
  yylval.typed_val.type = builtin_type_chill_char;
  lexptr = tokptr;
  return (CHARACTER_LITERAL);
}

/* Recognize an integer literal, as specified in Z.200 sec 5.2.4.2.
   Note that according to 5.2.4.2, a single "_" is also a valid integer
   literal, however GNU-chill requires there to be at least one "digit"
   in any integer literal. */

static int
match_integer_literal ()
{
  char *tokptr = lexptr;
  int ival;
  
  if (!decode_integer_literal (&ival, &tokptr))
    {
      return (0);
    }
  else 
    {
      yylval.typed_val.val = ival;
      yylval.typed_val.type = builtin_type_int;
      lexptr = tokptr;
      return (INTEGER_LITERAL);
    }
}

/* Recognize a bit-string literal, as specified in Z.200 sec 5.2.4.8
   Note that according to 5.2.4.8, a single "_" is also a valid bit-string
   literal, however GNU-chill requires there to be at least one "digit"
   in any bit-string literal. */

static int
match_bitstring_literal ()
{
  register char *tokptr = lexptr;
  int bitoffset = 0;
  int bitcount = 0;
  int bits_per_char;
  int digit;
  
  tempbufindex = 0;
  CHECKBUF (1);
  tempbuf[0] = 0;

  /* Look for the required explicit base specifier. */
  
  switch (*tokptr++)
    {
    case 'b':
    case 'B':
      bits_per_char = 1;
      break;
    case 'o':
    case 'O':
      bits_per_char = 3;
      break;
    case 'h':
    case 'H':
      bits_per_char = 4;
      break;
    default:
      return (0);
      break;
    }

  /* Ensure that the character after the explicit base is a single quote. */
  
  if (*tokptr++ != '\'')
    {
      return (0);
    }
  
  while (*tokptr != '\0' && *tokptr != '\'')
    {
      digit = *tokptr;
      if (isupper (digit))
        digit = tolower (digit);
      tokptr++;
      switch (digit)
	{
	  case '_':
	    continue;
	  case '0':  case '1':  case '2':  case '3':  case '4':
	  case '5':  case '6':  case '7':  case '8':  case '9':
	    digit -= '0';
	    break;
	  case 'a':  case 'b':  case 'c':  case 'd':  case 'e': case 'f':
	    digit -= 'a';
	    digit += 10;
	    break;
	  default:
	    return (0);
	    break;
	}
      if (digit >= 1 << bits_per_char)
	{
	  /* Found something not in domain for current base. */
	  return (0);
	}
      else
	{
	  /* Extract bits from digit, packing them into the bitstring byte. */
	  int k = TARGET_BYTE_ORDER == BIG_ENDIAN ? bits_per_char - 1 : 0;
	  for (; TARGET_BYTE_ORDER == BIG_ENDIAN ? k >= 0 : k < bits_per_char;
	       TARGET_BYTE_ORDER == BIG_ENDIAN ? k-- : k++)
	    {
	      bitcount++;
	      if (digit & (1 << k))
		{
		  tempbuf[tempbufindex] |=
		    (TARGET_BYTE_ORDER == BIG_ENDIAN)
		      ? (1 << (HOST_CHAR_BIT - 1 - bitoffset))
			: (1 << bitoffset);
		}
	      bitoffset++;
	      if (bitoffset == HOST_CHAR_BIT)
		{
		  bitoffset = 0;
		  tempbufindex++;
		  CHECKBUF(1);
		  tempbuf[tempbufindex] = 0;
		}
	    }
	}
    }
  
  /* Verify that we consumed everything up to the trailing single quote,
     and that we found some bits (IE not just underbars). */

  if (*tokptr++ != '\'')
    {
      return (0);
    }
  else 
    {
      yylval.sval.ptr = tempbuf;
      yylval.sval.length = bitcount;
      lexptr = tokptr;
      return (BIT_STRING_LITERAL);
    }
}

/* Recognize tokens that start with '$'.  These include:

	$regname	A native register name or a "standard
			register name".
			Return token GDB_REGNAME.

	$variable	A convenience variable with a name chosen
			by the user.
			Return token GDB_VARIABLE.

	$digits		Value history with index <digits>, starting
			from the first value which has index 1.
			Return GDB_LAST.

	$$digits	Value history with index <digits> relative
			to the last value.  I.E. $$0 is the last
			value, $$1 is the one previous to that, $$2
			is the one previous to $$1, etc.
			Return token GDB_LAST.

	$ | $0 | $$0	The last value in the value history.
			Return token GDB_LAST.

	$$		An abbreviation for the second to the last
			value in the value history, I.E. $$1
			Return token GDB_LAST.

    Note that we currently assume that register names and convenience
    variables follow the convention of starting with a letter or '_'.

   */

static int
match_dollar_tokens ()
{
  char *tokptr;
  int regno;
  int namelength;
  int negate;
  int ival;

  /* We will always have a successful match, even if it is just for
     a single '$', the abbreviation for $$0.  So advance lexptr. */

  tokptr = ++lexptr;

  if (*tokptr == '_' || isalpha (*tokptr))
    {
      /* Look for a match with a native register name, usually something
	 like "r0" for example. */

      for (regno = 0; regno < NUM_REGS; regno++)
	{
	  namelength = strlen (reg_names[regno]);
	  if (STREQN (tokptr, reg_names[regno], namelength)
	      && !isalnum (tokptr[namelength]))
	    {
	      yylval.lval = regno;
	      lexptr += namelength;
	      return (GDB_REGNAME);
	    }
	}

      /* Look for a match with a standard register name, usually something
	 like "pc", which gdb always recognizes as the program counter
	 regardless of what the native register name is. */

      for (regno = 0; regno < num_std_regs; regno++)
	{
	  namelength = strlen (std_regs[regno].name);
	  if (STREQN (tokptr, std_regs[regno].name, namelength)
	      && !isalnum (tokptr[namelength]))
	    {
	      yylval.lval = std_regs[regno].regnum;
	      lexptr += namelength;
	      return (GDB_REGNAME);
	    }
	}

      /* Attempt to match against a convenience variable.  Note that
	 this will always succeed, because if no variable of that name
	 already exists, the lookup_internalvar will create one for us.
	 Also note that both lexptr and tokptr currently point to the
	 start of the input string we are trying to match, and that we
	 have already tested the first character for non-numeric, so we
	 don't have to treat it specially. */

      while (*tokptr == '_' || isalnum (*tokptr))
	{
	  tokptr++;
	}
      yylval.sval.ptr = lexptr;
      yylval.sval.length = tokptr - lexptr;
      yylval.ivar = lookup_internalvar (copy_name (yylval.sval));
      lexptr = tokptr;
      return (GDB_VARIABLE);
    }

  /* Since we didn't match against a register name or convenience
     variable, our only choice left is a history value. */

  if (*tokptr == '$')
    {
      negate = 1;
      ival = 1;
      tokptr++;
    }
  else
    {
      negate = 0;
      ival = 0;
    }

  /* Attempt to decode more characters as an integer value giving
     the index in the history list.  If successful, the value will
     overwrite ival (currently 0 or 1), and if not, ival will be
     left alone, which is good since it is currently correct for
     the '$' or '$$' case. */

  decode_integer_literal (&ival, &tokptr);
  yylval.lval = negate ? -ival : ival;
  lexptr = tokptr;
  return (GDB_LAST);
}

struct token
{
  char *operator;
  int token;
};

static const struct token idtokentab[] =
{
    { "array", ARRAY },
    { "length", LENGTH },
    { "lower", LOWER },
    { "upper", UPPER },
    { "andif", ANDIF },
    { "pred", PRED },
    { "succ", SUCC },
    { "card", CARD },
    { "size", SIZE },
    { "orif", ORIF },
    { "num", NUM },
    { "abs", ABS },
    { "max", MAX_TOKEN },
    { "min", MIN_TOKEN },
    { "mod", MOD },
    { "rem", REM },
    { "not", NOT },
    { "xor", LOGXOR },
    { "and", LOGAND },
    { "in", IN },
    { "or", LOGIOR },
    { "up", UP },
    { "addr", ADDR_TOKEN },
    { "null", EMPTINESS_LITERAL }
};

static const struct token tokentab2[] =
{
    { ":=", GDB_ASSIGNMENT },
    { "//", SLASH_SLASH },
    { "->", POINTER },
    { "/=", NOTEQUAL },
    { "<=", LEQ },
    { ">=", GTR }
};

/* Read one token, getting characters through lexptr.  */
/* This is where we will check to make sure that the language and the
   operators used are compatible.  */

static int
yylex ()
{
    unsigned int i;
    int token;
    char *inputname;
    struct symbol *sym;

    /* Skip over any leading whitespace. */
    while (isspace (*lexptr))
	{
	    lexptr++;
	}
    /* Look for special single character cases which can't be the first
       character of some other multicharacter token. */
    switch (*lexptr)
	{
	    case '\0':
	        return (0);
	    case ',':
	    case '=':
	    case ';':
	    case '!':
	    case '+':
	    case '*':
	    case '(':
	    case ')':
	    case '[':
	    case ']':
		return (*lexptr++);
	}
    /* Look for characters which start a particular kind of multicharacter
       token, such as a character literal, register name, convenience
       variable name, string literal, etc. */
    switch (*lexptr)
      {
	case '\'':
	case '\"':
	  /* First try to match a string literal, which is any
	     sequence of characters enclosed in matching single or double
	     quotes, except that a single character inside single quotes
	     is a character literal, so we have to catch that case also. */
	  token = match_string_literal ();
	  if (token != 0)
	    {
	      return (token);
	    }
	  if (*lexptr == '\'')
	    {
	      token = match_character_literal ();
	      if (token != 0)
		{
		  return (token);
		}
	    }
	  break;
        case 'C':
        case 'c':
	  token = match_character_literal ();
	  if (token != 0)
	    {
	      return (token);
	    }
	  break;
	case '$':
	  token = match_dollar_tokens ();
	  if (token != 0)
	    {
	      return (token);
	    }
	  break;
      }
    /* See if it is a special token of length 2.  */
    for (i = 0; i < sizeof (tokentab2) / sizeof (tokentab2[0]); i++)
	{
	    if (STREQN (lexptr, tokentab2[i].operator, 2))
		{
		    lexptr += 2;
		    return (tokentab2[i].token);
		}
	}
    /* Look for single character cases which which could be the first
       character of some other multicharacter token, but aren't, or we
       would already have found it. */
    switch (*lexptr)
	{
	    case '-':
	    case ':':
	    case '/':
	    case '<':
	    case '>':
		return (*lexptr++);
	}
    /* Look for a float literal before looking for an integer literal, so
       we match as much of the input stream as possible. */
    token = match_float_literal ();
    if (token != 0)
	{
	    return (token);
	}
    token = match_bitstring_literal ();
    if (token != 0)
	{
	    return (token);
	}
    token = match_integer_literal ();
    if (token != 0)
	{
	    return (token);
	}

    /* Try to match a simple name string, and if a match is found, then
       further classify what sort of name it is and return an appropriate
       token.  Note that attempting to match a simple name string consumes
       the token from lexptr, so we can't back out if we later find that
       we can't classify what sort of name it is. */

    inputname = match_simple_name_string ();

    if (inputname != NULL)
      {
	char *simplename = (char*) alloca (strlen (inputname) + 1);

	char *dptr = simplename, *sptr = inputname;
	for (; *sptr; sptr++)
	  *dptr++ = isupper (*sptr) ? tolower(*sptr) : *sptr;
	*dptr = '\0';

	/* See if it is a reserved identifier. */
	for (i = 0; i < sizeof (idtokentab) / sizeof (idtokentab[0]); i++)
	    {
		if (STREQ (simplename, idtokentab[i].operator))
		    {
			return (idtokentab[i].token);
		    }
	    }

	/* Look for other special tokens. */
	if (STREQ (simplename, "true"))
	    {
		yylval.ulval = 1;
		return (BOOLEAN_LITERAL);
	    }
	if (STREQ (simplename, "false"))
	    {
		yylval.ulval = 0;
		return (BOOLEAN_LITERAL);
	    }

	sym = lookup_symbol (inputname, expression_context_block,
			     VAR_NAMESPACE, (int *) NULL,
			     (struct symtab **) NULL);
	if (sym == NULL && strcmp (inputname, simplename) != 0)
	  {
	    sym = lookup_symbol (simplename, expression_context_block,
				 VAR_NAMESPACE, (int *) NULL,
				 (struct symtab **) NULL);
	  }
	if (sym != NULL)
	  {
	    yylval.ssym.stoken.ptr = NULL;
	    yylval.ssym.stoken.length = 0;
	    yylval.ssym.sym = sym;
	    yylval.ssym.is_a_field_of_this = 0;	/* FIXME, C++'ism */
	    switch (SYMBOL_CLASS (sym))
	      {
	      case LOC_BLOCK:
		/* Found a procedure name. */
		return (GENERAL_PROCEDURE_NAME);
	      case LOC_STATIC:
		/* Found a global or local static variable. */
		return (LOCATION_NAME);
	      case LOC_REGISTER:
	      case LOC_ARG:
	      case LOC_REF_ARG:
	      case LOC_REGPARM:
	      case LOC_REGPARM_ADDR:
	      case LOC_LOCAL:
	      case LOC_LOCAL_ARG:
	      case LOC_BASEREG:
	      case LOC_BASEREG_ARG:
		if (innermost_block == NULL
		    || contained_in (block_found, innermost_block))
		  {
		    innermost_block = block_found;
		  }
		return (LOCATION_NAME);
		break;
	      case LOC_CONST:
	      case LOC_LABEL:
		return (LOCATION_NAME);
		break;
	      case LOC_TYPEDEF:
		yylval.tsym.type = SYMBOL_TYPE (sym);
		return TYPENAME;
	      case LOC_UNDEF:
	      case LOC_CONST_BYTES:
	      case LOC_OPTIMIZED_OUT:
		error ("Symbol \"%s\" names no location.", inputname);
		break;
	      }
	  }
	else if (!have_full_symbols () && !have_partial_symbols ())
	  {
	    error ("No symbol table is loaded.  Use the \"file\" command.");
	  }
	else
	  {
	    error ("No symbol \"%s\" in current context.", inputname);
	  }
      }

    /* Catch single character tokens which are not part of some
       longer token. */

    switch (*lexptr)
      {
	case '.':			/* Not float for example. */
	  lexptr++;
	  while (isspace (*lexptr)) lexptr++;
	  inputname = match_simple_name_string ();
	  if (!inputname)
	    return '.';
	  return FIELD_NAME;
      }

    return (ILLEGAL_TOKEN);
}

static void
write_lower_upper_value (opcode, type)
     enum exp_opcode opcode;  /* Either UNOP_LOWER or UNOP_UPPER */
     struct type *type;
{
  extern LONGEST type_lower_upper ();
  struct type *result_type;
  LONGEST val = type_lower_upper (opcode, type, &result_type);
  write_exp_elt_opcode (OP_LONG);
  write_exp_elt_type (result_type);
  write_exp_elt_longcst (val);
  write_exp_elt_opcode (OP_LONG);
}

void
yyerror (msg)
     char *msg;
{
  error ("A %s in expression, near `%s'.", (msg ? msg : "error"), lexptr);
}
