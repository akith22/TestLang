%{
#include "parser.tab.h"
#include <string.h>
#include <stdlib.h>

void yyerror(const char *s);
int line_num = 1;
%}

/* %option yylineno */

digit       [0-9]
letter      [A-Za-z_]
id          {letter}({letter}|{digit})*
string      \"([^\"\\\n\\]|\\.)*\"

%%

"config"                return CONFIG;
"base_url"              return BASE_URL;
"header"                return HEADER;
"let"                   return LET;
"test"                  return TEST;
"GET"                   return GET;
"POST"                  return POST;
"PUT"                   return PUT;
"DELETE"                return DELETE;
"expect"                return EXPECT;
"status"                return STATUS;
"body"                  return BODY;
"contains"              return CONTAINS;
"description"|"DESCRIPTION"   return DESCRIPTION;
"="                     return '=';
"{"                     return '{';
"}"                     return '}';
";"                     return ';';
"("                     return '(';
")"                     return ')';
"["                     return '[';
"]"                     return ']';

{digit}+                { yylval.num = atoi(yytext); return NUMBER; }
{id}                    { yylval.str = strdup(yytext); return IDENT; }
{string}                {
                          /* keep the quotes in the string token text so parser can print them unchanged */
                          yylval.str = strdup(yytext);
                          return STRING;
                        }

"//".*                  ;   /* comment */
[ \t\r]+                ;   /* skip spaces and tabs */
\n                      { yylineno++; }
.                       { fprintf(stderr, "Unknown character on line %d: %s\n", yylineno, yytext); }

%%

int yywrap() { return 1; }