/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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
#define YYBISON_VERSION "2.7"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
/* Line 371 of yacc.c  */
#line 1 "parser.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  // ADDED for isdigit()

FILE *out;
void yyerror(const char *s);
int yylex(void);

/* Variable table */
#define MAX_VARS 256
static char *var_name[MAX_VARS];
static char *var_value[MAX_VARS];
static int var_count = 0;

/* Config storage */
static char *conf_base = NULL;
#define MAX_CONF_HDRS 64
static char *conf_hdr_k[MAX_CONF_HDRS];
static char *conf_hdr_v[MAX_CONF_HDRS];
static int conf_hdr_count = 0;

extern int yylineno;

// ADDED: Error tracking
static int error_occurred = 0;

// ADDED: Semantic error checking functions (return 1 for error, 0 for OK)
static int check_identifier(const char *ident) {
    if (ident == NULL || ident[0] == '\0') return 0;
    
    // Check if identifier starts with digit
    if (isdigit(ident[0])) {
        fprintf(stderr, "Line %d: expected IDENT after 'let'\n", yylineno);
        error_occurred = 1;
        return 1;
    }
    return 0;
}

static int check_body_type(int is_string) {
    if (!is_string) {
        fprintf(stderr, "Line %d: expected STRING after 'body ='\n", yylineno);
        error_occurred = 1;
        return 1;
    }
    return 0;
}

static int check_status_type(int is_number) {
    if (!is_number) {
        fprintf(stderr, "Line %d: expected NUMBER for status\n", yylineno);
        error_occurred = 1;
        return 1;
    }
    return 0;
}

static void add_var(const char *n, const char *v) {
    if (var_count >= MAX_VARS) return;
    var_name[var_count] = strdup(n);
    var_value[var_count] = strdup(v);
    var_count++;
}
static const char *lookup_var(const char *n) {
    for (int i=0;i<var_count;i++) if (strcmp(var_name[i], n)==0) return var_value[i];
    return "";
}

static char *unquote(const char *s) {
    size_t L = strlen(s);
    if (L>=2 && s[0]=='"' && s[L-1]=='"') {
        char *r = malloc(L-1);
        memcpy(r, s+1, L-2);
        r[L-2]=0;
        return r;
    }
    return strdup(s);
}

/* substitute $name inside inner (unquoted) string, return newly allocated string (unquoted) */
static char *subst_inner(const char *inner) {
    size_t cap = strlen(inner) + 128;
    char *out = malloc(cap);
    out[0]=0;
    const char *p = inner;
    while (*p) {
        if (*p == '$') {
            p++;
            char id[128]; int ii=0;
            while ((*p>='A' && *p<='Z') || (*p>='a' && *p<='z') || (*p>='0' && *p<='9') || *p=='_') {
                if (ii < (int)sizeof(id)-1) id[ii++]=*p;
                p++;
            }
            id[ii]=0;
            const char *val = lookup_var(id);
            size_t need = strlen(out) + strlen(val) + 1;
            if (need >= cap) { cap = need + 128; out = realloc(out, cap); }
            strcat(out, val);
        } else {
            size_t l = strlen(out);
            if (l+2 >= cap) { cap = cap + 128; out = realloc(out, cap); }
            out[l]=*p; out[l+1]=0;
            p++;
        }
    }
    return out;
}

/* take a quoted STRING token, substitute vars, and return new quoted string (including quotes) */
static char *subst_quoted(const char *quoted) {
    char *inner = unquote(quoted);
    char *s = subst_inner(inner);
    free(inner);
    size_t total = strlen(s) + 3;
    char *res = malloc(total);
    snprintf(res, total, "\"%s\"", s);
    free(s);
    return res;
}

/* generate header of file */
static void emit_header(void) {
    fprintf(out,
        "import org.junit.jupiter.api.*;\n"
        "import static org.junit.jupiter.api.Assertions.*;\n"
        "import java.net.http.*; import java.net.*; import java.time.Duration;\n"
        "import java.nio.charset.StandardCharsets; import java.util.*;\n\n"
        "public class GeneratedTests {\n"
        "  static String BASE = \"http://localhost:8080\";\n"  
        "  static Map<String,String> DEFAULT_HEADERS = new HashMap<>();\n"  
        "  static HttpClient client;\n\n"  
    );
}

/* Emit the single @BeforeAll setup block (writes DEFAULT_HEADERS and optionally BASE) */
static void emit_setup(void) {
    fprintf(out, "  @BeforeAll\n");
    fprintf(out, "  static void setup() {\n");
    fprintf(out, "    client = HttpClient.newBuilder().connectTimeout(Duration.ofSeconds(5)).build();\n");
    if (conf_hdr_count==0) {
        fprintf(out, "    DEFAULT_HEADERS.put(\"Content-Type\",\"application/json\");\n");
    } else {
        for (int i=0;i<conf_hdr_count;i++) {
            fprintf(out, "    DEFAULT_HEADERS.put(\"%s\",\"%s\");\n", conf_hdr_k[i], conf_hdr_v[i]);
        }
    }
    if (conf_base) {
        fprintf(out, "    BASE = %s;\n", conf_base);
    }
    fprintf(out, "  }\n\n");
}



/* Line 371 of yacc.c  */
#line 225 "parser.tab.c"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "parser.tab.h".  */
#ifndef YY_YY_PARSER_TAB_H_INCLUDED
# define YY_YY_PARSER_TAB_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     CONFIG = 258,
     BASE_URL = 259,
     HEADER = 260,
     LET = 261,
     TEST = 262,
     GET = 263,
     POST = 264,
     PUT = 265,
     DELETE = 266,
     EXPECT = 267,
     STATUS = 268,
     BODY = 269,
     CONTAINS = 270,
     IDENT = 271,
     STRING = 272,
     NUMBER = 273
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 387 of yacc.c  */
#line 158 "parser.y"

    char *str;
    int num;


/* Line 387 of yacc.c  */
#line 292 "parser.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

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

#endif /* !YY_YY_PARSER_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

/* Line 390 of yacc.c  */
#line 320 "parser.tab.c"

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
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(N) (N)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
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
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   93

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  23
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  17
/* YYNRULES -- Number of rules.  */
#define YYNRULES  43
/* YYNRULES -- Number of states.  */
#define YYNSTATES  100

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   273

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    22,
       2,    21,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    19,     2,    20,     2,     2,     2,     2,
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
      15,    16,    17,    18
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     7,     8,     9,    10,    13,    15,    17,
      19,    24,    25,    28,    33,    39,    45,    51,    58,    65,
      71,    77,    83,    84,    87,    89,    91,    95,    98,   102,
     105,   112,   118,   125,   131,   132,   135,   141,   146,   151,
     157,   163,   169,   176
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      24,     0,    -1,    25,    27,    26,    -1,    -1,    -1,    -1,
      27,    28,    -1,    29,    -1,    32,    -1,    33,    -1,     3,
      19,    30,    20,    -1,    -1,    30,    31,    -1,     4,    21,
      17,    22,    -1,     5,    17,    21,    17,    22,    -1,     6,
      16,    21,    17,    22,    -1,     6,    16,    21,    18,    22,
      -1,     6,    18,    16,    21,    17,    22,    -1,     6,    18,
      16,    21,    18,    22,    -1,     6,    18,    21,    17,    22,
      -1,     6,    18,    21,    18,    22,    -1,     7,    16,    19,
      34,    20,    -1,    -1,    34,    35,    -1,    36,    -1,    39,
      -1,     8,    17,    22,    -1,     8,    17,    -1,    11,    17,
      22,    -1,    11,    17,    -1,     9,    17,    19,    37,    20,
      22,    -1,     9,    17,    19,    37,    20,    -1,    10,    17,
      19,    37,    20,    22,    -1,    10,    17,    19,    37,    20,
      -1,    -1,    37,    38,    -1,     5,    17,    21,    17,    22,
      -1,    14,    21,    17,    22,    -1,    14,    21,    18,    22,
      -1,    12,    13,    21,    18,    22,    -1,    12,    13,    21,
      17,    22,    -1,    12,    14,    15,    17,    22,    -1,    12,
       5,    17,    21,    17,    22,    -1,    12,     5,    17,    15,
      17,    22,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   173,   173,   177,   181,   184,   186,   190,   191,   192,
     196,   200,   201,   210,   217,   232,   238,   244,   249,   254,
     259,   267,   278,   279,   288,   289,   293,   307,   312,   326,
     331,   366,   371,   410,   418,   419,   428,   436,   445,   453,
     459,   463,   470,   481
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "CONFIG", "BASE_URL", "HEADER", "LET",
  "TEST", "GET", "POST", "PUT", "DELETE", "EXPECT", "STATUS", "BODY",
  "CONTAINS", "IDENT", "STRING", "NUMBER", "'{'", "'}'", "'='", "';'",
  "$accept", "program", "prologue", "epilogue", "sections", "section",
  "config_block", "config_items", "config_item", "var_decl", "test_block",
  "statements", "statement", "request_stmt", "request_items",
  "request_item", "expect_stmt", YY_NULL
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   123,
     125,    61,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    23,    24,    25,    26,    27,    27,    28,    28,    28,
      29,    30,    30,    31,    31,    32,    32,    32,    32,    32,
      32,    33,    34,    34,    35,    35,    36,    36,    36,    36,
      36,    36,    36,    36,    37,    37,    38,    38,    38,    39,
      39,    39,    39,    39
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     0,     0,     0,     2,     1,     1,     1,
       4,     0,     2,     4,     5,     5,     5,     6,     6,     5,
       5,     5,     0,     2,     1,     1,     3,     2,     3,     2,
       6,     5,     6,     5,     0,     2,     5,     4,     4,     5,
       5,     5,     6,     6
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     5,     1,     4,     0,     0,     0,     2,     6,
       7,     8,     9,    11,     0,     0,     0,     0,     0,     0,
       0,    22,     0,     0,    10,    12,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,    19,    20,
       0,     0,     0,     0,     0,    21,    23,    24,    25,     0,
       0,    17,    18,    27,     0,     0,    29,     0,     0,     0,
      13,     0,    26,    34,    34,    28,     0,     0,     0,    14,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    31,
      35,    33,     0,     0,    40,    39,    41,     0,     0,    30,
      32,    43,    42,     0,     0,     0,     0,    37,    38,    36
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,     2,     8,     4,     9,    10,    17,    25,    11,
      12,    31,    46,    47,    70,    80,    48
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -37
static const yytype_int8 yypact[] =
{
     -37,     7,   -37,   -37,    27,     6,   -10,    -7,   -37,   -37,
     -37,   -37,   -37,   -37,    -6,     8,    13,    -4,     9,    23,
      18,   -37,    25,    26,   -37,   -37,    28,    29,    20,    30,
      31,    11,    32,    33,   -37,   -37,    34,    35,   -37,   -37,
      38,    41,    42,    43,     0,   -37,   -37,   -37,   -37,    39,
      45,   -37,   -37,    44,    46,    48,    47,    51,    49,    56,
     -37,    50,   -37,   -37,   -37,   -37,   -11,    22,    57,   -37,
      -3,    -2,    58,    59,    55,    60,    61,    62,    52,    63,
     -37,    64,    65,    66,   -37,   -37,   -37,    68,    24,   -37,
     -37,   -37,   -37,    67,    69,    70,    71,   -37,   -37,   -37
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -37,   -37,   -37,   -37,   -37,   -37,   -37,   -37,   -37,   -37,
     -37,   -37,   -37,   -37,   -36,   -37,   -37
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      22,    23,    77,    77,    72,    57,    14,     3,    15,    16,
      73,    78,    78,    58,    59,    18,    24,    79,    81,    40,
      41,    42,    43,    44,    19,    13,    26,    27,    71,    20,
       5,    45,    21,     6,     7,    29,    30,    36,    37,    74,
      75,    94,    95,    33,    28,     0,    32,     0,     0,    49,
      34,    35,    38,    39,    50,    53,    51,    52,    54,    55,
      56,    60,    61,     0,     0,    63,    62,    64,    66,    65,
      67,    68,    69,    88,    76,    82,    83,    84,     0,    87,
       0,     0,    85,    86,    96,    89,    90,    91,    92,    93,
       0,    97,    98,    99
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-37)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int8 yycheck[] =
{
       4,     5,     5,     5,    15,     5,    16,     0,    18,    16,
      21,    14,    14,    13,    14,    21,    20,    20,    20,     8,
       9,    10,    11,    12,    16,    19,    17,    18,    64,    21,
       3,    20,    19,     6,     7,    17,    18,    17,    18,    17,
      18,    17,    18,    17,    21,    -1,    21,    -1,    -1,    17,
      22,    22,    22,    22,    21,    17,    22,    22,    17,    17,
      17,    22,    17,    -1,    -1,    19,    22,    19,    17,    22,
      21,    15,    22,    21,    17,    17,    17,    22,    -1,    17,
      -1,    -1,    22,    22,    17,    22,    22,    22,    22,    21,
      -1,    22,    22,    22
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    24,    25,     0,    27,     3,     6,     7,    26,    28,
      29,    32,    33,    19,    16,    18,    16,    30,    21,    16,
      21,    19,     4,     5,    20,    31,    17,    18,    21,    17,
      18,    34,    21,    17,    22,    22,    17,    18,    22,    22,
       8,     9,    10,    11,    12,    20,    35,    36,    39,    17,
      21,    22,    22,    17,    17,    17,    17,     5,    13,    14,
      22,    17,    22,    19,    19,    22,    17,    21,    15,    22,
      37,    37,    15,    21,    17,    18,    17,     5,    14,    20,
      38,    20,    17,    17,    22,    22,    22,    17,    21,    22,
      22,    22,    22,    21,    17,    18,    17,    22,    22,    22
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
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))

/* Error token number */
#define YYTERROR	1
#define YYERRCODE	256


/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
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
  FILE *yyo = yyoutput;
  YYUSE (yyo);
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
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
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
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
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

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
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




/* The lookahead symbol.  */
int yychar;


#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval YY_INITIAL_VALUE(yyval_default);

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
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
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
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
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

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
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
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
        case 3:
/* Line 1792 of yacc.c  */
#line 177 "parser.y"
    { out = fopen("GeneratedTests.java", "w"); emit_header(); emit_setup(); }
    break;

  case 4:
/* Line 1792 of yacc.c  */
#line 181 "parser.y"
    { fprintf(out, "}\n"); fclose(out); }
    break;

  case 10:
/* Line 1792 of yacc.c  */
#line 196 "parser.y"
    { free((yyvsp[(3) - (4)].str)); }
    break;

  case 11:
/* Line 1792 of yacc.c  */
#line 200 "parser.y"
    { (yyval.str) = strdup(""); }
    break;

  case 12:
/* Line 1792 of yacc.c  */
#line 201 "parser.y"
    {
          size_t L = strlen((yyvsp[(1) - (2)].str))+strlen((yyvsp[(2) - (2)].str))+1;
          (yyval.str) = malloc(L+1);
          strcpy((yyval.str), (yyvsp[(1) - (2)].str)); strcat((yyval.str), (yyvsp[(2) - (2)].str));
          free((yyvsp[(1) - (2)].str)); free((yyvsp[(2) - (2)].str));
      }
    break;

  case 13:
/* Line 1792 of yacc.c  */
#line 210 "parser.y"
    {
          /* store quoted base url token (keep quotes) */
          if (conf_base) free(conf_base);
          conf_base = strdup((yyvsp[(3) - (4)].str));
          (yyval.str) = strdup("");
          free((yyvsp[(3) - (4)].str));
      }
    break;

  case 14:
/* Line 1792 of yacc.c  */
#line 217 "parser.y"
    {
          char *k = unquote((yyvsp[(2) - (5)].str));
          char *v = unquote((yyvsp[(4) - (5)].str));
          if (conf_hdr_count < MAX_CONF_HDRS) {
              conf_hdr_k[conf_hdr_count] = strdup(k);
              conf_hdr_v[conf_hdr_count] = strdup(v);
              conf_hdr_count++;
          }
          free(k); free(v);
          (yyval.str) = strdup("");
          free((yyvsp[(2) - (5)].str)); free((yyvsp[(4) - (5)].str));
      }
    break;

  case 15:
/* Line 1792 of yacc.c  */
#line 232 "parser.y"
    {
          if (check_identifier((yyvsp[(2) - (5)].str))) YYABORT;
          char *val = unquote((yyvsp[(4) - (5)].str));
          add_var((yyvsp[(2) - (5)].str), val);
          free(val); free((yyvsp[(2) - (5)].str)); free((yyvsp[(4) - (5)].str));
      }
    break;

  case 16:
/* Line 1792 of yacc.c  */
#line 238 "parser.y"
    {
          if (check_identifier((yyvsp[(2) - (5)].str))) YYABORT;
          char tmp[32]; snprintf(tmp, sizeof(tmp), "%d", (yyvsp[(4) - (5)].num));
          add_var((yyvsp[(2) - (5)].str), tmp);
          free((yyvsp[(2) - (5)].str));
      }
    break;

  case 17:
/* Line 1792 of yacc.c  */
#line 244 "parser.y"
    {
          fprintf(stderr, "Line %d: expected IDENT after 'let'\n", yylineno);
          error_occurred = 1;
          YYABORT;
      }
    break;

  case 18:
/* Line 1792 of yacc.c  */
#line 249 "parser.y"
    {
          fprintf(stderr, "Line %d: expected IDENT after 'let'\n", yylineno);
          error_occurred = 1;
          YYABORT;
      }
    break;

  case 19:
/* Line 1792 of yacc.c  */
#line 254 "parser.y"
    {
          fprintf(stderr, "Line %d: expected IDENT after 'let'\n", yylineno);
          error_occurred = 1;
          YYABORT;
      }
    break;

  case 20:
/* Line 1792 of yacc.c  */
#line 259 "parser.y"
    {
          fprintf(stderr, "Line %d: expected IDENT after 'let'\n", yylineno);
          error_occurred = 1;
          YYABORT;
      }
    break;

  case 21:
/* Line 1792 of yacc.c  */
#line 267 "parser.y"
    {
          fprintf(out, "  @Test\n");
          fprintf(out, "  void test_%s() throws Exception {\n", (yyvsp[(2) - (5)].str));
          fprintf(out, "%s", (yyvsp[(4) - (5)].str));
          fprintf(out, "  }\n\n");

          free((yyvsp[(2) - (5)].str));
          free((yyvsp[(4) - (5)].str));
      }
    break;

  case 22:
/* Line 1792 of yacc.c  */
#line 278 "parser.y"
    { (yyval.str) = strdup(""); }
    break;

  case 23:
/* Line 1792 of yacc.c  */
#line 279 "parser.y"
    {
          size_t L = strlen((yyvsp[(1) - (2)].str))+strlen((yyvsp[(2) - (2)].str))+1;
          (yyval.str) = malloc(L+1);
          strcpy((yyval.str), (yyvsp[(1) - (2)].str)); strcat((yyval.str), (yyvsp[(2) - (2)].str));
          free((yyvsp[(1) - (2)].str)); free((yyvsp[(2) - (2)].str));
      }
    break;

  case 24:
/* Line 1792 of yacc.c  */
#line 288 "parser.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 25:
/* Line 1792 of yacc.c  */
#line 289 "parser.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 26:
/* Line 1792 of yacc.c  */
#line 293 "parser.y"
    {
          char *url = subst_quoted((yyvsp[(2) - (3)].str));
          char buf[4096];
          snprintf(buf, sizeof(buf),
    "    HttpRequest.Builder b = HttpRequest.newBuilder(URI.create(BASE + %s))\n"
    "      .timeout(Duration.ofSeconds(10))\n"
    "      .GET();\n"
    "    for (var e: DEFAULT_HEADERS.entrySet()) b.header(e.getKey(), e.getValue());\n"
    "    HttpResponse<String> resp = client.send(b.build(), HttpResponse.BodyHandlers.ofString(StandardCharsets.UTF_8));\n\n",  
    url
);
          (yyval.str) = strdup(buf);
          free(url); free((yyvsp[(2) - (3)].str));
      }
    break;

  case 27:
/* Line 1792 of yacc.c  */
#line 307 "parser.y"
    {
          fprintf(stderr, "Line %d: expected ';' after request\n", yylineno);
          error_occurred = 1;
          YYABORT;
      }
    break;

  case 28:
/* Line 1792 of yacc.c  */
#line 312 "parser.y"
    {
          char *url = subst_quoted((yyvsp[(2) - (3)].str));
          char buf[4096];
          snprintf(buf, sizeof(buf),
    "    HttpRequest.Builder b = HttpRequest.newBuilder(URI.create(BASE + %s))\n"
    "      .timeout(Duration.ofSeconds(10))\n"
    "      .DELETE();\n"
    "    for (var e: DEFAULT_HEADERS.entrySet()) b.header(e.getKey(), e.getValue());\n"
    "    HttpResponse<String> resp = client.send(b.build(), HttpResponse.BodyHandlers.ofString(StandardCharsets.UTF_8));\n\n",
    url
);
          (yyval.str) = strdup(buf);
          free(url); free((yyvsp[(2) - (3)].str));
      }
    break;

  case 29:
/* Line 1792 of yacc.c  */
#line 326 "parser.y"
    {
          fprintf(stderr, "Line %d: expected ';' after request\n", yylineno);
          error_occurred = 1;
          YYABORT;
      }
    break;

  case 30:
/* Line 1792 of yacc.c  */
#line 331 "parser.y"
    {
    char *url = subst_quoted((yyvsp[(2) - (6)].str));
    
    // Separate body and headers
    char *body_code = "HttpRequest.BodyPublishers.noBody()";
    char header_code[4096] = "";
    
    char *items = (yyvsp[(4) - (6)].str);
    char *body_marker = strstr(items, "BODY_MARKER:");
    if (body_marker) {
        // Extract body code
        body_code = body_marker + 12; // Skip "BODY_MARKER:"
        
        // Extract headers (everything before body marker)
        char *body_start = body_marker;
        *body_start = '\0'; // Terminate headers part
        strncpy(header_code, items, sizeof(header_code)-1);
    } else {
        strncpy(header_code, items, sizeof(header_code)-1);
    }
    
    char buf[8192];
    snprintf(buf, sizeof(buf),
        "    HttpRequest.Builder b = HttpRequest.newBuilder(URI.create(BASE + %s))\n"
        "        .timeout(Duration.ofSeconds(10))\n"
        "        .POST(%s);\n"
        "%s"  // header code
        "    for (var e: DEFAULT_HEADERS.entrySet()) b.header(e.getKey(), e.getValue());\n"
        "    HttpResponse<String> resp = client.send(b.build(), HttpResponse.BodyHandlers.ofString(StandardCharsets.UTF_8));\n\n",
        url, body_code, header_code
    );
    
    (yyval.str) = strdup(buf);
    free(url); free((yyvsp[(2) - (6)].str)); free((yyvsp[(4) - (6)].str));
}
    break;

  case 31:
/* Line 1792 of yacc.c  */
#line 366 "parser.y"
    {
          fprintf(stderr, "Line %d: expected ';' after request\n", yylineno);
          error_occurred = 1;
          YYABORT;
      }
    break;

  case 32:
/* Line 1792 of yacc.c  */
#line 371 "parser.y"
    {
    char *url = subst_quoted((yyvsp[(2) - (6)].str));

    // Separate headers and body
    char *body_code = "HttpRequest.BodyPublishers.noBody()";
    char header_code[4096] = "";

    char *items = (yyvsp[(4) - (6)].str);
    char *body_marker = strstr(items, "BODY_MARKER:");

    if (body_marker) {
        // Extract body part after marker
        body_code = strdup(body_marker + strlen("BODY_MARKER:"));

        // Truncate items at marker → keeps only header code
        *body_marker = '\0';
        strncpy(header_code, items, sizeof(header_code) - 1);
    } else {
        strncpy(header_code, items, sizeof(header_code) - 1);
    }

    char buf[8192];
    snprintf(buf, sizeof(buf),
        "    HttpRequest.Builder b = HttpRequest.newBuilder(URI.create(BASE + %s))\n"
        "        .timeout(Duration.ofSeconds(10))\n"
        "        .PUT(%s);\n"
        "    for (var e : DEFAULT_HEADERS.entrySet()) b.header(e.getKey(), e.getValue());\n"
        "%s" // request-level headers appended *after* default headers
        "    HttpResponse<String> resp = client.send(b.build(), HttpResponse.BodyHandlers.ofString(StandardCharsets.UTF_8));\n\n",
        url, body_code, header_code
    );

    (yyval.str) = strdup(buf);
    free(url);
    free((yyvsp[(2) - (6)].str));
    free((yyvsp[(4) - (6)].str));
    free(body_code);
}
    break;

  case 33:
/* Line 1792 of yacc.c  */
#line 410 "parser.y"
    {
          fprintf(stderr, "Line %d: expected ';' after request\n", yylineno);
          error_occurred = 1;
          YYABORT;
      }
    break;

  case 34:
/* Line 1792 of yacc.c  */
#line 418 "parser.y"
    { (yyval.str) = strdup(""); }
    break;

  case 35:
/* Line 1792 of yacc.c  */
#line 419 "parser.y"
    {
          size_t L = strlen((yyvsp[(1) - (2)].str))+strlen((yyvsp[(2) - (2)].str))+1;
          (yyval.str) = malloc(L+1);
          strcpy((yyval.str), (yyvsp[(1) - (2)].str)); strcat((yyval.str), (yyvsp[(2) - (2)].str));
          free((yyvsp[(1) - (2)].str)); free((yyvsp[(2) - (2)].str));
      }
    break;

  case 36:
/* Line 1792 of yacc.c  */
#line 428 "parser.y"
    {
          char *kq = subst_quoted((yyvsp[(2) - (5)].str));
          char *vq = subst_quoted((yyvsp[(4) - (5)].str));
          char buf[512];
          snprintf(buf, sizeof(buf), "    b.header(%s, %s);\n", kq, vq);
          (yyval.str) = strdup(buf);
          free(kq); free(vq); free((yyvsp[(2) - (5)].str)); free((yyvsp[(4) - (5)].str));
      }
    break;

  case 37:
/* Line 1792 of yacc.c  */
#line 436 "parser.y"
    {
          if (check_body_type(1)) YYABORT;
          char *bodyq = subst_quoted((yyvsp[(3) - (4)].str));
          char buf[1024];
          // Mark this section as body content
          snprintf(buf, sizeof(buf), "BODY_MARKER:HttpRequest.BodyPublishers.ofString(%s)", bodyq);
          (yyval.str) = strdup(buf);
          free(bodyq); free((yyvsp[(3) - (4)].str));
      }
    break;

  case 38:
/* Line 1792 of yacc.c  */
#line 445 "parser.y"
    {
          if (check_body_type(0)) YYABORT;
          (yyval.str) = strdup("");
      }
    break;

  case 39:
/* Line 1792 of yacc.c  */
#line 453 "parser.y"
    {
          if (check_status_type(1)) YYABORT;
          char buf[128];
          snprintf(buf, sizeof(buf), "    assertEquals(%d, resp.statusCode());\n", (yyvsp[(4) - (5)].num));
          (yyval.str) = strdup(buf);
      }
    break;

  case 40:
/* Line 1792 of yacc.c  */
#line 459 "parser.y"
    {
          if (check_status_type(0)) YYABORT;
          (yyval.str) = strdup(""); // dummy return
      }
    break;

  case 41:
/* Line 1792 of yacc.c  */
#line 463 "parser.y"
    {
          char *s = subst_quoted((yyvsp[(4) - (5)].str));
          char buf[512];
          snprintf(buf, sizeof(buf), "    assertTrue(resp.body().contains(%s));\n", s);
          (yyval.str) = strdup(buf);
          free(s); free((yyvsp[(4) - (5)].str));
      }
    break;

  case 42:
/* Line 1792 of yacc.c  */
#line 470 "parser.y"
    {
          char *k = subst_quoted((yyvsp[(3) - (6)].str));
          char *v = subst_quoted((yyvsp[(5) - (6)].str));
          char buf[512];
          snprintf(buf, sizeof(buf),
            "    assertEquals(%s, resp.headers().firstValue(%s).orElse(\"\"));\n",
            v, k
          );
          (yyval.str) = strdup(buf);
          free(k); free(v); free((yyvsp[(3) - (6)].str)); free((yyvsp[(5) - (6)].str));
      }
    break;

  case 43:
/* Line 1792 of yacc.c  */
#line 481 "parser.y"
    {
          char *k = subst_quoted((yyvsp[(3) - (6)].str));
          char *v = subst_quoted((yyvsp[(5) - (6)].str));
          char buf[512];
          snprintf(buf, sizeof(buf),
            "    assertTrue(resp.headers().firstValue(%s).orElse(\"\").contains(%s));\n",
            k, v
          );
          (yyval.str) = strdup(buf);
          free(k); free(v); free((yyvsp[(3) - (6)].str)); free((yyvsp[(5) - (6)].str));
      }
    break;


/* Line 1792 of yacc.c  */
#line 2032 "parser.tab.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
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
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
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

  /* Else will try to reuse lookahead token after shifting the error
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
      if (!yypact_value_is_default (yyn))
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

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
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


/* Line 2055 of yacc.c  */
#line 494 "parser.y"


int main() {
    yyparse();
    return error_occurred;
}

void yyerror(const char *s) {
    // Only show generic syntax error if no custom error has been shown yet
    if (!error_occurred && strstr(s, "syntax error") != NULL) {
        fprintf(stderr, "Line %d: syntax error\n", yylineno);
        error_occurred = 1;
    }
}