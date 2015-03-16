package main
import "fmt"
import "strings"
// import "strconv"
// import "time"
// import "runtime/pprof"
import "os"
// import "github.com/ellisonch/aterm"
import "./aterm"

// TODO/Notes
// need to add static pass
// for most rules, we know what the next rule to take place will be

// need to deal with terms inside the store

// type C struct {
// 	// symbolCount int
// 	// symbolMap map[string]int
// 	term string
// }

// var c C

// var symbolMap map[string]int = map[string]int {
//     "Assign": 0,
//     "Div": 1,
// }

// #define symbol_assign 0
// #define symbol_div 1
// #define symbol_id 2
// #define symbol_if 3
// #define symbol_lte 4
// #define symbol_neg 5
// #define symbol_not 6
// #define symbol_plus 7
// #define symbol_program 8
// #define symbol_skip 9
// #define symbol_statements 10
// #define symbol_var 11
// #define symbol_while 12

func ATermListToStringList(l []aterm.ATerm) []string {
	arguments := make([]string, len(l))
	for i, t := range l {
		arguments[i] = ATermToC(&t)
	}
	// res := strings.Join(arguments, ",")
	// fmt.Printf("%s\n", res)
	// if strings.HasSuffix(res, " ") {
	// 	panic(fmt.Sprintf("Something up with %v", l))
	// }
	return arguments
}
func ATermToC(t *aterm.ATerm) string {
	switch t.Type {
		case aterm.Error:
			panic("Didn't expect error term in ATermToC()")
		case aterm.String:
			return fmt.Sprintf("new_builtin_string(\"%s\")", t.Str)
		case aterm.Int:
			// return NewK(Int64Label(t.Int), nil)
			return fmt.Sprintf("new_builtin_int(%d)", t.Int)
		case aterm.Appl:
			if (t.Appl.Name == "#Int" || t.Appl.Name == "#String") {
				return ATermToC(&t.Appl.Args[0])
			}
			if (t.Appl.Name == "#Hole") {
				// return ATermToC(&t.Appl.Args[0])
				nodolla := strings.TrimLeft(t.Appl.Args[0].Str, "$")
				return "hole_" + nodolla
				// strings.TrimLeft(t.Appl.Args[0], cutset)
			}
			// return NewK(StringLabel(t.Appl.Name), ATermListToListK(t.Appl.Args))
			// symbol, ok := symbolMap[t.Appl.Name]
			// if !ok {
			// 	panic(fmt.Sprintf("Couldn't find symbol %s", t.Appl.Name))
			// }
			symbol_name := "symbol_" + t.Appl.Name
			args := ATermListToStringList(t.Appl.Args)
			var sargs string
			if len(args) == 0 {
				sargs = "NULL"
			} else {
				sargs = fmt.Sprintf("newArgs(%d, %s)", len(args), strings.Join(args, ","))
			}
			return fmt.Sprintf("NewK(SymbolLabel(%s), %s)", symbol_name, sargs)
		// case aterm.List: 
		// 	return NewK(StringLabel("KList"), ATermListToListK(t.List))
	}
	panic(fmt.Sprintf("Not handling default for %v", t))
}

func getProgram() string {
	// x := []string{"1", "2", "3"}
	// fmt.Printf("%s\n", strings.Join(x, ","))

	l := aterm.NewATermLexer(os.Stdin)
	ret := aterm.ATermParse(l)
	if ret != 0 { 
		panic("Couldn't parse ATerm!")
		// fmt.Printf("%s\n\n", aterm.FinalTerm.String())		
	}
	// c.symbolMap = make(map[string]int)
	newk := ATermToC(&aterm.FinalTerm)
	// if newk != nil {
	// 	fmt.Printf("%s\n", newk.String())
	// } else {
	// 	fmt.Printf("nil")
	// }
	return newk
}

func main() {
    p1 := getProgram()
    fmt.Printf("%s\n", p1)

}
