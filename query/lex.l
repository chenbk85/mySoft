%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "yacc.tab.hpp"
#include "ccFlexLexer.hpp"

using namespace yy;
typedef yy::query_parser::token token;

#undef YY_DECL
#define YY_DECL int ccFlexLexer::yylex(yy::query_parser::semantic_type* yylval)

//#define DLEX(str...) fprintf(stderr, str)
#define DLEX(str...) do {} while(0)

%}

/*%option yylineno */
%option noyywrap
%option c++
%option yyclass="ccFlexlexer"

ODIGIT		[0-7]
DIGIT		[0-9]
XDIGIT		[0-9a-fA-F]
ID1		[a-zA-Z_]
ID2		[a-zA-Z0-9_]

%%

"AND"	{ DLEX("AND\n"); return token::AND; }
"OR"	{ DLEX("OR\n"); return token::OR; }
"NOT"	{ DLEX("NOT\n"); return token::NOT; }
":"	{ DLEX(":\n"); return token::COLON; }
","	{ DLEX(",\n"); return token::COMMA; }
"<="	{ DLEX("<=\n"); return token::LE; }
">="	{ DLEX(">=\n"); return token::GE; }
"<"	{ DLEX("<\n"); return token::LT; }
">"	{ DLEX(">\n"); return token::GT; }
"=="	{ DLEX("==\n"); return token::EQ; }
"!="	{ DLEX("!=\n"); return token::NE; }
"("	{ DLEX("(\n"); return token::LP; }
")"	{ DLEX(")\n"); return token::RP; }
"["	{ DLEX("[\n"); return token::LS; }
"]"	{ DLEX("]\n"); return token::RS; }

{ID1}{ID2}* { 
	DLEX("ID: %s\n", yytext);
	yylval->sval = strdup(yytext);
	return token::ID; 
}

("0x"|"0X"){XDIGIT}+ {
	DLEX("hex: %s\n", yytext);
	yylval->ival = strtoul(yytext, 0, 0);
	return token::INT;
}

"0"{ODIGIT}+ {
	DLEX("oct: %s\n", yytext);
	yylval->ival = strtoul(yytext, 0, 0);
	return token::INT;
}

{DIGIT}+ {
	DLEX("dec: %s\n", yytext);
	yylval->ival = strtoul(yytext, 0, 0);
	return token::INT;
}

("."{DIGIT}+|{DIGIT}+"."|{DIGIT}+"."{DIGIT}+)(e{DIGIT}+)? {
	DLEX("de?-double: %s\n", yytext);
	yylval->dval = strtod(yytext, 0);
	return token::DOUBLE;
}

{DIGIT}+e{DIGIT}+ {
	DLEX("iei-dobule: %s\n", yytext);
	yylval->dval = strtod(yytext, 0);
	return token::DOUBLE;
}

"\"".*"\"" {
	DLEX("qstring: %s\n", yytext);
	yylval->sval = strdup(yytext);
	return token::STRING;
}

[^\(\)\[\]\*\:\,\" ]+ {
	DLEX("string: %s\n", yytext);
	yylval->sval = strdup(yytext);
	return token::STRING;
}

[[:blank:]]+ {
	DLEX("eat up spaces: [%s]\n", yytext);
	// eat up
}

<<EOF>> {
	return token::END;
}

%%

#if 0
struct yy_buffer_state init_static_buffer_state(struct yy_buffer_state* bs)
{
	bs.yy_input_file = NULL;
	bs.yy_ch_buf = ibuf;

	bs->yy_ch_buf[0] = YY_END_OF_BUFFER_CHAR;
	bs->yy_ch_buf[1] = YY_END_OF_BUFFER_CHAR;

	b->yy_buf_pos = &b->yy_ch_buf[0];
	bs.yy_buf_pos = bs.yy_ch_buf;
	bs.yy_buf_size = isize;
	bs.yy_n_chars = isize;
	bs.yy_is_our_buffer = 0;
	bs.yy_is_interactive = 0;
	bs.yy_at_bol = 0;
	bs.yy_fill_buffer = 0;
	bs.yy_buffer_status = YY_BUFFER_NEW;
}
#endif
