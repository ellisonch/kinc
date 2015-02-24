package main

import "fmt"
import "os"
import "strings"
import "errors"

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
	<k> X:id => I ~> K </k> 
	<state> X |-> Y </state>

rule
	<k> div(I1:int, I2:int) => #divInt(I1, I2) ~> K </k>
	when #notEqInt(I2, I2)

rule <k> plus(I1:int, I2:int) => #plusInt(I1, I2) ~> K </k>
rule <k> lte(I1:int, I2:int) => #lteInt(I1, I2) ~> K </k>
rule <k> not(B:bool) => #not(B) ~> K </k>
rule <k> and(#true(), B:bool) => B ~> K </k>
rule <k> and(#false(), Any) => #false() ~> K </k>
`

func ParseString(s string) (Language, error) {
	l := NewLexer(strings.NewReader(s))
	KincInit()
	ret := yyParse(l)

	if (ret == 0) {
		return Final, nil
	} else {
		return Final, errors.New("Kinc: error parsing string")
	}
}

func main() {
	// fmt.Printf(prog)
	// l := NewLexer(strings.NewReader(prog))
	// KincInit()
	// ret := yyParse(l)
	// Final.String()

	lang, err := ParseString(prog)
	if err != nil {
		fmt.Printf("Couldn't parse language: %s\n", err)
		os.Exit(1)
	}

	s := lang.PrettyPrint()

	fmt.Printf("%s\n", s)
	os.Exit(0)
}
