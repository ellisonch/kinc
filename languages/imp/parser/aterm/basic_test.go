package aterm

import "testing"
import "strings"
import "fmt"

func go_sucks_basic_test() {
	_ = fmt.Printf
}

// Examples: "foobar", "string with quotes\"", "escaped escape character\\ and a newline\n".
// Lt(Var("n"),Int("1")){Type("bool")}

var simpleProg1 string = `"Plus"("Int"("4"), "Call"("f", ["Mul"("Int"("5"), "Var"("x"))]))`
var simpleProg2 string = `Plus(Int("4"), Call("f", [Mul(Int("5"), Var("x"))]))`

type Tokens []int

func TestBasic1(t *testing.T) {
	answer := Tokens{TOK_STRING, '(', TOK_STRING, '(', TOK_STRING, ')', ',', TOK_STRING, '(', TOK_STRING, ',', '[', TOK_STRING, '(', TOK_STRING, '(', TOK_STRING, ')', ',', TOK_STRING, '(', TOK_STRING, ')', ')', ']', ')', ')', TOK_EOF}
	l := NewATermLexer(strings.NewReader(simpleProg1))
	
	tokens := l.TokenList()

	if !answer.Equals(tokens) { t.FailNow() }
}
func TestBasic2(t *testing.T) {
	answer := Tokens{TOK_CONSTRUCTOR, '(', TOK_CONSTRUCTOR, '(', TOK_STRING, ')', ',', TOK_CONSTRUCTOR, '(', TOK_STRING, ',', '[', TOK_CONSTRUCTOR, '(', TOK_CONSTRUCTOR, '(', TOK_STRING, ')', ',', TOK_CONSTRUCTOR, '(', TOK_STRING, ')', ')', ']', ')', ')', TOK_EOF}
	l := NewATermLexer(strings.NewReader(simpleProg2))

	tokens := l.TokenList()
	
	if !answer.Equals(tokens) { t.FailNow() }
}

func (s1 Tokens) Equals(s2 Tokens) bool {
	if len(s1) != len(s2) {
		return false
	}
	for i, ans := range s1 {
		if s2[i] != ans {
			return false
		}
	}
	return true
}

// func BenchmarkHello(b *testing.B) {
//     for i := 0; i < b.N; i++ {
//         fmt.Sprintf("hello")
//     }
// }