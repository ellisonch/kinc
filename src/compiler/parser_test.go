package main

import "testing"
import "strings"
import "fmt"
import "os"
// import "os/signal"
// import "syscall"

func go_sucks_parser_test() {
	_ = fmt.Printf
	_ = os.DevNull
}

var tests = []struct {
	inp string
}{
	{`configuration <a> </a>`},
	{`configuration <a> <b> <c> </c> </b> </a>`},
	{`configuration <a> <b> </b> <c> <d> </d> </c> </a>`},
	{`configuration <k> </k> rule <k> X => Y </k> <mem> Z </mem> `},
	{`configuration <k> </k> rule <k> foo() </k> `},
	{`configuration <k> </k> rule <k> foo(X) </k> `},
	{`configuration <k> </k> rule <k> (x => q)(Z, Y) </k>  `},
	{`configuration <k> </k> rule <k> X:id => I </k>`},
	{`configuration <a> </a> rule <k> X => Y ~> Z </k>`},
	{`configuration <a type="k"> </a>`},
	{`configuration <a type="k"> </a> rule <a> V </a>`},
	{`configuration <a type="bag"> </a> rule <a> V </a>`},
	{`configuration <a> </a> rule <a> V </a> <b> V </b>`},
	{`configuration <a type="map"> </a> rule <a> V </a>`},
	{`configuration <a type="map"> </a> rule <a> (X |-> Y) </a>`},
	{`configuration <a type="map"> </a> rule <a> Z (X |-> Y) </a>`},
	{`configuration <a type="map"> </a> rule <a> Z (X |-> Y) </a>`},
	{`configuration <foo-bar> </foo-bar>`},
	{`configuration <a> </a> rule <a> plus(I1:int, I2) => #_plusInt_(I1, I2) </a>`},
	{`configuration <a> </a> rule <a> div(I1, I2) => #_divInt_(I1, I2) </a> when #_notEqInt_(I2, I2)`},
	{`configuration <a type="k"> </a> rule <a> A ~> B ~> C => D ~> E</a>`},
	{`configuration <a type="k"> </a> rule <a> #inttok{0}() => .k ~> K </a>`},
};

// var simpleProg3 string = `Plus(Int("4"), Call("f", [Mul(Int(5), Var("x"))]))`
// // var simpleProg4 string = `Plus(Int(4), Call("f", [Mul(Int(5), Var("x"))]))`
// var simpleProg4 string = `Assign(Id("n"), Plus(Id("n"), Int("-1")))`

// var prog0 string = `Program(Assign(Id("n"), Int(100)))`;

// var prog1 string = `Program(Var(Id("n"),Id("s"),Id("r")),Assign(Id("n"), Int(100)),Assign(Id("s"), Int(0)),While(Not(Paren(LTE(Id("n"), Int(0)))), Assign(Id("s"), Plus(Id("s"), Id("n"))),Assign(Id("n"), Plus(Id("n"), Neg(Int(1))))))`
// var prog2 string = `Program(Var(Id("m"),Id("n"),Id("q"),Id("r"),Id("s")),Assign(Id("m"), Int(10)),While(Paren(Not(Paren(LTE(Id("m"), Int(2))))), Assign(Id("n"), Id("m")),Assign(Id("m"), Plus(Id("m"), Neg(Int(1)))),While(Paren(Not(Paren(LTE(Id("n"), Int(1))))), Assign(Id("s"), Plus(Id("s"), Int(1))),Assign(Id("q"), Div(Id("n"), Int(2))),Assign(Id("r"), Plus(Plus(Id("q"), Id("q")), Int(1))),While(Paren(LTE(Id("r"), Id("n"))), Assign(Id("n"), Plus(Plus(Plus(Id("n"), Id("n")), Id("n")), Int(1))), Assign(Id("n"), Id("q"))))))`
// var prog3 string = `Program(Var(Id("i"),Id("m"),Id("n"),Id("q"),Id("r"),Id("s"),Id("t"),Id("x"),Id("y"),Id("z")),Assign(Id("m"), Int(10)),Assign(Id("n"), Int(2)),While(Paren(LTE(Id("n"), Id("m"))), Assign(Id("i"), Int(2)),Assign(Id("q"), Div(Id("n"), Id("i"))),Assign(Id("t"), Int(1)),While(Paren(And(LTE(Id("i"), Id("q")), LTE(Int(1), Id("t")))), Assign(Id("x"), Id("i")),Assign(Id("y"), Id("q")),Assign(Id("z"), Int(0)),While(Paren(Not(Paren(LTE(Id("x"), Int(0))))), Assign(Id("q"), Div(Id("x"), Int(2))),Assign(Id("r"), Plus(Plus(Id("q"), Id("q")), Int(1))),While(Paren(LTE(Id("r"), Id("x"))), Assign(Id("z"), Plus(Id("z"), Id("y"))), Skip()),Assign(Id("x"), Id("q")),Assign(Id("y"), Plus(Id("y"), Id("y")))),While(Paren(LTE(Id("n"), Id("z"))), Assign(Id("t"), Int(0)), Assign(Id("i"), Plus(Id("i"), Int(1))),Assign(Id("q"), Div(Id("n"), Id("i"))))),While(Paren(LTE(Int(1), Id("t"))), Assign(Id("s"), Plus(Id("s"), Int(1))), Skip()),Assign(Id("n"), Plus(Id("n"), Int(1)))))`

func PrintTokens(s string) {
	l := NewLexer(strings.NewReader(s))
	
	var lval yySymType

	for {
		char := l.Lex(&lval)
		if char == 0 || char == yyEofCode || char == yyErrCode {
			break
		}
		// name := yyTokname(char)
		// var name string
		offset := char - yyPrivate - 2
		var name string
		if offset >= 0 && offset < len(yyToknames) {
			name = yyToknames[offset]
		} else {
			name = fmt.Sprintf("%c", char)
		}
		fmt.Printf("%s ", name)
	}
	fmt.Printf("\n")
}

func TestParser(t *testing.T) {
	yyDebug = 1
	for _, prog := range tests {
		l := NewLexer(strings.NewReader(prog.inp))
		KincInit()

		ret := yyParse(l)

		if ret == 0 {
			t.Logf("\n%s\n", Final.String())
		} else {
			t.Logf("%s\n", "Error")
			t.FailNow()
		}
	}
}

func TestImp(t *testing.T) {
	yyDebug = 1

	imp, err := os.Open("../imp/imp.kinc")
	if (err != nil) {
		t.Logf("Couldn't open file")
		t.FailNow()
	}

	l := NewLexer(imp)
	ret := yyParse(l)

	if ret == 0 {
		t.Logf("\n%s\n", Final.String())
	} else {
		t.Logf("%s\n", "Error")
		t.FailNow()
	}
}

// func TestParser2(t *testing.T) {
// 	ATermDebug = 1
// 	l := NewATermLexer(strings.NewReader(simpleProg4))
// 	ret := ATermParse(l)

// 	if ret == 0 { 
// 		t.Logf("%s\n", FinalTerm.String())		
// 	} else { 
// 		t.FailNow()
// 	}
// }
// func TestProg0(t *testing.T) {
// 	ATermDebug = 1
// 	l := NewATermLexer(strings.NewReader(prog0))
// 	ret := ATermParse(l)
// 	if ret == 0 { 
// 		t.Logf("%s\n", FinalTerm.String())		
// 	} else { 
// 		t.FailNow()
// 	}
// }
// func TestProg1(t *testing.T) {
// 	ATermDebug = 1
// 	l := NewATermLexer(strings.NewReader(prog1))
// 	ret := ATermParse(l)
// 	if ret == 0 { 
// 		t.Logf("%s\n", FinalTerm.String())		
// 	} else { 
// 		t.FailNow()
// 	}
// }
// func TestProg2(t *testing.T) {
// 	ATermDebug = 1
// 	l := NewATermLexer(strings.NewReader(prog2))
// 	ret := ATermParse(l)
// 	if ret == 0 { 
// 		t.Logf("%s\n", FinalTerm.String())		
// 	} else { 
// 		t.FailNow()
// 	}
// }
// func TestProg3(t *testing.T) {
// 	ATermDebug = 1
// 	l := NewATermLexer(strings.NewReader(prog3))
// 	ret := ATermParse(l)
// 	if ret == 0 { 
// 		t.Logf("%s\n", FinalTerm.String())		
// 	} else { 
// 		t.FailNow()
// 	}
// }

// func BenchmarkHello(b *testing.B) {
//     for i := 0; i < b.N; i++ {
//         fmt.Sprintf("hello")
//     }
// }