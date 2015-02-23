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
	term *Term
	label *Label
	term_list []*Term
	term_variable Variable
	cell_attributes CellAttributes
	bag []*BagItem
	bag_item *BagItem
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
%type <term_variable> term_variable
%type <cell_attributes> cell_attributes
%type <bag> bag
%type <bag_item> bag_item
// these apparently encode precedence, so be careful
%token <str> TOK_UC_NAME TOK_LC_NAME TOK_STRING TOK_CONFIGURATION TOK_RULE TOK_CELL_BEGIN_K TOK_CELL_BEGIN_BAG
%token <str> TOK_ARROW TOK_KRA TOK_MAPS_TO
%token <str> TOK_CELL_RIGHT_OPEN TOK_CELL_RIGHT_CLOSED TOK_CELL_LEFT_OPEN
%token <i64> TOK_INTEGER 
%token <real> TOK_REAL
%token <val> TOK_ERR

%left TOK_ARROW TOK_KRA

%%
final
	: TOK_CONFIGURATION configuration rules
		{ Final = KincDefinition{Configuration: $2, Rules: $3} }
	;

configuration
	: ccell
		{ $$ = Configuration{Cell: $1} }

ccell
	: TOK_CELL_BEGIN_K cell_attributes '>' ccells TOK_CELL_RIGHT_CLOSED TOK_LC_NAME '>'
		{
			if $1 != $6 {
				panic(fmt.Sprintf("cell %s isn't %s", $1, $6))
			}
			for k, v := range $2.Table {
				if k == "type" {
					fmt.Printf("%s has type %s\n", $1, v)
					CellTypes[$1] = v
				}
			}
			if _, ok := CellTypes[$1]; !ok {
				CellTypes[$1] = "k"
			}

			$$ = CCell{Name: $1, Attributes: $2, Children: $4}
		}

ccells
	: // empty
		{ $$ = []CCell{} }
	| ccells ccell
		{ $$ = append($1, $2) }

cell_attributes
	: // empty
		{ $$ = CellAttributes{Table: make(map[string]string)} }
	| cell_attributes TOK_LC_NAME '=' '"' TOK_LC_NAME '"'
		{
			$1.Table[$2] = $5
			$$ = $1
		}

rules
	: // empty
		{ $$ = []Rule{} }
	| rules rule
		{ $$ = append($1, $2) }

rule
	: TOK_RULE bag
		{ $$ = Rule{Bag: $2} }

term
	: term_variable
		{ $$ = &Term{Type: TermVariable, Variable: $1} }
	| term TOK_ARROW term
		{
			// fmt.Printf("Arrow(%s, %s)", &$1, &$3)
			// $$ = Term{Type: TermRewrite, Rewrite: &Rewrite{LHS: &Term{Type: TermVariable, Variable: "foo"}, RHS: &$3}}  // &$1
			$$ = &Term{Type: TermRewrite, Rewrite: Rewrite{LHS: $1, RHS: $3}}
		}
	| term TOK_KRA term
		{
			// fmt.Printf("Arrow(%s, %s)", &$1, &$3)
			// $$ = Term{Type: TermRewrite, Rewrite: &Rewrite{LHS: &Term{Type: TermVariable, Variable: "foo"}, RHS: &$3}}  // &$1
			$$ = &Term{Type: TermKra, Kra: Kra{LHS: $1, RHS: $3}}
		}
	| label '(' term_list ')'
		{ $$ = &Term{Type: TermAppl, Appl: Appl{Label: $1, Body: $3}} }
	| '(' term ')'
		{ $$ = &Term{Type: TermParen, Paren: $2} }

term_variable
	: TOK_UC_NAME
		{ $$ = Variable{Name: $1, Sort: "k"} }
	| TOK_UC_NAME ':' TOK_LC_NAME
		{ $$ = Variable{Name: $1, Sort: $3} }

label
	: TOK_LC_NAME
		{ $$ = &Label{Type: E_LabelName, Name: $1} }
	| '(' label TOK_ARROW label ')'
		{ $$ = &Label{Type: E_LabelRewrite, Rewrite: LabelRewrite{LHS: $2, RHS: $4}} }

term_list
	: // empty
		{ $$ = []*Term{} }
	| term
		{ $$ = []*Term{$1} }
	| term_list ',' term
		{ $$ = append($1, $3) }

bag
	: bag_item
		{ $$ = []*BagItem{$1} }
	| bag bag_item
		{ $$ = append($1, $2) }

bag_item
	: cell
		{ $$ = &BagItem{Type: BagCell, Cell: $1} }

cell
	: TOK_CELL_BEGIN_K '>' term TOK_CELL_RIGHT_CLOSED TOK_LC_NAME '>'
		{
			/*if $2 != $6 {
				panic(fmt.Sprintf("cell %s isn't %s", $2, $6))
			}*/
			$$ = Cell{Name: $1, Body: $3}
		}
	| TOK_CELL_BEGIN_BAG '>' term TOK_CELL_RIGHT_CLOSED TOK_LC_NAME '>'
		{
			/*if $2 != $6 {
				panic(fmt.Sprintf("cell %s isn't %s", $2, $6))
			}*/
			$$ = Cell{Name: $1, Body: $3}
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
