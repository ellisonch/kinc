package main

import "testing"
import "strings"
import "fmt"
import "os"
// import "os/signal"
// import "syscall"

func go_sucks_compiler_test() {
	_ = fmt.Printf
	_ = os.DevNull
	_ = testing.Verbose()
	_ = strings.Contains
}

// var compiler_tests = []struct {
// 	inp string
// }{
// 	{`configuration <a> </a>`},
// 	// {`configuration <a> <b> <c> </c> </b> </a>`},
// 	// {`configuration <a> <b> </b> <c> <d> </d> </c> </a>`},
// 	// {`configuration <k> </k> rule <k> X => Y </k> <mem> Z </mem> `},
// 	// {`configuration <k> </k> rule <k> foo() </k> `},
// 	// {`configuration <k> </k> rule <k> foo(X) </k> `},
// 	// {`configuration <k> </k> rule <k> (x => q)(Z, Y) </k>  `},
// 	// {`configuration <k> </k> rule <k> X:id => I </k>`},
// 	// {`configuration <a> </a> rule <k> X => Y ~> Z </k>`},
// 	// {`configuration <a type="k"> </a>`},
// 	// {`configuration <a type="k"> </a> rule <a> V </a>`},
// 	// {`configuration <a type="bag"> </a> rule <a> V </a>`},
// 	// {`configuration <a> </a> rule <a> V </a> <b> V </b>`},
// 	// {`configuration <a type="map"> </a> rule <a> V </a>`},
// 	// {`configuration <a type="map"> </a> rule <a> X |-> Y </a>`},
// 	// {`configuration <a type="map"> </a> rule <a> Z X |-> Y </a>`},
// 	// {`configuration <a type="map"> </a> rule <a> Z X |-> Y </a>`},
// 	// {`configuration <foo-bar> </foo-bar>`},
// 	// {`configuration <a> </a> rule <a> plus(I1, I2) => #_plusInt_(I1, I2) </a>`},
// 	// {`configuration <a> </a> rule <a> div(I1, I2) => #_divInt_(I1, I2) </a> when #_notEqInt_(I2, I2)`},
// };

// func TestCompiler(t *testing.T) {
// 	yyDebug = 1
// 	for _, prog := range compiler_tests {
// 		l := NewLexer(strings.NewReader(prog.inp))
// 		KincInit()

// 		ret := yyParse(l)

// 		if ret == 0 {
// 			t.Logf("\n%s\n", Final.String())
// 		} else {
// 			t.Logf("%s\n", "Error")
// 			t.FailNow()
// 		}
// 	}
// }
