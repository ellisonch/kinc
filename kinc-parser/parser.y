%{
package kinc

import "fmt"
var Final KincDefinition
%}

%union {
	i64 int64
	str string
	real float64	
	config Configuration
	cell Cell
	cells []Cell
	ccell CCell
	ccells []CCell
	rule Rule
	rules []Rule
	term Term
	label Label
	term_list []Term
}
/*
lst []ATerm
at ATerm
*/

/*
%type <at> aterm
%type <lst> comma_list
*/
%type <config> configuration
%type <cell> cell
%type <cells> cells
%type <ccell> ccell
%type <ccells> ccells
%type <rules> rules
%type <rule> rule
%type <term> term
%type <label> label
%type <term_list> term_list

// these apparently encode precedence, so be careful
%token <str> TOK_UC_NAME TOK_LC_NAME TOK_STRING TOK_CONFIGURATION TOK_RULE
%token <str> TOK_ARROW TOK_BEGIN_END
%token <i64> TOK_INTEGER 
%token <real> TOK_REAL
%token <val> TOK_ERR

%left TOK_ARROW

%%
final
	: TOK_CONFIGURATION configuration rules
		{ Final = KincDefinition{Configuration: $2, Rules: $3} }
	;

configuration
	: ccell
		{ $$ = Configuration{Cell: $1} }

ccell
	: '<' TOK_LC_NAME '>' ccells TOK_BEGIN_END TOK_LC_NAME '>'
		{
			if $2 != $6 {
				panic(fmt.Sprintf("cell %s isn't %s", $2, $6))
			}
			$$ = CCell{Name: $2, Children: $4}
		}

ccells
	: // empty
		{ $$ = []CCell{} }
	| ccells ccell
		{ $$ = append($1, $2) }

rules
	: // empty
		{ $$ = []Rule{} }
	| rules rule
		{ $$ = append($1, $2) }

rule
	: TOK_RULE term
		{ $$ = Rule{Term: &$2} }

term
	: TOK_UC_NAME
		{ $$ = Term{Type: TermVariable, Variable: $1} }
	| term TOK_ARROW term
		{ $$ = Term{Type: TermRewrite, Rewrite: &Rewrite{LHS: &$1, RHS: &$3}} }
	| label '(' term_list ')'
		{ $$ = Term{Type: TermAppl, Appl: &Appl{Label: &$1, Body: $3}} }
	| cells
		{ $$ = Term{Type: TermCells, Cells: $1} }

label
	: TOK_LC_NAME
		{ $$ = Label{Type: E_LabelName, Name: $1} }
	| '(' label TOK_ARROW label ')'
		{ $$ = Label{Type: E_LabelRewrite, Rewrite: &LabelRewrite{LHS: $2, RHS: $4}} }

term_list
	: // empty
		{ $$ = []Term{} }
	| term
		{ $$ = []Term{$1} }
	| term_list ',' term
		{ $$ = append($1, $3) }


cell
	: '<' TOK_LC_NAME '>' term TOK_BEGIN_END TOK_LC_NAME '>'
		{
			if $2 != $6 {
				panic(fmt.Sprintf("cell %s isn't %s", $2, $6))
			}
			$$ = Cell{Name: $2, Body: &$4}
		}

cells
	: cell
		{ $$ = []Cell{$1} }
	| cells cell
		{ $$ = append($1, $2) }

/*
aterm
	: TOK_STRING
		{ $$ = ATerm{Type: Appl, Appl: ATermAppl{$1, nil}} }
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
*/

%%
