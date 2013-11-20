%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "yacc.tab.hpp"
//%option yylineno
//%option c++
%}

%option noyywrap

ODIGIT		[0-7]
DIGIT		[0-9]
XDIGIT		[0-9a-fA-F]
ID1		[a-zA-Z_]
ID2		[a-zA-Z0-9_]

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
"("	{ return LP; }
")"	{ return RP; }
"["	{ return LS; }
"]"	{ return RS; }

{ID1}{ID2}*	{ return ID; }
("0x"|"0X"){XDIGIT}+	{ return INT; }
"0"{ODIGIT}+	{ return INT; }
{DIGIT}+ { return INT; }
("."{DIGIT}+|{DIGIT}+"."|{DIGIT}+"."{DIGIT}+)(e{DIGIT}+)? { return DOUBLE; }
{DIGIT}+e{DIGIT}+	{ return DOUBLE; }
"\"".*"\"" { return STRING; }
[^[:blank:]]+ { return STRING; }
<<EOF>> { return END; }

%%

