package main

import "fmt"
// import "strings"

func (l *Language) PrettyPrint() string {
	pp := new(prettyPrinter)
	Walk(pp, l)
	return pp.s
}

func (pp *prettyPrinter) VisitPre(node Node) Visitor {
	switch n := node.(type) {
	case *Variable:
		pp.s += fmt.Sprintf("%s:%s", n.Name, n.Sort)
	}
	return pp
}

func (pp *prettyPrinter) VisitPost(node Node) {
	// switch n := node.(type) {
	// case *Variable:
	// 	pp.s += fmt.Sprintf("%s:%s", n.Name, n.Sort)
	// }
	return
}

type prettyPrinter struct {
	s string
	children []string
}


