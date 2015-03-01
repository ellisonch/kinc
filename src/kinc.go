package main

import "fmt"
import "os"
import "strings"
import "errors"
import "io"

// import "./parser"
// import "strings"

// func KincVisitor

var prog string = `
configuration 
<t>
	<state type="map"> </state>
	<k type="computation"> </k>
</t>
rule
	<k> div(I1:int, I2:int) => #divInt(I1, I2) ~> K </k>
	when #notEqInt(I2, I2)

rule <k> plus(I1:int, I2:int) => #plusInt(I1, I2) ~> K </k>
rule <k> lte(I1:int, I2:int) => #lteInt(I1, I2) ~> K </k>
rule <k> not(B:bool) => #not(B) ~> K </k>
rule <k> and(#false(), Any) => #false() ~> K </k>
rule <k> block() => .K ~> K </k>
`

// rule 
// 	<k> X:id => I ~> K </k> 
// 	<state> S X |-> Y </state>

// rule
// 	<k> div(I1:int, I2:int) => #divInt(I1, I2) ~> K </k>
// 	when #notEqInt(I2, I2)

// rule <k> plus(I1:int, I2:int) => #plusInt(I1, I2) ~> K </k>
// rule <k> lte(I1:int, I2:int) => #lteInt(I1, I2) ~> K </k>
// rule <k> not(B:bool) => #not(B) ~> K </k>
// rule <k> and(#true(), B:bool) => B ~> K </k>

func ParseReader(r io.Reader) (*Language, error) {
	l := NewLexer(r)
	KincInit()
	ret := yyParse(l)

	if (ret == 0) {
		return Final, nil
	} else {
		return Final, errors.New("Kinc: error parsing string")
	}
}

func ParseString(s string) (*Language, error) {
	return ParseReader(strings.NewReader(s))	
}

func ParseFile(fname string) (*Language, error) {
	f, err := os.Open(fname)
	if (err != nil) {
		return nil, err
	}
	return ParseReader(f)
}

func main() {
	// fmt.Printf(prog)
	// l := NewLexer(strings.NewReader(prog))
	// KincInit()
	// ret := yyParse(l)
	// Final.String()

	// lang, err := ParseFile("../imp/imp.kinc")
	lang, err := ParseFile("../peano/peano.kinc")
	// lang, err := ParseString(prog)
	if err != nil {
		fmt.Printf("Couldn't parse language: %s\n", err)
		os.Exit(1)
	}

	for i, rule := range lang.Rules {
		rule.CompleteVariableTypes()

		fmt.Printf("\nrule: %s", rule.String())
		ch := rule.BuildChecks()
		fmt.Printf(ch.String())
		c := RuleToC(ch, rule, i)
		fmt.Printf("Compilation:\n")
		fmt.Printf(c)
	}

	// s := lang.PrettyPrint()
	// s := lang.String()
	// fmt.Printf("%s\n", s)

	os.Exit(0)
}

func RuleToC(ch *CheckHelper, r *Rule, i int) string {
	c := checksToC(ch)
	return fmt.Sprintf("/*\n%s\n*/\nint rule%d(Configuration* config) {\n%s\n}\n", r.String(), i, c)
}




/*
rule <k> plus(I1:int, I2:int) => #plusInt(I1, I2) ~> K </k>
	look at k cell
	get first argument of k cell k.0
	k.0 is appl
	check label(k.0) = 'plus
	check label(k.0.0) = 'int
	check label(k.0.1) = 'int
	---
	replace k.0 with #plusInt(k.0.0, k.0.1)


rule 
	<k> X:id => I ~> K </k> 
	<state> X |-> Y </state>

	look at k cell
	check label(k.0) = id
	X === k.0
	find X in state cell
	replace k.0 with state[X]


rule <k> and(#false(), Any) => #false() ~> K </k>
	look at k cell
	check label(k.0) = and
	check k.0.0 == #false
	Any === k.0.1
	---
	replace k.0 with #false

*/