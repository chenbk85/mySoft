%{
#include <iostream>
#include <sstream>

#include "query_exp.h"
#include "document.h"

#define OBJ document_helper
%skeleton "lalr1.cc"
%}

%require "2.3"
%defines

%pure_parser
%locations
%define "parser_class_name" "query_parser"
%parse-param {yyFlexLexer* lexer, query_exp<OBJ>** query}

%union {
	long ival;
	double dval;
	char* sval;
	query_exp<OBJ>* qexp;	
}

%token AND OR NOT COLON LE GE LT GT EQ NE LP RP LS RS END

%left OR AND
%left NOT

%token <ival> INT
%token <dval> DOUBLE
%token <sval> ID STRING
%type  <qexp> query expr fieldexp



%%
query: expr END { YYACCEPT; }
	;
expr: fieldexp { $$ = $1; }
	| '(' expr ')' { $$ = $2; }
	| expr AND expr { $$ = new and_query_exp<OBJ>($1, $3); }
	| expr OR  expr { $$ = new or_query_exp<OBJ>($1, $3); }
	| expr NOT expr { $$ = new not_query_exp<OBJ>($1, $3); }
	;

fieldexp: ID LT INT { $$ = new unary_field_exp<OBJ, long, field_greater<long> >($1, $3); }
	| ID LE INT { $$ = new unary_field_exp<OBJ, long, field_greater_equal<long> >($1, $3); }
	| ID GT INT { $$ = new unary_field_exp<OBJ, long, field_less<long> >($1, $3); }
	| ID GE INT { $$ = new unary_field_exp<OBJ, long, field_less_equal<long> >($1, $3); }
	| ID EQ INT { $$ = new unary_field_exp<OBJ, long, field_equal_to<long> >($1, $3); }
	| ID NE INT { $$ = new unary_field_exp<OBJ, long, field_not_equal_to<long> >($1, $3); }
	| ID COLON LP INT ',' INT RP { $$ = new binary_field_exp<OBJ, long, field_between_ee<long> >($1, $4, $6); }
	| ID COLON LP INT ',' INT RS { $$ = new binary_field_exp<OBJ, long, field_between_ei<long> >($1, $4, $6); }
	| ID COLON LS INT ',' INT RP { $$ = new binary_field_exp<OBJ, long, field_between_ie<long> >($1, $4, $6); }
	| ID COLON LS INT ',' INT RS { $$ = new binary_field_exp<OBJ, long, field_between_ii<long> >($1, $4, $6); }
	;

%%

