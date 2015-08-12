// Define a grammar called Hello
grammar Imp;

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
	: statements
;
statements 
	: statement (';' statement)*
	;
statement 
	: assign
	| var
	| while_
	;

assign 
	: id ':=' int_expression
	;
var 
	: 'var' id_list
	;
while_
	: 'while' bool_expression 'do' '(' statements ')'
	;

id_list 
	: id (',' id)*
	;

int_expression 
	: '(' int_expression ')'
	| '-' int_expression
	| int_expression '+' int_expression 
	| int_expression '-' int_expression
	| int_expression '*' int_expression
	| int_expression '/' int_expression
	| int_literal
	| id
	;
plus
	: int_expression '+' int_expression
	;
bool_expression
	: '(' bool_expression ')'
	| int_expression '<=' int_expression
	| 'not' bool_expression
	;

int_literal
	: raw_int
	;

id
	: raw_string_id
	;

raw_string_id
	: Id
	;

raw_int
	: Int_literal
	; 

Int_literal : [0-9]+ ;
Id : [a-zA-Z][a-zA-Z0-9]* ;
WS : [ \t\r\n]+ -> skip ; // skip spaces, tabs, newlines