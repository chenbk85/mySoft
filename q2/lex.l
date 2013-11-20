%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "yacc.tab.hpp"

%}

%option yylineno
%option noyywrap
%option c++

ODIGIT		[0-7]
NZODIGIT	[1-7]
DIGIT		[0-9]
NZDIGIT		[1-9]
XDIGIT		[0-9a-fA-F]
NZXDIGIT	[1-9a-fA-F]
IDSTART	[a-zA-Z_]
IDEND	[a-zA-Z0-9_]

%%
"AND"	{ return AND; }
"OR"	{ return OR; }
"NOT"	{ return NOT; }
":"	{ return COLON; }
"<="	{ return LE; }
">="	{ return GE; }
"<"	{ return LT; }
">"	{ return GT; }
"=="	{ return EQ; }
"!="	{ return NE; }
"("	{ return '('; }
")"	{ return ')'; }
"["	{ return '['; }
"]"	{ return ']'; }


{IDSTART}{IDEND}*	{ return NAME; }

0.{DIGIT}
.{DIGIT}+
{NZDIGIT}{DIGIT}*\.
0${DIGIT}
{NZDIGIT}{DIGIT}*	{ return INT; }
(0x|0X){XDIGIT}+	{ return INT; }


%%
