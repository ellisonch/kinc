// Define a grammar called Hello
grammar ImpUgly;

/*
var n, s ;
n := 100 ;
s := 0 ;
while not(n <= 0) do (
	s := s + n ;
	n := n + -1
)
*/

program 
	: statements int_expression
		{ System.out.println("Program(" + $statements.s + ", " + $int_expression.s + ")"); }
;
statements returns [String s] locals [String temp = ""]
	: 
		{ $s = "Skip()"; }
	| a=statement (';' b=statement {$temp += "," + $b.s;})*
		// { $s = "Statements([" + $a.s + $temp + "])"; }
		{ $s = "Statements(" + $a.s + $temp + ")"; }
	;
statement returns [String s]
	: assign
		{ $s = $assign.s; }
	| var
		{ $s = $var.s; }
	| while_
		{ $s = $while_.s; }
	| if_
		{ $s = $if_.s; }	
	;

assign returns [String s]
	: id '=' int_expression
		{ $s = "Assign(" + $id.s + ", " + $int_expression.s + ")"; }
	;
var returns [String s]
	: 'var' id_list
		{ $s = "Var(" + $id_list.s + ")"; }
	;
while_ returns [String s]
	: 'while' bool_expression '{' statements '}'
		{ $s = "While(" + $bool_expression.s + ", " + $statements.s + ")"; }
	;
if_ returns [String s]
	: 'if' a=bool_expression '{' b=statements '}' 'else' '{' c=statements '}'
		{ $s = "If(" + $a.s + ", " + $b.s + ", " + $c.s + ")"; }
	;

id_list returns [String s] locals [String temp = ""]
	: a=id (',' b=id {$temp += "," + $b.s;})*
		{ $s = $a.s + $temp; }
	;

int_expression returns [String s]
	: '(' int_expression ')'
		{ $s = "Paren(" + $int_expression.s + ")"; }
	| '-' a=int_expression
		{ $s = "Neg(" + $a.s + ")"; }
	| a=int_expression '+' b=int_expression
		{ $s = "Plus(" + $a.s + ", " + $b.s + ")"; }
	| a=int_expression '-' b=int_expression
		{ $s = "Minus(" + $a.s + ", " + $b.s + ")"; }
	| a=int_expression '*' b=int_expression
		{ $s = "Times(" + $a.s + ", " + $b.s + ")"; }
	| a=int_expression '/' b=int_expression
		{ $s = "Div(" + $a.s + ", " + $b.s + ")"; }
	| int_literal
		{ $s = $int_literal.s; }
	| hole_expression
		{ $s = $hole_expression.s; }
	| id
		{ $s = $id.s; }
	;
bool_expression returns [String s]
	: '(' b=bool_expression ')'
		{ $s = "Paren(" + $b.s + ")"; }
	| i1=int_expression '<=' i2=int_expression
		{ $s = "LTE(" + $i1.s + ", " + $i2.s + ")"; }
	| b1=bool_expression '&&' b2=bool_expression
		{ $s = "And(" + $b1.s + ", " + $b2.s + ")"; }
	| 'not' b=bool_expression
		{ $s = "Not(" + $b.s + ")"; }
	;

hole_expression returns [String s]
	: Hole_expression
		{ $s = "#Hole(\"" + $Hole_expression.text + "\")"; }
	;

int_literal returns [String s]
	: Int_literal
		{ $s = "#Int(" + $Int_literal.text + ")"; }
	;

id returns [String s]
	: Id
		{ $s = "Id(#String(\"" + $Id.text + "\"))"; }
	;

Int_literal : [0-9]+ ;
Hole_expression : [$][a-zA-Z][a-zA-Z0-9]* ;
Id : [a-zA-Z][a-zA-Z0-9]* ;
WS : [ \t\r\n]+ -> skip ; // skip spaces, tabs, newlines
LINE_COMMENT
    : '//' ~[\r\n]* -> skip
;