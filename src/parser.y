%{
package main

import "fmt"
var Final *Language
%}

%union {
	i64 int64
	str string
	real float64	
	config *Configuration
	cell Cell
	ccell *CCell
	ccells []*CCell
	ccellBody *CCellBody
	syntax []*Subsort
	subsort *Subsort
	rule *Rule
	rules []*Rule
	term K
	label Label
	term_list *TermList
	term_list_item TermListItem
	// kraless_term_list *TermListList
	variable *Variable
	cell_attributes CellAttributes
	bag Bag
	bag_item BagItem
	mymap Map
	map_item MapItem
	when_clause *When
	// kra []K
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
%type <ccell> ccell
%type <ccells> ccells
%type <ccellBody> ccellBody
%type <syntax> syntax syntax_rules
%type <subsort> subsort
%type <rules> rules
%type <rule> rule
%type <term> term
%type <label> label
%type <term_list> term_list
%type <term_list_item> term_list_item
// %type <kraless_term_list> kraless_term_list
%type <variable> term_variable bag_variable map_variable
%type <cell_attributes> cell_attributes
%type <bag> bag
%type <bag_item> bag_item
%type <mymap> map
%type <map_item> map_item
%type <when_clause> when_clause
// %type <kra> kra
%type <i64> integer
// these apparently encode precedence, so be careful
%token <str> TOK_ERROR TOK_UC_NAME TOK_LC_NAME TOK_STRING TOK_CELL_BEGIN_K TOK_CELL_BEGIN_BAG TOK_CELL_BEGIN_MAP
%token <str> TOK_MAPS_TO_PRE TOK_CONFIGURATION TOK_RULE TOK_SYNTAX
%token <str> TOK_ARROW TOK_KRA TOK_MAPS_TO TOK_LBRACE TOK_RBRACE
%token <str> TOK_DOT_K TOK_DOT_MAP TOK_DOT_BAG
%token <str> TOK_CELL_RIGHT_OPEN TOK_CELL_RIGHT_CLOSED TOK_CELL_LEFT_OPEN TOK_WHEN TOK_BANG_COLON
%token <i64> TOK_INTEGER 
%token <real> TOK_REAL
%token <val> TOK_ERR

%left TOK_KRA
%left TOK_ARROW
%left ','


%%
final
	: TOK_CONFIGURATION configuration syntax rules
		{ Final = &Language{Configuration: $2, Syntax: $3, Rules: $4} }
	;

configuration
	: ccell
		{ $$ = &Configuration{Cell: $1} }

ccell
	: TOK_CELL_BEGIN_K cell_attributes '>' ccellBody TOK_CELL_RIGHT_CLOSED TOK_LC_NAME '>'
		{
			if $1 != $6 {
				panic(fmt.Sprintf("cell %s isn't %s", $1, $6))
			}
			for k, v := range $2.Table {
				if k == "type" {
					// fmt.Printf("%s has type %s\n", $1, v)
					CellTypes[$1] = v
				}
			}
			if _, ok := CellTypes[$1]; !ok {
				CellTypes[$1] = "k"
			}

			$$ = &CCell{Name: $1, Attributes: $2, Children: $4.Children, Magic: $4.Magic}
		}

ccellBody
	: ccells
		{ $$ = &CCellBody{Children: $1} }
	| TOK_UC_NAME
		{ $$ = &CCellBody{Children: []*CCell{}, Magic: $1} }

ccells
	: // empty
		{ $$ = []*CCell{} }
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
syntax
	: // empty
		{ $$ = []*Subsort{} }
	| TOK_SYNTAX syntax_rules
		{ $$ = $2 }

syntax_rules
	: // empty
		{ $$ = []*Subsort{} }
	| syntax_rules subsort
		{ $$ = append($1, $2) }

subsort
	: TOK_LC_NAME ':' TOK_LC_NAME
		{ $$ = &Subsort{Sort: $3, Subsort: $1} }

rules
	: // empty
		{ $$ = []*Rule{} }
	| rules rule
		{ $$ = append($1, $2) }

rule
	: TOK_RULE bag when_clause
		{ $$ = &Rule{Bag: $2, When: $3} }
	| TOK_RULE bag
		{ $$ = &Rule{Bag: $2} }

when_clause
	: TOK_WHEN term
		{ $$ = &When{Term: $2} }

term
	// : kra
	// 	{ $$ = &Kra{Children: $1} }
	: term_variable
		{ $$ = $1 }
	/*| term TOK_ARROW term
		{
			// fmt.Printf("Arrow(%s, %s)", &$1, &$3)
			// $$ = Term{Type: TermRewrite, Rewrite: &Rewrite{LHS: &Term{Type: TermVariable, Variable: "foo"}, RHS: &$3}}  // &$1
			$$ = &Rewrite{LHS: $1, RHS: $3}
		}*/
	| label '(' term_list ')'
		{ $$ = &Appl{Label: $1, Body: $3} }
	| '(' term ')'
		{ $$ = &Paren{Body: $2} }
	| TOK_DOT_K
		{ $$ = &DotK{} }

/*
kra 
	: term TOK_KRA term
		{ $$ = []K{$1, $3} }
	| kra TOK_KRA term
		{ $$ = append($1, $3) }
*/

term_variable
	: TOK_UC_NAME
		{ $$ = &Variable{Name: $1, Sort: "k", Default: true} }
	| TOK_UC_NAME ':' TOK_LC_NAME
		{ $$ = &Variable{Name: $1, Sort: $3} }
	| TOK_UC_NAME TOK_BANG_COLON TOK_LC_NAME
		{ $$ = &Variable{Name: $1, Sort: $3, ReverseSort: true} }

bag_variable
	: TOK_UC_NAME
		{ $$ = &Variable{Name: $1, Sort: "bag", Default: true} }
	| TOK_UC_NAME ':' TOK_LC_NAME
		{ $$ = &Variable{Name: $1, Sort: $3} }

map_variable
	: TOK_UC_NAME
		{ $$ = &Variable{Name: $1, Sort: "map", Default: true} }
	| TOK_UC_NAME ':' TOK_LC_NAME
		{ $$ = &Variable{Name: $1, Sort: $3} }	

label
	: TOK_LC_NAME
		{ $$ = &NameLabel{Name: $1} }
	| TOK_LC_NAME TOK_LBRACE integer TOK_RBRACE
		{ $$ = &InjectLabel{Name: $1, Type: E_inject_integer, Int: $3} }
	| '(' label TOK_ARROW label ')'
		{ $$ = &RewriteLabel{LHS: $2, RHS: $4} }

integer
	: TOK_INTEGER
		{ $$ = $1 }

term_list
	: // empty
		{ $$ = &TermList{[]TermListItem{}} }
	| term_list_item
		{ $$ = &TermList{[]TermListItem{$1}} }
	| term_list ',' term_list
		{
			$1.Children = append($1.Children, $3.Children...)
			$$ = $1
		}
	| '(' term_list ',' term_list ')'
		{
			$2.Children = append($2.Children, $4.Children...)
			$$ = $2
		}
	/*| '(' term_list ',' term_list_item ')'
		{
			$2.Children = append($2.Children, $4)
			$$ = $2
		}*/
	/*| term
		{ $$ = &TermList{[]TermListItem{&TermListKItem{$1}}} }
	| '(' term_list TOK_ARROW term_list ')'
		// { $$ = $2 }
		{ $$ = &TermListRewrite{LHS: $2, RHS: $4} }
	| term_list ',' term
		{ $1.Children = append($1.Children, $3) }
		*/
/*
true_term_list
	: term_list ',' term_list_item
		{
			$1.Children = append($1.Children, $3)
			$$ = $1
		}
	| paren_term_list
		{ $$ = $1 }

paren_term_list
	: '(' term_list ',' term_list_item ')'
		{
*/
term_list_item
	: term
		{ $$ = &TermListKItem{$1} }
	| term_list TOK_ARROW term_list
		{ $$ = &TermListRewrite{LHS: $1, RHS: $3} }
	| '(' term_list TOK_ARROW term_list ')'
		{ $$ = &TermListRewrite{LHS: $2, RHS: $4} }

/*
kraless_term_list
	: // empty
		{ $$ = &TermListList{[]K{}} }
	| term
		{ $$ = &TermListList{[]K{$1}} }
	| kraless_term_list ',' term
		{
			// tll := $1
			// tll.Elements = append(tll.Elements, $3)
			// $$ = tll
			$$ = $1
		}
*/

bag
	: bag_item
		{ $$ = Bag{$1} }
	| bag bag_item
		{ $$ = append($1, $2) }

bag_item
	: cell
		{ $$ = $1 }
	| bag_variable
		{ $$ = $1 }

map
	: map_item
		{ $$ = Map{$1} }
	| map map_item
		{ $$ = append($1, $2) }

map_item
	: map_variable
		{ $$ = $1 }
	| TOK_DOT_MAP
		{ $$ = &DotMap{} }
	| '(' term TOK_MAPS_TO term ')'
	// | TOK_MAPS_TO_PRE '(' term ',' term ')'
		{ $$ = &Mapping{LHS: $2, RHS: $4} }
	| map_item TOK_ARROW map_item
	 	{ $$ = &RewriteMapItem{LHS: $1, RHS: $3} }

cell
	: TOK_CELL_BEGIN_K '>' term_list TOK_CELL_RIGHT_CLOSED TOK_LC_NAME '>'
		{
			if $1 != $5 {
				panic(fmt.Sprintf("cell %s isn't %s", $1, $5))
			}
			$$ = &ComputationCell{Name: $1, Computation: $3}
		}
	| TOK_CELL_BEGIN_BAG '>' bag TOK_CELL_RIGHT_CLOSED TOK_LC_NAME '>'
		{
			if $1 != $5 {
				panic(fmt.Sprintf("cell %s isn't %s", $1, $5))
			}
			$$ = &BagCell{Name: $1, Bag: $3}
		}
	| TOK_CELL_BEGIN_MAP '>' map TOK_CELL_RIGHT_CLOSED TOK_LC_NAME '>'
		{
			if $1 != $5 {
				panic(fmt.Sprintf("cell %s isn't %s", $1, $5))
			}
			$$ = &MapCell{Name: $1, Map: $3}
		}

%%
