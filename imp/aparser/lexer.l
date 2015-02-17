%{
#include <stdio.h>
#include <stdint.h>

#include "aterm.h"

#define YY_DECL int yylex()

#include "parser.tab.h"
%}

%option noyywrap

%%

["][$A-Za-z0-9]*["] {
	char* p = malloc(strlen(yytext) + 1);
	memcpy(p, yytext+1, strlen(yytext)-2);
	p[strlen(yytext)-2] = '\0';
	yylval.str = p; 
	return TOK_STRING;
}

[A-Za-z#][A-Za-z0-9]* {
	char* p = malloc(strlen(yytext) + 1);
	strcpy(p, yytext);
	yylval.str = p; 
	return TOK_CONSTRUCTOR;
}

[0-9]+ {
	yylval.i64 = atoi(yytext);
	return TOK_I64; 
}

","	{ return ','; }
"("	{ return '('; }
")"	{ return ')'; }
"{"	{ return '{'; }
"}"	{ return '}'; }
"["	{ return '['; }
"]"	{ return ']'; }

%%