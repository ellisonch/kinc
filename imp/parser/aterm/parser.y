%{
package aterm

var FinalTerm ATerm
%}

%union {
	i64 int64
	str string
	real float64
	lst []ATerm
	at ATerm
}

%type <at> aterm
%type <lst> comma_list
%token <str> TOK_CONSTRUCTOR TOK_STRING 
%token <i64> TOK_INTEGER 
%token <real> TOK_REAL
%token <val> TOK_ERR

%%
final
	: aterm
		{ FinalTerm = $1 }
	;

aterm
	: TOK_STRING
		{ $$ = ATerm{Type: String, Str: $1} }
	| TOK_INTEGER
		{ $$ = ATerm{Type: Int, Int: $1} }
	| TOK_REAL
		{ $$ = ATerm{Type: Real, Real: $1} }
	| TOK_CONSTRUCTOR '(' comma_list ')'
		{ $$ = ATerm{Type: Appl, Appl: ATermAppl{$1, []ATerm($3)}} }
	| TOK_CONSTRUCTOR
		{ $$ = ATerm{Type: Appl, Appl: ATermAppl{$1, nil}} }
	| '[' comma_list ']' 
		{ $$ = ATerm{Type: List, List: ATermList($2)} }
	;


comma_list
	: // empty 
		{$$ = []ATerm{}}
	| aterm
		{$$ = []ATerm{$1}}
	| comma_list ',' aterm
		{$$ = append($1, $3)}
	;


%%
