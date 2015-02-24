package main

import "fmt"
// import "strings"

func (l *Language) PrettyPrint() string {
	pp := new(prettyPrinter)
	Walk(pp, l)
	return pp.s
}

func (pp *prettyPrinter) Visit(node Node) Visitor {
	switch n := node.(type) {
	case *Variable:
		pp.s += fmt.Sprintf("%s:%s", n.Name, n.Sort)
	}
	return pp
}

type prettyPrinter struct {
	s string
}


