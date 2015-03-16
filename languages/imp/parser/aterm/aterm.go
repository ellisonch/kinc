package aterm

import "fmt"
import "strings"

type ATermType int
const (
	Error = iota
	String
	Int
	Real
	Appl
	List
)

type ATerm struct {
	Type ATermType
	Str string
	Int int64
	Real float64
	Appl ATermAppl
	List ATermList
}

type ATermAppl struct {
	Name string
	Args CommaList
}
type ATermList CommaList
type CommaList []ATerm

func (at *ATerm) String() string {
	switch at.Type {
		case Error: return "---Error---"
		case String: return fmt.Sprintf("\"%s\"", at.Str)
		case Int: return fmt.Sprintf("%d", at.Int)
		case Real: return fmt.Sprintf("%f", at.Real)
		case Appl: return fmt.Sprintf("%s(%s)", at.Appl.Name, at.Appl.Args.String())
		case List: return fmt.Sprintf("[%s]", CommaList(at.List).String())
	}
	return "---Error---"
}
func (l CommaList) String() string {
	ss := []string{}
	for _, at := range l {
		ss = append(ss, at.String())
	}
	return strings.Join(ss, ",")
}