%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "aterm.h"

extern int yylex();
extern int yyparse();
extern FILE* yyin;
void yyerror(const char* s);

aterm final_term;

%}

// %define api.pure full

%union {
	int64_t i64;
	char* str;
	double real;
	at_list* lst;
	aterm at;
}

%type <at> aterm
%type <lst> comma_list

%token <str> TOK_CONSTRUCTOR TOK_STRING 
%token <i64> TOK_I64
%token <real> TOK_REAL
%token <val> TOK_ERR
%start final

%%

final
	: aterm
		{ final_term = $1; }
	;

aterm
	: TOK_STRING
		{ $$ = ((aterm){ .type = AT_STRING, .string = $1 }); } //  ATerm{Type: String, Str: $1}; 
	| TOK_I64
		{ $$ = ((aterm){ .type = AT_INT64, .int64 = $1 }); } // ATerm{Type: Int, Int: $1};
	// | TOK_REAL
		// { $$ = ((aterm){ .type = AT_REAL, .real = $1 }); } // ATerm{Type: Real, Real: $1};
	| TOK_CONSTRUCTOR '(' comma_list ')'
		{
			at_appl appl = { .name = $1, .args = $3 };
			$$ = ((aterm){ .type = AT_APPL, .appl = appl }); 
		} // ATerm{Type: Appl, Appl: ATermAppl{$1, []ATerm($3)}};  // FIXME at_appl
	| TOK_CONSTRUCTOR
		{ 
			at_appl appl = { .name = $1, .args = NULL };
			$$ = ((aterm){ .type = AT_APPL, .appl = appl }); 
		} // ATerm{Type: Appl, Appl: ATermAppl{$1, nil}}; // FIXME 
	// | '[' comma_list ']' 
		// { $$ = ((aterm){ .type = AT_LIST }); } // ATerm{Type: List, List: ATermList($2)};
	;


comma_list
	: // empty
		{ $$ = NULL; } // []ATerm{};
	| aterm
		{ $$ = at_list_append(NULL, $1); } // append($1, $3);
	| comma_list ',' aterm
		{ $$ = at_list_append($1, $3); } // append($1, $3);
	;

%%
/*
main() {
	setvbuf(stdout, NULL, _IONBF, 0);
	yyin = stdin;
	do {
		yyparse();
	} while(!feof(yyin));
	printf("Getting ready to to_string\n");
	char* sterm = aterm_to_string(final_term);
	printf("Back from to_string\n");
	printf("%s\n", sterm);
}
*/
void yyerror(const char* s) {
	fprintf(stderr, "Parse error: %s\n", s);
	exit(1);
}
