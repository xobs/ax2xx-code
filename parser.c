/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     STRING = 258,
     D_ORG = 259,
     D_BYTE = 260,
     D_WORD = 261,
     D_SKIP = 262,
     D_EQU = 263,
     D_FLAG = 264,
     D_END = 265,
     ACALL = 266,
     ADD = 267,
     ADDC = 268,
     AJMP = 269,
     ANL = 270,
     CJNE = 271,
     CLR = 272,
     CPL = 273,
     DA = 274,
     DEC = 275,
     DIV = 276,
     DJNZ = 277,
     INC = 278,
     JB = 279,
     JBC = 280,
     JC = 281,
     JMP = 282,
     JNB = 283,
     JNC = 284,
     JNZ = 285,
     JZ = 286,
     LCALL = 287,
     LJMP = 288,
     MOV = 289,
     MOVC = 290,
     MOVX = 291,
     NOP = 292,
     MUL = 293,
     ORL = 294,
     POP = 295,
     PUSH = 296,
     RET = 297,
     RETI = 298,
     RL = 299,
     RLC = 300,
     RR = 301,
     RRC = 302,
     SETB = 303,
     SJMP = 304,
     SUBB = 305,
     SWAP = 306,
     XCH = 307,
     XCHD = 308,
     XRL = 309,
     AB = 310,
     A = 311,
     C = 312,
     PC = 313,
     DPTR = 314,
     BITPOS = 315,
     R0 = 316,
     R1 = 317,
     R2 = 318,
     R3 = 319,
     R4 = 320,
     R5 = 321,
     R6 = 322,
     R7 = 323,
     VALUE = 324,
     SYMBOL = 325
   };
#endif
/* Tokens.  */
#define STRING 258
#define D_ORG 259
#define D_BYTE 260
#define D_WORD 261
#define D_SKIP 262
#define D_EQU 263
#define D_FLAG 264
#define D_END 265
#define ACALL 266
#define ADD 267
#define ADDC 268
#define AJMP 269
#define ANL 270
#define CJNE 271
#define CLR 272
#define CPL 273
#define DA 274
#define DEC 275
#define DIV 276
#define DJNZ 277
#define INC 278
#define JB 279
#define JBC 280
#define JC 281
#define JMP 282
#define JNB 283
#define JNC 284
#define JNZ 285
#define JZ 286
#define LCALL 287
#define LJMP 288
#define MOV 289
#define MOVC 290
#define MOVX 291
#define NOP 292
#define MUL 293
#define ORL 294
#define POP 295
#define PUSH 296
#define RET 297
#define RETI 298
#define RL 299
#define RLC 300
#define RR 301
#define RRC 302
#define SETB 303
#define SJMP 304
#define SUBB 305
#define SWAP 306
#define XCH 307
#define XCHD 308
#define XRL 309
#define AB 310
#define A 311
#define C 312
#define PC 313
#define DPTR 314
#define BITPOS 315
#define R0 316
#define R1 317
#define R2 318
#define R3 319
#define R4 320
#define R5 321
#define R6 322
#define R7 323
#define VALUE 324
#define SYMBOL 325




/* Copy the first part of user declarations.  */
#line 21 "parser.y"


#include <stdio.h>
#include <stdlib.h>

#define NOPE
#include "as31.h"
#undef NOPE

#define YYSTYPE union ystack

static unsigned char bytebuf[1024];		/* used by dumplist() */
static int bytecount;


void yyerror(const char *s);
int makeop(struct opcode * op, struct mode *m, int add);
void inclc(int i);
char *padline(char *line);
void dumplist(char *txt, int show);
void genbyte(int b);
void genstr(const char *s);
void genword(unsigned long w);

/* ------------------------ G R A M M E R ----------------------------- */



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 274 "parser.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  155
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   599

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  89
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  35
/* YYNRULES -- Number of rules.  */
#define YYNRULES  165
/* YYNRULES -- Number of states.  */
#define YYNSTATES  316

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   325

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      81,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    88,     2,    87,     2,    75,    77,     2,
      84,    85,    73,    71,    83,    72,    82,    74,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    80,     2,
      79,     2,    78,     2,    86,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    76,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    14,    16,    19,    22,
      24,    25,    29,    33,    37,    41,    45,    51,    57,    60,
      62,    65,    67,    69,    71,    75,    79,    81,    83,    87,
      89,    91,    95,    98,   102,   106,   110,   114,   118,   122,
     126,   131,   136,   138,   140,   142,   145,   148,   151,   154,
     157,   160,   163,   166,   169,   172,   175,   178,   181,   184,
     187,   190,   193,   196,   199,   205,   211,   214,   216,   218,
     221,   224,   227,   230,   233,   236,   239,   242,   245,   248,
     251,   254,   257,   260,   263,   266,   269,   272,   275,   278,
     281,   284,   287,   290,   293,   296,   304,   312,   320,   328,
     334,   340,   346,   352,   356,   360,   365,   370,   374,   379,
     383,   388,   393,   397,   401,   405,   409,   413,   418,   422,
     426,   431,   436,   441,   447,   452,   456,   460,   465,   467,
     469,   471,   474,   476,   478,   480,   486,   493,   500,   508,
     510,   512,   515,   517,   519,   521,   523,   525,   527,   529,
     531,   533,   535,   537,   539,   541,   543,   545,   547,   549,
     551,   553,   555,   557,   559,   561
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      90,     0,    -1,    91,    -1,    91,    92,    -1,    92,    -1,
      99,    80,    93,    -1,    93,    -1,    95,    81,    -1,   103,
      81,    -1,    81,    -1,    -1,     1,    94,    81,    -1,    82,
       4,    96,    -1,    82,     5,   100,    -1,    82,     6,   101,
      -1,    82,     7,    96,    -1,    82,     8,    99,    83,   102,
      -1,    82,     9,    70,    83,    97,    -1,    82,    10,    -1,
     102,    -1,    98,    60,    -1,    70,    -1,    69,    -1,    70,
      -1,   100,    83,   119,    -1,   100,    83,     3,    -1,   119,
      -1,     3,    -1,   101,    83,   120,    -1,   120,    -1,    73,
      -1,    84,   102,    85,    -1,    72,   102,    -1,   102,    76,
     102,    -1,   102,    77,   102,    -1,   102,    73,   102,    -1,
     102,    74,   102,    -1,   102,    75,   102,    -1,   102,    72,
     102,    -1,   102,    71,   102,    -1,   102,    78,    78,   102,
      -1,   102,    79,    79,   102,    -1,    70,    -1,    69,    -1,
      37,    -1,    11,   121,    -1,    14,   121,    -1,    12,   104,
      -1,    13,   104,    -1,    50,   104,    -1,    54,   104,    -1,
      54,   105,    -1,    15,   104,    -1,    15,   105,    -1,    15,
     106,    -1,    39,   104,    -1,    39,   105,    -1,    39,   106,
      -1,    52,   104,    -1,    23,   110,    -1,    23,    59,    -1,
      20,   110,    -1,    19,    56,    -1,    21,    55,    -1,    27,
      86,    56,    71,    59,    -1,    27,    86,    59,    71,    56,
      -1,    38,    55,    -1,    42,    -1,    43,    -1,    44,    56,
      -1,    45,    56,    -1,    46,    56,    -1,    47,    56,    -1,
      51,    56,    -1,    53,   104,    -1,    17,   111,    -1,    18,
     111,    -1,    48,   111,    -1,    41,   119,    -1,    40,   119,
      -1,    33,   122,    -1,    32,   122,    -1,    26,   123,    -1,
      29,   123,    -1,    30,   123,    -1,    31,   123,    -1,    49,
     123,    -1,    16,   112,    -1,    24,   107,    -1,    28,   107,
      -1,    25,   107,    -1,    22,   108,    -1,    34,   104,    -1,
      34,   105,    -1,    34,   109,    -1,    35,    56,    83,    86,
      56,    71,    59,    -1,    35,    56,    83,    86,    59,    71,
      56,    -1,    35,    56,    83,    86,    56,    71,    58,    -1,
      35,    56,    83,    86,    58,    71,    56,    -1,    36,    56,
      83,    86,   118,    -1,    36,    56,    83,    86,    59,    -1,
      36,    86,   118,    83,    56,    -1,    36,    86,    59,    83,
      56,    -1,    56,    83,   117,    -1,    56,    83,   119,    -1,
      56,    83,    86,   118,    -1,    56,    83,    87,   119,    -1,
     119,    83,    56,    -1,   119,    83,    87,   119,    -1,    57,
      83,   115,    -1,    57,    83,    74,   115,    -1,    57,    83,
      88,   115,    -1,   115,    83,   113,    -1,   117,    83,   114,
      -1,   119,    83,   113,    -1,   117,    83,    56,    -1,   117,
      83,   119,    -1,   117,    83,    87,   119,    -1,   119,    83,
     117,    -1,   119,    83,   119,    -1,   119,    83,    86,   118,
      -1,    86,   118,    83,    56,    -1,    86,   118,    83,   119,
      -1,    86,   118,    83,    87,   119,    -1,    59,    83,    87,
     120,    -1,    57,    83,   115,    -1,   119,    83,    57,    -1,
     119,    60,    83,    57,    -1,    56,    -1,   117,    -1,   119,
      -1,    86,   118,    -1,    56,    -1,    57,    -1,   115,    -1,
      56,    83,   119,    83,   113,    -1,    56,    83,    87,   119,
      83,   113,    -1,   117,    83,    87,   119,    83,   113,    -1,
      86,   118,    83,    87,   119,    83,   113,    -1,   102,    -1,
     102,    -1,   116,    60,    -1,   116,    -1,    70,    -1,    69,
      -1,    61,    -1,    62,    -1,    63,    -1,    64,    -1,    65,
      -1,    66,    -1,    67,    -1,    68,    -1,    61,    -1,    62,
      -1,    63,    -1,    64,    -1,    65,    -1,    66,    -1,    67,
      -1,    68,    -1,   102,    -1,   102,    -1,   102,    -1,   102,
      -1,   102,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   126,   126,   131,   132,   135,   145,   148,   153,   159,
     164,   164,   183,   190,   191,   192,   195,   204,   210,   213,
     223,   236,   242,   246,   254,   259,   270,   275,   287,   292,
     306,   309,   312,   315,   318,   321,   324,   327,   330,   333,
     336,   339,   341,   354,   366,   368,   370,   372,   374,   376,
     378,   380,   382,   384,   386,   388,   390,   392,   394,   399,
     401,   403,   405,   407,   409,   411,   413,   415,   417,   419,
     421,   423,   425,   427,   429,   433,   435,   437,   441,   450,
     459,   461,   463,   465,   467,   469,   471,   473,   475,   477,
     479,   481,   483,   485,   487,   491,   493,   495,   497,   500,
     502,   504,   506,   518,   524,   531,   537,   546,   553,   563,
     570,   577,   586,   596,   603,   613,   619,   626,   633,   640,
     648,   655,   661,   668,   675,   683,   699,   706,   725,   732,
     738,   745,   753,   759,   765,   774,   782,   790,   798,   808,
     825,   838,   851,   861,   867,   870,   871,   872,   873,   874,
     875,   876,   877,   880,   881,   882,   885,   888,   891,   894,
     897,   902,   912,   922,   937,   951
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "STRING", "D_ORG", "D_BYTE", "D_WORD",
  "D_SKIP", "D_EQU", "D_FLAG", "D_END", "ACALL", "ADD", "ADDC", "AJMP",
  "ANL", "CJNE", "CLR", "CPL", "DA", "DEC", "DIV", "DJNZ", "INC", "JB",
  "JBC", "JC", "JMP", "JNB", "JNC", "JNZ", "JZ", "LCALL", "LJMP", "MOV",
  "MOVC", "MOVX", "NOP", "MUL", "ORL", "POP", "PUSH", "RET", "RETI", "RL",
  "RLC", "RR", "RRC", "SETB", "SJMP", "SUBB", "SWAP", "XCH", "XCHD", "XRL",
  "AB", "A", "C", "PC", "DPTR", "BITPOS", "R0", "R1", "R2", "R3", "R4",
  "R5", "R6", "R7", "VALUE", "SYMBOL", "'+'", "'-'", "'*'", "'/'", "'%'",
  "'|'", "'&'", "'>'", "'<'", "':'", "'\\n'", "'.'", "','", "'('", "')'",
  "'@'", "'#'", "'!'", "$accept", "program", "linelist", "line",
  "linerest", "@1", "directive", "defexpr", "flag", "flagv", "undefsym",
  "blist", "wlist", "expr", "instr", "two_op1", "two_op2", "two_op3",
  "two_op4", "two_op5", "two_op6", "single_op1", "single_op2", "three_op1",
  "rel", "rel2", "bit", "bitv", "reg", "regi", "data8", "data16", "addr11",
  "addr16", "relative", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,    43,    45,    42,    47,    37,   124,    38,    62,    60,
      58,    10,    46,    44,    40,    41,    64,    35,    33
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    89,    90,    91,    91,    92,    92,    93,    93,    93,
      94,    93,    95,    95,    95,    95,    95,    95,    95,    96,
      97,    98,    98,    99,   100,   100,   100,   100,   101,   101,
     102,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   104,   104,   104,   104,   105,   105,   106,
     106,   106,   107,   108,   108,   109,   109,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   109,   110,   110,
     110,   110,   111,   111,   111,   112,   112,   112,   112,   113,
     114,   115,   115,   116,   116,   117,   117,   117,   117,   117,
     117,   117,   117,   118,   118,   118,   118,   118,   118,   118,
     118,   119,   120,   121,   122,   123
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     3,     1,     2,     2,     1,
       0,     3,     3,     3,     3,     3,     5,     5,     2,     1,
       2,     1,     1,     1,     3,     3,     1,     1,     3,     1,
       1,     3,     2,     3,     3,     3,     3,     3,     3,     3,
       4,     4,     1,     1,     1,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     5,     5,     2,     1,     1,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     7,     7,     7,     7,     5,
       5,     5,     5,     3,     3,     4,     4,     3,     4,     3,
       4,     4,     3,     3,     3,     3,     3,     4,     3,     3,
       4,     4,     4,     5,     4,     3,     3,     4,     1,     1,
       1,     2,     1,     1,     1,     5,     6,     6,     7,     1,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    44,     0,
       0,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    23,     9,     0,     0,
       0,     4,     6,     0,     0,     0,     0,    43,    42,     0,
      30,     0,   163,    45,     0,    47,    48,    46,     0,   161,
      52,    53,    54,     0,     0,   145,   146,   147,   148,   149,
     150,   151,   152,     0,    87,     0,   132,   133,   144,   143,
      75,   134,   142,    76,    62,   128,     0,    61,   129,   130,
      63,    91,     0,     0,    60,    59,    88,     0,    90,   165,
      82,     0,    89,    83,    84,    85,   164,    81,    80,     0,
       0,     0,    92,    93,    94,     0,     0,     0,     0,     0,
      66,    55,    56,    57,    79,    78,    69,    70,    71,    72,
      77,    86,    49,    73,    58,    74,    50,    51,     0,     0,
       0,     0,     0,     0,    18,     1,     3,     7,     0,     8,
      11,    32,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   153,   154,   155,   156,
     157,   158,   159,   160,     0,     0,   141,   131,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    19,    27,    13,    26,    14,   162,
      29,    15,     0,     0,     5,    31,    39,    38,    35,    36,
      37,    33,    34,     0,     0,     0,     0,   103,   104,     0,
       0,   109,   107,     0,     0,     0,     0,     0,   140,   113,
     139,   114,   112,     0,     0,   125,     0,     0,   115,     0,
     116,     0,   126,     0,   118,   119,     0,     0,     0,     0,
       0,     0,     0,     0,    40,    41,   105,   106,   110,   111,
     108,     0,     0,     0,     0,    64,    65,   124,   121,     0,
     122,   117,   127,   120,     0,     0,     0,   100,    99,   102,
     101,    25,    24,    28,    16,    22,    21,    17,     0,     0,
     135,     0,     0,   123,     0,     0,     0,    20,   136,     0,
     137,    97,    95,    98,    96,   138
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    49,    50,    51,    52,    56,    53,   203,   297,   298,
      54,   206,   208,    69,    55,    65,    71,    72,   106,   101,
     124,    97,    90,    84,   241,   239,    91,    92,    98,   184,
      73,   210,    63,   117,   110
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -239
static const yytype_int16 yypact[] =
{
     370,  -239,   139,   -24,   -24,   139,   120,    70,    15,    15,
     -19,   470,   -12,   292,   441,    23,    23,   139,   -45,    23,
     139,   139,   139,   139,   139,   222,    -9,   -26,  -239,    14,
     120,   139,   139,  -239,  -239,    19,    25,    31,    49,    15,
     139,   -24,    54,   -24,   -24,    85,  -239,  -239,   440,    89,
     298,  -239,  -239,   -27,    37,    38,    41,  -239,  -239,   139,
    -239,   139,   513,  -239,    64,  -239,  -239,  -239,    67,   513,
    -239,  -239,  -239,    68,    77,  -239,  -239,  -239,  -239,  -239,
    -239,  -239,  -239,   454,  -239,    87,  -239,  -239,  -239,  -239,
    -239,  -239,    83,  -239,  -239,  -239,   454,  -239,  -239,  -239,
    -239,  -239,    90,    91,  -239,  -239,  -239,   101,  -239,   513,
    -239,   -30,  -239,  -239,  -239,  -239,   513,  -239,  -239,   102,
     108,   454,  -239,  -239,  -239,   111,   -50,   115,   124,   485,
    -239,  -239,  -239,  -239,  -239,  -239,  -239,  -239,  -239,  -239,
    -239,  -239,  -239,  -239,  -239,  -239,  -239,  -239,   139,     4,
     139,   139,   140,   143,  -239,  -239,  -239,  -239,   442,  -239,
    -239,   104,   354,   139,   139,   139,   139,   139,   139,   139,
      75,   145,   496,   126,   -47,   133,  -239,  -239,  -239,  -239,
    -239,  -239,  -239,  -239,   135,   132,  -239,  -239,   139,   139,
     139,   154,   155,    23,   141,   144,   -17,   147,    34,   146,
     148,   150,   152,  -239,   513,  -239,   156,  -239,   158,   513,
    -239,  -239,   159,   160,  -239,  -239,   520,   520,   104,   104,
     104,    30,    30,   139,   139,   454,   139,  -239,  -239,    23,
      23,  -239,  -239,   139,   139,   162,   151,   139,   513,  -239,
     513,  -239,  -239,   172,   190,  -239,   139,    55,  -239,   139,
    -239,   191,  -239,   454,  -239,  -239,    56,   511,   193,   195,
      10,   139,   139,    60,  -239,  -239,  -239,  -239,  -239,  -239,
    -239,   164,   139,   139,   170,  -239,  -239,  -239,  -239,   139,
    -239,  -239,  -239,  -239,   183,   184,   185,  -239,  -239,  -239,
    -239,  -239,  -239,  -239,   513,  -239,  -239,  -239,   197,   139,
    -239,   175,   139,  -239,    86,   205,   206,  -239,  -239,   139,
    -239,  -239,  -239,  -239,  -239,  -239
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -239,  -239,  -239,   214,   107,  -239,  -239,   116,  -239,  -239,
     114,  -239,  -239,    -2,  -239,    21,    33,   238,    26,  -239,
    -239,   255,    22,  -239,  -186,  -239,   -14,  -239,    -1,   -85,
       3,  -238,   266,   248,    28
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -3
static const yytype_int16 yytable[] =
{
      62,   107,   107,    62,   242,   107,    85,   205,   277,   232,
     197,   187,   102,   291,    99,   109,   103,    99,   109,   109,
     109,   116,   116,   293,   125,    66,   191,    70,   126,   192,
     128,    93,    64,   198,   134,   135,   195,    94,   109,   248,
     233,   111,   108,   100,   202,   112,   122,   127,   113,   114,
     115,   131,    57,    58,   157,    59,    60,   161,   123,   162,
     129,   140,   142,   132,   144,   145,   146,    61,   141,   130,
     249,    86,    87,    57,    58,   136,    59,    60,   147,    57,
      58,   137,    59,    60,    88,    89,   300,   138,    61,   155,
     232,   252,    88,    89,    61,    75,    76,    77,    78,    79,
      80,    81,    82,    57,    58,   139,    59,    60,   170,   171,
     143,   278,   284,   308,   285,   286,   310,   158,    61,   159,
     253,   233,   160,   315,    57,    58,    74,    59,    60,   295,
     296,    75,    76,    77,    78,    79,    80,    81,    82,    61,
     266,    64,   279,   186,   311,   312,   204,   172,   209,   204,
     173,   174,   207,   223,    57,    58,    83,    59,    60,   231,
     175,   216,   217,   218,   219,   220,   221,   222,   283,    61,
     185,   227,   288,   188,   189,   228,    64,    68,   235,   245,
     168,   169,   170,   171,   190,   193,   238,   240,   240,    57,
      58,   194,    59,    60,   196,    88,    89,   254,   199,   250,
     229,   255,    57,    58,    61,    59,    60,   200,    57,    58,
      46,    59,    60,   213,   230,   268,   269,    61,   236,   237,
     234,   264,   265,    61,   224,   243,   244,   247,   246,   267,
     251,   275,   256,   258,   257,   259,   270,   271,   273,   260,
     274,   261,   262,   263,   209,   272,   276,   299,   282,   289,
     280,   290,   281,   302,   304,   305,   306,   307,   309,   209,
     294,   313,   314,   292,   156,   214,   212,   211,   133,   105,
     240,    67,   118,     0,     0,     0,   301,     0,    64,   119,
       0,   120,   303,    75,    76,    77,    78,    79,    80,    81,
      82,    57,    58,     0,    59,    60,     0,   240,    -2,     1,
     240,     0,     0,     0,     0,     0,    61,   240,   121,     2,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    75,    76,    77,    78,    79,    80,    81,
      82,    57,    58,     0,    59,    60,     0,     0,    46,     0,
       0,     1,     0,     0,     0,     0,    61,     0,     0,    47,
      48,     2,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,   163,   164,   165,   166,   167,
     168,   169,   170,   171,     0,     0,     0,     0,     0,   215,
      46,     0,     0,     1,   148,   149,   150,   151,   152,   153,
     154,    47,    48,     2,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    95,     0,     0,
     104,     0,    75,    76,    77,    78,    79,    80,    81,    82,
      57,    58,     0,    59,    60,   176,   177,   178,   179,   180,
     181,   182,   183,    47,    48,    61,    95,    96,     0,     0,
       0,    75,    76,    77,    78,    79,    80,    81,    82,    57,
      58,     0,    59,    60,   201,     0,   176,   177,   178,   179,
     180,   181,   182,   183,    61,     0,    96,    75,    76,    77,
      78,    79,    80,    81,    82,    57,    58,     0,    59,    60,
     287,     0,   176,   177,   178,   179,   180,   181,   182,   183,
      61,     0,   225,   226,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   165,   166,   167,   168,   169,   170,   171
};

static const yytype_int16 yycheck[] =
{
       2,    15,    16,     5,   190,    19,     7,     3,   246,    56,
      60,    96,    13,     3,    11,    17,    13,    14,    20,    21,
      22,    23,    24,   261,    25,     4,    56,     6,    25,    59,
      56,     9,    56,    83,    31,    32,   121,    56,    40,    56,
      87,    86,    16,    55,   129,    19,    25,    56,    20,    21,
      22,    30,    69,    70,    81,    72,    73,    59,    25,    61,
      86,    39,    41,    30,    43,    44,    45,    84,    40,    55,
      87,    56,    57,    69,    70,    56,    72,    73,    45,    69,
      70,    56,    72,    73,    69,    70,   272,    56,    84,     0,
      56,    57,    69,    70,    84,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    56,    72,    73,    78,    79,
      56,    56,    56,   299,    58,    59,   302,    80,    84,    81,
      86,    87,    81,   309,    69,    70,    56,    72,    73,    69,
      70,    61,    62,    63,    64,    65,    66,    67,    68,    84,
     225,    56,    87,    60,    58,    59,   148,    83,   150,   151,
      83,    83,   149,    78,    69,    70,    86,    72,    73,   173,
      83,   163,   164,   165,   166,   167,   168,   169,   253,    84,
      83,   172,   257,    83,    83,   172,    56,    57,   175,   193,
      76,    77,    78,    79,    83,    83,   188,   189,   190,    69,
      70,    83,    72,    73,    83,    69,    70,   198,    83,   196,
      74,   198,    69,    70,    84,    72,    73,    83,    69,    70,
      70,    72,    73,    70,    88,   229,   230,    84,    83,    87,
      87,   223,   224,    84,    79,    71,    71,    83,    87,   226,
      83,    59,    86,    83,    86,    83,   233,   234,    87,    83,
     237,    83,    83,    83,   246,    83,    56,    83,    57,    56,
     247,    56,   249,    83,    71,    71,    71,    60,    83,   261,
     262,    56,    56,   260,    50,   158,   152,   151,    30,    14,
     272,     5,    24,    -1,    -1,    -1,   273,    -1,    56,    57,
      -1,    59,   279,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    -1,   299,     0,     1,
     302,    -1,    -1,    -1,    -1,    -1,    84,   309,    86,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    -1,    72,    73,    -1,    -1,    70,    -1,
      -1,     1,    -1,    -1,    -1,    -1,    84,    -1,    -1,    81,
      82,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,
      70,    -1,    -1,     1,     4,     5,     6,     7,     8,     9,
      10,    81,    82,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    56,    -1,    -1,
      59,    -1,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    72,    73,    61,    62,    63,    64,    65,
      66,    67,    68,    81,    82,    84,    56,    86,    -1,    -1,
      -1,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    59,    -1,    61,    62,    63,    64,
      65,    66,    67,    68,    84,    -1,    86,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    -1,    72,    73,
      59,    -1,    61,    62,    63,    64,    65,    66,    67,    68,
      84,    -1,    86,    87,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    73,    74,    75,    76,    77,    78,    79
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    70,    81,    82,    90,
      91,    92,    93,    95,    99,   103,    94,    69,    70,    72,
      73,    84,   102,   121,    56,   104,   104,   121,    57,   102,
     104,   105,   106,   119,    56,    61,    62,    63,    64,    65,
      66,    67,    68,    86,   112,   117,    56,    57,    69,    70,
     111,   115,   116,   111,    56,    56,    86,   110,   117,   119,
      55,   108,   117,   119,    59,   110,   107,   115,   107,   102,
     123,    86,   107,   123,   123,   123,   102,   122,   122,    57,
      59,    86,   104,   105,   109,   117,   119,    56,    56,    86,
      55,   104,   105,   106,   119,   119,    56,    56,    56,    56,
     111,   123,   104,    56,   104,   104,   104,   105,     4,     5,
       6,     7,     8,     9,    10,     0,    92,    81,    80,    81,
      81,   102,   102,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    83,    83,    83,    83,    61,    62,    63,    64,
      65,    66,    67,    68,   118,    83,    60,   118,    83,    83,
      83,    56,    59,    83,    83,   118,    83,    60,    83,    83,
      83,    59,   118,    96,   102,     3,   100,   119,   101,   102,
     120,    96,    99,    70,    93,    85,   102,   102,   102,   102,
     102,   102,   102,    78,    79,    86,    87,   117,   119,    74,
      88,   115,    56,    87,    87,   119,    83,    87,   102,   114,
     102,   113,   113,    71,    71,   115,    87,    83,    56,    87,
     119,    83,    57,    86,   117,   119,    86,    86,    83,    83,
      83,    83,    83,    83,   102,   102,   118,   119,   115,   115,
     119,   119,    83,    87,   119,    59,    56,   120,    56,    87,
     119,   119,    57,   118,    56,    58,    59,    59,   118,    56,
      56,     3,   119,   120,   102,    69,    70,    97,    98,    83,
     113,   119,    83,   119,    71,    71,    71,    60,   113,    83,
     113,    58,    59,    56,    56,   113
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 127 "parser.y"
    {
;}
    break;

  case 5:
#line 136 "parser.y"
    {
	if (abort_asap) {YYABORT;}
	if( pass1 ) {
		(yyvsp[(1) - (3)]).sym->type = LABEL;
		(yyvsp[(1) - (3)]).sym->value = lc;
	}
	inclc((yyvsp[(3) - (3)]).value);
	bytecount = 0;
;}
    break;

  case 6:
#line 145 "parser.y"
    { inclc((yyvsp[(1) - (1)]).value); bytecount = 0; ;}
    break;

  case 7:
#line 148 "parser.y"
    {
						(yyval).value = (yyvsp[(1) - (2)]).value;
						if( dashl && pass2 )
							dumplist((yyvsp[(2) - (2)]).str,1);
					;}
    break;

  case 8:
#line 153 "parser.y"
    {
						(yyval).value = (yyvsp[(1) - (2)]).value;
						if( dashl && pass2 )
							dumplist((yyvsp[(2) - (2)]).str,1);

					;}
    break;

  case 9:
#line 159 "parser.y"
    {
						(yyval).value = 0;
						if( dashl && pass2 )
							dumplist((yyvsp[(1) - (1)]).str,0);
					;}
    break;

  case 10:
#line 164 "parser.y"
    {
						seek_eol();
					;}
    break;

  case 11:
#line 167 "parser.y"
    {
						(yyval).value = 0;
						if( dashl && pass2 )
							dumplist((yyvsp[(1) - (3)]).str,0);
					;}
    break;

  case 12:
#line 184 "parser.y"
    {
	lc = (yyvsp[(3) - (3)]).val.v;
	if( pass2 ) emitaddr(lc);
	bytecount = 0;
	(yyval).value = 0;
;}
    break;

  case 13:
#line 190 "parser.y"
    { (yyval).value = (yyvsp[(3) - (3)]).value; ;}
    break;

  case 14:
#line 191 "parser.y"
    { (yyval).value = (yyvsp[(3) - (3)]).value; ;}
    break;

  case 15:
#line 192 "parser.y"
    { (yyval).value = (yyvsp[(3) - (3)]).val.v;
					  if( pass2 )
						emitaddr(lc+(yyval).value); ;}
    break;

  case 16:
#line 196 "parser.y"
    {
	if( (yyvsp[(5) - (5)]).val.d == 0 )
		warn("Expression is undefined in pass 1");
	(yyvsp[(3) - (5)]).sym->type = LABEL;
	(yyvsp[(3) - (5)]).sym->value = (yyvsp[(5) - (5)]).val.v;
	(yyval).value = 0;
;}
    break;

  case 17:
#line 205 "parser.y"
    {
	(yyvsp[(3) - (5)]).sym->type = LABEL;
	(yyvsp[(3) - (5)]).sym->value = (yyvsp[(5) - (5)]).value;
	(yyval).value = 0;
;}
    break;

  case 18:
#line 210 "parser.y"
    { (yyval).value = 0; ;}
    break;

  case 19:
#line 214 "parser.y"
    {
		if( (yyvsp[(1) - (1)]).val.d == 0 )
			warn("Expression is undefined in pass 1");
		if( !(isbit16((yyvsp[(1) - (1)]).val.v)) )
			warn("Value greater than 16-bits");
		(yyval).value = (yyvsp[(1) - (1)]).val.v;
;}
    break;

  case 20:
#line 224 "parser.y"
    {
	if( !isbit8((yyvsp[(1) - (2)]).value) )
		warn("Bit address exceeds 8-bits");
	if( isbmram((yyvsp[(1) - (2)]).value) )
		(yyval).value = ((yyvsp[(1) - (2)]).value-0x20)*8+ (yyvsp[(2) - (2)]).value;
	else if( isbmsfr((yyvsp[(1) - (2)]).value) )
		(yyval).value = (yyvsp[(1) - (2)]).value + (yyvsp[(2) - (2)]).value;
	else
		warn("Invalid bit addressable RAM location");
;}
    break;

  case 21:
#line 237 "parser.y"
    {
	if( (yyvsp[(1) - (1)]).sym->type == UNDEF )
		warn("Symbol %s must be defined in pass 1",(yyvsp[(1) - (1)]).sym->name);
	(yyval).value = (yyvsp[(1) - (1)]).sym->value;
;}
    break;

  case 22:
#line 242 "parser.y"
    { (yyval).value = (yyvsp[(1) - (1)]).value; ;}
    break;

  case 23:
#line 247 "parser.y"
    {
	if( (yyvsp[(1) - (1)]).sym->type != UNDEF && pass1)
		warn("Attempt to redefine symbol: %s",(yyvsp[(1) - (1)]).sym->name);
	(yyval).sym = (yyvsp[(1) - (1)]).sym;
;}
    break;

  case 24:
#line 255 "parser.y"
    {
	if( pass2 ) genbyte((yyvsp[(3) - (3)]).value);
	(yyval).value = (yyvsp[(1) - (3)]).value + 1;
;}
    break;

  case 25:
#line 260 "parser.y"
    {
	if( pass1 )
		(yyval).value = (yyvsp[(1) - (3)]).value + (yyvsp[(3) - (3)]).value;
	else {
		(yyval).value = (yyvsp[(1) - (3)]).value + strlen((yyvsp[(3) - (3)]).str);
		genstr((yyvsp[(3) - (3)]).str);
		
		free((yyvsp[(3) - (3)]).str);
	}
;}
    break;

  case 26:
#line 271 "parser.y"
    {
	if( pass2 ) genbyte((yyvsp[(1) - (1)]).value);
	(yyval).value = 1;
;}
    break;

  case 27:
#line 276 "parser.y"
    {
	if( pass1 )
		(yyval).value = (yyvsp[(1) - (1)]).value;
	else {
		(yyval).value = strlen((yyvsp[(1) - (1)]).str);
		genstr((yyvsp[(1) - (1)]).str);
		free((yyvsp[(1) - (1)]).str);
	}
;}
    break;

  case 28:
#line 288 "parser.y"
    {
	if( pass2 ) genword((yyvsp[(3) - (3)]).value);
	(yyval).value = (yyvsp[(1) - (3)]).value + 2;
;}
    break;

  case 29:
#line 293 "parser.y"
    {
	if( pass2 ) genword((yyvsp[(1) - (1)]).value);
	(yyval).value = 2;
;}
    break;

  case 30:
#line 306 "parser.y"
    { (yyval).val.v = lc;
					  (yyval).val.d = 1; ;}
    break;

  case 31:
#line 309 "parser.y"
    { (yyval).val.v = (yyvsp[(2) - (3)]).val.v;
					  (yyval).val.d = (yyvsp[(2) - (3)]).val.d; ;}
    break;

  case 32:
#line 312 "parser.y"
    { (yyval).val.v = -(yyvsp[(2) - (2)]).val.v;
					  (yyval).val.d = (yyvsp[(2) - (2)]).val.d;  ;}
    break;

  case 33:
#line 315 "parser.y"
    { (yyval).val.v = (yyvsp[(1) - (3)]).val.v | (yyvsp[(3) - (3)]).val.v;
					  (yyval).val.d = (yyvsp[(1) - (3)]).val.d && (yyvsp[(3) - (3)]).val.d; ;}
    break;

  case 34:
#line 318 "parser.y"
    { (yyval).val.v = (yyvsp[(1) - (3)]).val.v & (yyvsp[(3) - (3)]).val.v;
					  (yyval).val.d = (yyvsp[(1) - (3)]).val.d && (yyvsp[(3) - (3)]).val.d; ;}
    break;

  case 35:
#line 321 "parser.y"
    { (yyval).val.v = (yyvsp[(1) - (3)]).val.v * (yyvsp[(3) - (3)]).val.v;
					  (yyval).val.d = (yyvsp[(1) - (3)]).val.d && (yyvsp[(3) - (3)]).val.d; ;}
    break;

  case 36:
#line 324 "parser.y"
    { (yyval).val.v = (yyvsp[(1) - (3)]).val.v / (yyvsp[(3) - (3)]).val.v;
					  (yyval).val.d = (yyvsp[(1) - (3)]).val.d && (yyvsp[(3) - (3)]).val.d; ;}
    break;

  case 37:
#line 327 "parser.y"
    { (yyval).val.v = (yyvsp[(1) - (3)]).val.v % (yyvsp[(3) - (3)]).val.v;
					  (yyval).val.d = (yyvsp[(1) - (3)]).val.d && (yyvsp[(3) - (3)]).val.d; ;}
    break;

  case 38:
#line 330 "parser.y"
    { (yyval).val.v = (yyvsp[(1) - (3)]).val.v - (yyvsp[(3) - (3)]).val.v;
					  (yyval).val.d = (yyvsp[(1) - (3)]).val.d && (yyvsp[(3) - (3)]).val.d; ;}
    break;

  case 39:
#line 333 "parser.y"
    { (yyval).val.v = (yyvsp[(1) - (3)]).val.v + (yyvsp[(3) - (3)]).val.v;
					  (yyval).val.d = (yyvsp[(1) - (3)]).val.d && (yyvsp[(3) - (3)]).val.d; ;}
    break;

  case 40:
#line 336 "parser.y"
    { (yyval).val.v = (yyvsp[(1) - (4)]).val.v >> (yyvsp[(4) - (4)]).val.v;
					  (yyval).val.d = (yyvsp[(1) - (4)]).val.d && (yyvsp[(4) - (4)]).val.d; ;}
    break;

  case 41:
#line 339 "parser.y"
    { (yyval).val.v = (yyvsp[(1) - (4)]).val.v << (yyvsp[(4) - (4)]).val.v;
					  (yyval).val.d = (yyvsp[(1) - (4)]).val.d && (yyvsp[(4) - (4)]).val.d; ;}
    break;

  case 42:
#line 342 "parser.y"
    {
	if( pass1 ) {
		(yyval).val.v = (yyvsp[(1) - (1)]).sym->value;
		(yyval).val.d = ((yyvsp[(1) - (1)]).sym->type != UNDEF);
	}
	else {
		if( (yyvsp[(1) - (1)]).sym->type == UNDEF )
			warn("Undefined symbol %s",(yyvsp[(1) - (1)]).sym->name);
		(yyval).val.v = (yyvsp[(1) - (1)]).sym->value;
		(yyval).val.d = 1;
	}
;}
    break;

  case 43:
#line 354 "parser.y"
    { (yyval).val.v = (yyvsp[(1) - (1)]).val.v; (yyval).val.d=1; ;}
    break;

  case 44:
#line 367 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (1)]).op,NULL,0); ;}
    break;

  case 45:
#line 369 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 46:
#line 371 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 47:
#line 373 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 48:
#line 375 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 49:
#line 377 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 50:
#line 379 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 51:
#line 381 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,4); ;}
    break;

  case 52:
#line 383 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 53:
#line 385 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,4); ;}
    break;

  case 54:
#line 387 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,6); ;}
    break;

  case 55:
#line 389 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 56:
#line 391 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,4); ;}
    break;

  case 57:
#line 393 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,6); ;}
    break;

  case 58:
#line 395 "parser.y"
    { if( get_md((yyvsp[(2) - (2)]).mode) == 3 )
					warn("Immediate mode is illegal");
				  (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0);
				;}
    break;

  case 59:
#line 400 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 60:
#line 402 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,NULL,4); ;}
    break;

  case 61:
#line 404 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 62:
#line 406 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,NULL,0); ;}
    break;

  case 63:
#line 408 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,NULL,0); ;}
    break;

  case 64:
#line 410 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (5)]).op,NULL,0); ;}
    break;

  case 65:
#line 412 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (5)]).op,NULL,0); ;}
    break;

  case 66:
#line 414 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,NULL,0); ;}
    break;

  case 67:
#line 416 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (1)]).op,NULL,0); ;}
    break;

  case 68:
#line 418 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (1)]).op,NULL,0); ;}
    break;

  case 69:
#line 420 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,NULL,0); ;}
    break;

  case 70:
#line 422 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,NULL,0); ;}
    break;

  case 71:
#line 424 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,NULL,0); ;}
    break;

  case 72:
#line 426 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,NULL,0); ;}
    break;

  case 73:
#line 428 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,NULL,0); ;}
    break;

  case 74:
#line 430 "parser.y"
    { if( get_md((yyvsp[(2) - (2)]).mode) != 2 )
					warn("Invalid addressing mode");
				  (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,-2); ;}
    break;

  case 75:
#line 434 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 76:
#line 436 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 77:
#line 438 "parser.y"
    { if( get_md((yyvsp[(2) - (2)]).mode) == 0 )
					warn("Invalid addressing mode");
				  (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,-1); ;}
    break;

  case 78:
#line 442 "parser.y"
    {
				   struct mode tmp;
					set_md(tmp,0);
					set_ov(tmp,0);
					set_sz(tmp,1);
					set_b1(tmp,(yyvsp[(2) - (2)]).value);
					(yyval).value = makeop((yyvsp[(1) - (2)]).op,&tmp,0);
				;}
    break;

  case 79:
#line 451 "parser.y"
    {
				   struct mode tmp;
					set_md(tmp,0);
					set_ov(tmp,0);
					set_sz(tmp,1);
					set_b1(tmp,(yyvsp[(2) - (2)]).value);
					(yyval).value = makeop((yyvsp[(1) - (2)]).op,&tmp,0);
				;}
    break;

  case 80:
#line 460 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 81:
#line 462 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 82:
#line 464 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 83:
#line 466 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 84:
#line 468 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 85:
#line 470 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 86:
#line 472 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 87:
#line 474 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 88:
#line 476 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 89:
#line 478 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 90:
#line 480 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 91:
#line 482 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 92:
#line 484 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,0); ;}
    break;

  case 93:
#line 486 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,4); ;}
    break;

  case 94:
#line 488 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (2)]).op,&(yyvsp[(2) - (2)]).mode,6); ;}
    break;

  case 95:
#line 492 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (7)]).op,NULL,0); ;}
    break;

  case 96:
#line 494 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (7)]).op,NULL,0); ;}
    break;

  case 97:
#line 496 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (7)]).op,NULL,1); ;}
    break;

  case 98:
#line 498 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (7)]).op,NULL,1); ;}
    break;

  case 99:
#line 501 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (5)]).op,NULL,(yyvsp[(5) - (5)]).value); ;}
    break;

  case 100:
#line 503 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (5)]).op,NULL,2); ;}
    break;

  case 101:
#line 505 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (5)]).op,NULL,(yyvsp[(3) - (5)]).value+3); ;}
    break;

  case 102:
#line 507 "parser.y"
    { (yyval).value = makeop((yyvsp[(1) - (5)]).op,NULL,5); ;}
    break;

  case 103:
#line 519 "parser.y"
    {
					set_md((yyval).mode,0);
					set_ov((yyval).mode, (yyvsp[(3) - (3)]).value);
					set_sz((yyval).mode, 0);
				;}
    break;

  case 104:
#line 525 "parser.y"
    {
					set_md((yyval).mode,1);
					set_ov((yyval).mode,0);
					set_sz((yyval).mode,1);
					set_b1((yyval).mode,(yyvsp[(3) - (3)]).value);
				;}
    break;

  case 105:
#line 532 "parser.y"
    {
					set_md((yyval).mode,2);
					set_ov((yyval).mode,(yyvsp[(4) - (4)]).value);
					set_sz((yyval).mode,0);
				;}
    break;

  case 106:
#line 538 "parser.y"
    {
					set_md((yyval).mode,3);
					set_ov((yyval).mode,0);
					set_sz((yyval).mode,1);
					set_b1((yyval).mode,(yyvsp[(4) - (4)]).value);
				;}
    break;

  case 107:
#line 547 "parser.y"
    {
					set_md((yyval).mode,0);
					set_ov((yyval).mode,0);
					set_sz((yyval).mode,1);
					set_b1((yyval).mode,(yyvsp[(1) - (3)]).value);
				;}
    break;

  case 108:
#line 554 "parser.y"
    {
					set_md((yyval).mode,1);
					set_ov((yyval).mode,0);
					set_sz((yyval).mode,2);
					set_b1((yyval).mode,(yyvsp[(1) - (4)]).value);
					set_b2((yyval).mode,(yyvsp[(4) - (4)]).value);
				;}
    break;

  case 109:
#line 564 "parser.y"
    {
					set_md((yyval).mode,0);
					set_ov((yyval).mode,0);
					set_sz((yyval).mode,1);
					set_b1((yyval).mode,(yyvsp[(3) - (3)]).value);
				;}
    break;

  case 110:
#line 571 "parser.y"
    {
					set_md((yyval).mode,1);
					set_ov((yyval).mode,0);
					set_sz((yyval).mode,1);
					set_b1((yyval).mode,(yyvsp[(4) - (4)]).value);
				;}
    break;

  case 111:
#line 578 "parser.y"
    {
					set_md((yyval).mode,1);
					set_ov((yyval).mode,0);
					set_sz((yyval).mode,1);
					set_b1((yyval).mode,(yyvsp[(4) - (4)]).value);
				;}
    break;

  case 112:
#line 587 "parser.y"
    {
					set_md((yyval).mode,0);
					set_ov((yyval).mode,0);
					set_sz((yyval).mode,2);
					set_b1((yyval).mode,(yyvsp[(1) - (3)]).value);
					set_b2((yyval).mode,(yyvsp[(3) - (3)]).value);
				;}
    break;

  case 113:
#line 597 "parser.y"
    {
					set_md((yyval).mode,0);
					set_ov((yyval).mode,(yyvsp[(1) - (3)]).value);
					set_sz((yyval).mode,1);
					set_b1((yyval).mode,(yyvsp[(3) - (3)]).value);
				;}
    break;

  case 114:
#line 604 "parser.y"
    {
					set_md((yyval).mode,1);
					set_ov((yyval).mode,0);
					set_sz((yyval).mode,2);
					set_b1((yyval).mode,(yyvsp[(1) - (3)]).value);
					set_b2((yyval).mode,(yyvsp[(3) - (3)]).value);
				;}
    break;

  case 115:
#line 614 "parser.y"
    {
					set_md((yyval).mode,0);
					set_ov((yyval).mode,(yyvsp[(1) - (3)]).value);
					set_sz((yyval).mode,0);
				;}
    break;

  case 116:
#line 620 "parser.y"
    {
					set_md((yyval).mode,1);
					set_ov((yyval).mode,(yyvsp[(1) - (3)]).value);
					set_sz((yyval).mode,1);
					set_b1((yyval).mode,(yyvsp[(3) - (3)]).value);
				;}
    break;

  case 117:
#line 627 "parser.y"
    {
					set_md((yyval).mode,2);
					set_ov((yyval).mode,(yyvsp[(1) - (4)]).value);
					set_sz((yyval).mode,1);
					set_b1((yyval).mode,(yyvsp[(4) - (4)]).value);
				;}
    break;

  case 118:
#line 634 "parser.y"
    {
					set_md((yyval).mode,3);
					set_ov((yyval).mode,(yyvsp[(3) - (3)]).value);
					set_sz((yyval).mode,1);
					set_b1((yyval).mode,(yyvsp[(1) - (3)]).value);
				;}
    break;

  case 119:
#line 641 "parser.y"
    {
					set_md((yyval).mode,4);
					set_ov((yyval).mode,0);
					set_sz((yyval).mode,2);
					set_b1((yyval).mode,(yyvsp[(3) - (3)]).value);
					set_b2((yyval).mode,(yyvsp[(1) - (3)]).value);
				;}
    break;

  case 120:
#line 649 "parser.y"
    {
					set_md((yyval).mode,5);
					set_ov((yyval).mode,(yyvsp[(4) - (4)]).value);
					set_sz((yyval).mode,1);
					set_b1((yyval).mode,(yyvsp[(1) - (4)]).value);
				;}
    break;

  case 121:
#line 656 "parser.y"
    {
					set_md((yyval).mode,6);
					set_ov((yyval).mode,(yyvsp[(2) - (4)]).value);
					set_sz((yyval).mode,0);
				;}
    break;

  case 122:
#line 662 "parser.y"
    {
					set_md((yyval).mode,7);
					set_ov((yyval).mode,(yyvsp[(2) - (4)]).value);
					set_sz((yyval).mode,1);
					set_b1((yyval).mode,(yyvsp[(4) - (4)]).value);
				;}
    break;

  case 123:
#line 669 "parser.y"
    {
					set_md((yyval).mode,8);
					set_ov((yyval).mode,(yyvsp[(2) - (5)]).value);
					set_sz((yyval).mode,1);
					set_b1((yyval).mode,(yyvsp[(5) - (5)]).value);
				;}
    break;

  case 124:
#line 676 "parser.y"
    {
				set_md((yyval).mode,9);
				set_ov((yyval).mode,0);
				set_sz((yyval).mode,2);
				set_b1((yyval).mode, ((yyvsp[(4) - (4)]).value & 0xff00) >> 8 );
				set_b2((yyval).mode, ((yyvsp[(4) - (4)]).value & 0x00ff) );
			;}
    break;

  case 125:
#line 684 "parser.y"
    {
					set_md((yyval).mode,10);
					set_ov((yyval).mode,0);
					set_sz((yyval).mode,1);
					set_b1((yyval).mode,(yyvsp[(3) - (3)]).value);
				;}
    break;

  case 126:
#line 700 "parser.y"
    {
					set_md((yyval).mode,11);
					set_ov((yyval).mode,0);
					set_sz((yyval).mode,1);
					set_b1((yyval).mode,(yyvsp[(1) - (3)]).value);
				;}
    break;

  case 127:
#line 707 "parser.y"
    {
	if( pass2 ) {
		if( !isbit8((yyvsp[(1) - (4)]).value) )
			warn("Bit address exceeds 8-bits");
		if( isbmram((yyvsp[(1) - (4)]).value) )
			set_b1((yyval).mode, ((yyvsp[(1) - (4)]).value-0x20)*8+ (yyvsp[(2) - (4)]).value );
		else if( isbmsfr((yyvsp[(1) - (4)]).value) )
			set_b1((yyval).mode, (yyvsp[(1) - (4)]).value + (yyvsp[(2) - (4)]).value );
		else
			warn("Invalid bit addressable RAM location");
	}
	set_md((yyval).mode,11);
	set_ov((yyval).mode,0);
	set_sz((yyval).mode,1);
;}
    break;

  case 128:
#line 726 "parser.y"
    {
					set_md((yyval).mode,0);
					set_ov((yyval).mode,0);
					set_sz((yyval).mode,0);
				;}
    break;

  case 129:
#line 733 "parser.y"
    {
					set_md((yyval).mode,1);
					set_ov((yyval).mode,(yyvsp[(1) - (1)]).value);
					set_sz((yyval).mode,0);
				;}
    break;

  case 130:
#line 739 "parser.y"
    {
					set_md((yyval).mode,2);
					set_ov((yyval).mode,0);
					set_sz((yyval).mode,1);
					set_b1((yyval).mode,(yyvsp[(1) - (1)]).value);
				;}
    break;

  case 131:
#line 746 "parser.y"
    {
					set_md((yyval).mode,3);
					set_ov((yyval).mode,(yyvsp[(2) - (2)]).value);
					set_sz((yyval).mode,0);
				;}
    break;

  case 132:
#line 754 "parser.y"
    {
					set_md((yyval).mode,0);
					set_ov((yyval).mode,0);
					set_sz((yyval).mode,0);
				;}
    break;

  case 133:
#line 760 "parser.y"
    {
					set_md((yyval).mode,1);
					set_ov((yyval).mode,0);
					set_sz((yyval).mode,0);
				;}
    break;

  case 134:
#line 766 "parser.y"
    {
					set_md((yyval).mode,2);
					set_ov((yyval).mode,0);
					set_sz((yyval).mode,1);
					set_b1((yyval).mode,(yyvsp[(1) - (1)]).value);
				;}
    break;

  case 135:
#line 775 "parser.y"
    {
					set_md((yyval).mode,0);
					set_ov((yyval).mode,0);
					set_sz((yyval).mode,2);
					set_b1((yyval).mode,(yyvsp[(3) - (5)]).value);
					set_b2((yyval).mode,(yyvsp[(5) - (5)]).value);
				;}
    break;

  case 136:
#line 783 "parser.y"
    {
					set_md((yyval).mode,1);
					set_ov((yyval).mode,0);
					set_sz((yyval).mode,2);
					set_b1((yyval).mode,(yyvsp[(4) - (6)]).value);
					set_b2((yyval).mode,(yyvsp[(6) - (6)]).value);
				;}
    break;

  case 137:
#line 791 "parser.y"
    {
					set_md((yyval).mode,2);
					set_ov((yyval).mode,(yyvsp[(1) - (6)]).value);
					set_sz((yyval).mode,2);
					set_b1((yyval).mode,(yyvsp[(4) - (6)]).value);
					set_b2((yyval).mode,(yyvsp[(6) - (6)]).value);
				;}
    break;

  case 138:
#line 799 "parser.y"
    {
					set_md((yyval).mode,3);
					set_ov((yyval).mode,(yyvsp[(2) - (7)]).value);
					set_sz((yyval).mode,2);
					set_b1((yyval).mode,(yyvsp[(5) - (7)]).value);
					set_b2((yyval).mode,(yyvsp[(7) - (7)]).value);
				;}
    break;

  case 139:
#line 809 "parser.y"
    {
		long offset;
		if( pass2 ) {
			offset = (yyvsp[(1) - (1)]).val.v - (lc+3);
			if( offset > 127 || offset < -128 )
			   warn("Relative offset exceeds -128 / +127");
			(yyval).value = offset;
		}
;}
    break;

  case 140:
#line 826 "parser.y"
    {
		long offset;
		if( pass2 ) {
			offset = (yyvsp[(1) - (1)]).val.v - (lc+2); /* different! */
			if( offset > 127 || offset < -128 )
			   warn("Relative offset exceeds -128 / +127");
			(yyval).value = offset;
		}
;}
    break;

  case 141:
#line 839 "parser.y"
    {
	if( pass2 ) {
		if( !isbit8((yyvsp[(1) - (2)]).value) )
			warn("Bit address exceeds 8-bits");
		if( isbmram((yyvsp[(1) - (2)]).value) )
			(yyval).value = ((yyvsp[(1) - (2)]).value-0x20)*8+(yyvsp[(2) - (2)]).value;
		else if( isbmsfr((yyvsp[(1) - (2)]).value) )
			(yyval).value = (yyvsp[(1) - (2)]).value + (yyvsp[(2) - (2)]).value;
		else
			warn("Invalid bit addressable RAM location");
	}
;}
    break;

  case 142:
#line 852 "parser.y"
    {
	if( pass2 ) {
		if( !isbit8((yyvsp[(1) - (1)]).value) )
			warn("Bit address exceeds 8-bits");
		(yyval).value = (yyvsp[(1) - (1)]).value;
	}
;}
    break;

  case 143:
#line 862 "parser.y"
    {
	if( (yyvsp[(1) - (1)]).sym->type == UNDEF && pass2 )
		warn("Symbol %s undefined",(yyvsp[(1) - (1)]).sym->name);
	(yyval).value = (yyvsp[(1) - (1)]).sym->value;
;}
    break;

  case 144:
#line 867 "parser.y"
    { (yyval).value = (yyvsp[(1) - (1)]).value; ;}
    break;

  case 145:
#line 870 "parser.y"
    { (yyval).value = 0; ;}
    break;

  case 146:
#line 871 "parser.y"
    { (yyval).value = 1; ;}
    break;

  case 147:
#line 872 "parser.y"
    { (yyval).value = 2; ;}
    break;

  case 148:
#line 873 "parser.y"
    { (yyval).value = 3; ;}
    break;

  case 149:
#line 874 "parser.y"
    { (yyval).value = 4; ;}
    break;

  case 150:
#line 875 "parser.y"
    { (yyval).value = 5; ;}
    break;

  case 151:
#line 876 "parser.y"
    { (yyval).value = 6; ;}
    break;

  case 152:
#line 877 "parser.y"
    { (yyval).value = 7; ;}
    break;

  case 153:
#line 880 "parser.y"
    { (yyval).value = 0; ;}
    break;

  case 154:
#line 881 "parser.y"
    { (yyval).value = 1; ;}
    break;

  case 155:
#line 883 "parser.y"
    { (yyval).value = 0;
				  warn("Illegal indirect register: @r2"); ;}
    break;

  case 156:
#line 886 "parser.y"
    { (yyval).value = 0;
				  warn("Illegal indirect register: @r3"); ;}
    break;

  case 157:
#line 889 "parser.y"
    { (yyval).value = 0;
				  warn("Illegal indirect register: @r4"); ;}
    break;

  case 158:
#line 892 "parser.y"
    { (yyval).value = 0;
				  warn("Illegal indirect register: @r5"); ;}
    break;

  case 159:
#line 895 "parser.y"
    { (yyval).value = 0;
				  warn("Illegal indirect register: @r6"); ;}
    break;

  case 160:
#line 898 "parser.y"
    { (yyval).value = 0;
				  warn("Illegal indirect register: @r7"); ;}
    break;

  case 161:
#line 903 "parser.y"
    {
	if( pass2 ) {
		if( !isbit8((yyvsp[(1) - (1)]).val.v) )
			warn("Expression greater than 8-bits");
	}
	(yyval).value = (yyvsp[(1) - (1)]).val.v;
;}
    break;

  case 162:
#line 913 "parser.y"
    {
	if( pass2 ) {
		if( !isbit16((yyvsp[(1) - (1)]).val.v) )
			warn("Expression greater than 16-bits");
	}
	(yyval).value = (yyvsp[(1) - (1)]).val.v;
;}
    break;

  case 163:
#line 923 "parser.y"
    {
		if( pass2 ) {
			if( !isbit16((yyvsp[(1) - (1)]).val.v)  )
				warn("Address greater than 16-bits");
			if( ((yyvsp[(1) - (1)]).val.v & size11) != ((lc+2) & size11) )
				warn("Address outside current 2K page");
		}
		set_md((yyval).mode,0);
		set_ov((yyval).mode, ((yyvsp[(1) - (1)]).val.v&0x0700)>>3 );
		set_sz((yyval).mode,1);
		set_b1((yyval).mode,(yyvsp[(1) - (1)]).val.v&0x00ff);
;}
    break;

  case 164:
#line 938 "parser.y"
    {
		if( pass2 ) {
			if( !isbit16((yyvsp[(1) - (1)]).val.v)  )
				warn("Address greater than 16-bits");
		}
		set_md((yyval).mode,0);
		set_ov((yyval).mode, 0 );
		set_sz((yyval).mode,2);
		set_b1((yyval).mode, ((yyvsp[(1) - (1)]).val.v & 0xff00 ) >> 8 );
		set_b2((yyval).mode, ((yyvsp[(1) - (1)]).val.v & 0x00ff ) );
;}
    break;

  case 165:
#line 952 "parser.y"
    {
		long offset=0;
		if( pass2 ) {
			offset = (yyvsp[(1) - (1)]).val.v - (lc+2);
			if( offset>127 || offset<-128 )
			   warn("Relative offset exceeds -128 / +127");
		}
		set_md((yyval).mode,0);
		set_ov((yyval).mode,0);
		set_sz((yyval).mode,1);
		set_b1((yyval).mode,offset);

;}
    break;


/* Line 1267 of yacc.c.  */
#line 3046 "parser.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 967 "parser.y"


/* ---------------------------------------------------------------------- */

void yyerror(const char *s)
{
	warn("%s near \"%s\"", s, get_last_token());
}


/* ----------------------------------------------------------------------
 * makeop:
 *	This function makes an opcode based on the instruction symbol table
 *	entry, and an addressing mode structure.
 *	This function is called from both passes, but
 *	only generates code in pass 2.
 *
 *	Resultant opcode bytes are passed to genbyte().
 *
 *	Returns the nuumber of bytes that the instruction
 *	occupies.
 *
 */

int makeop(struct opcode * op, struct mode *m, int add)
{
	register unsigned int newop;

	if( m == NULL ) {
		if(pass2) genbyte(op->bytes[0+add]);
		return(1);
	}

	if( pass2 ) {
		newop = op->bytes[ get_md(*m)+add ] | get_ov(*m);
		genbyte(newop);
		if( get_sz(*m) > 0 ) genbyte( get_b1(*m) );
		if( get_sz(*m) > 1 ) genbyte( get_b2(*m) );
	}
	return( get_sz(*m)+1 );
}


/* ----------------------------------------------------------------------
 * inclc:
 *	Increments the Location Counter by 'i' amount.
 *	Check to see if 'i' overflows 64K.
 *	Checks to see if assembler is overlapping previous sections
 *	of code. (using a large bit field).
 *
 */

#define indx(a) ( (a)/(sizeof(long)*8) )
#define bit(a)	( 1 << ((a)%(sizeof(long)*8)) )

#define getloc(a) (regions[indx(a)] & bit(a))
#define setloc(a) (regions[indx(a)] |= bit(a))

static unsigned long regions[ 0x10000/(sizeof(long)*8) ];

void inclc(int i)
{

	while (i-- > 0) {
		if( pass2 && getloc(lc) )
			error("Location counter overlaps");
		if( pass2 ) setloc(lc);
		lc += 1;
	}
		
	if( lc > 0xffff )
		error("Location counter has exceeded 16-bits");
}

void clear_location_counter(void)
{
	memset(regions, 0, sizeof(regions));
}


/* ----------------------------------------------------------------------
 * padline:
 *	This routine returns a new string, which is equivilant to
 *	'line' except that all tabs have been expanded to spaces, and
 *	the total length has been truncated to 60 chars.
 */

char *padline(char *line)
{
	static char newline[61];
	char *p1;
	int pos=0,nxtpos;

	for(p1=line; pos<sizeof(newline)-1 && *p1; p1++ ) {
		if( *p1 == '\t' ) {
			nxtpos = pos+8-pos%8;
			while(pos<sizeof(newline)-1 && pos <= nxtpos)
				newline[pos++] = ' ';
		} else if( *p1 != '\n' )
			newline[pos++]= *p1;
	}
	newline[pos] = '\0';
	return(newline);
}


/* ----------------------------------------------------------------------
 * dumplist:
 *	Outputs the current location counter, bytebuf[] array, and
 *	the string 'txt' to the listing file.
 *	This routine is called for every source line encountered in the
 *	source file. (Only in pass 2, and if listing is turned on).
 *
 */

void dumplist(char *txt, int show)
{
	int i,j;

	fprintf(listing,show?"%04X: ":"      ",lc);

	j=0;
	for(i=0; i<bytecount; i++ ) {
		fprintf(listing,"%02X ",bytebuf[i]);
		if( ++j >= 4 ) {
			j = 0;
			fprintf(listing,"\n      ");
		}
	}
	while(++j <= 4)
		fprintf(listing,"   ");

	fprintf(listing," %s\n",padline(txt));
}

/* ----------------------------------------------------------------------
 * gen* routines:
 *	Place information into the bytebuf[] array, and also
 *	call emitbyte with the byte.
 *
 */

void genbyte(int b)
{
	if( bytecount < sizeof(bytebuf) )
		bytebuf[bytecount++] = b;
	emitbyte(b);
}

void genstr(const char *s)
{
	while( *s )
		genbyte(*s++);
}

void genword(unsigned long w)
{
	genbyte( (w & 0xff00) >> 8 );
	genbyte( (w & 0x00ff) );
}


