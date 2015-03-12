package main

import "fmt"
// import "log"
// import "errors"
import "strings"

func go_sucks_symbol_labels() {
	_ = fmt.Printf
}

func (l *Language) CompleteLabelSymbols() map[string]int {
	vis := NewSymbolLabels()
	// fmt.Printf("Starting walk...")
	Walk(vis, l)
	return vis.lookup
}

func NewSymbolLabels() *SymbolLabels {
	r := new(SymbolLabels)
	r.lookup = make(map[string]int)
	return r
}

type SymbolLabels struct {
	next int
	lookup map[string]int
}

func (vis *SymbolLabels) VisitPre(node Node) Visitor {
	// fmt.Printf("Visiting %s\n", node)
	switch n := node.(type) {
	case *NameLabel:
		name := n.Name
		// fmt.Printf("Looking at %s\n", name)
		if strings.HasPrefix(name, "#") {
			// fmt.Printf("Builtin\n")
			return vis
		}
		if _, ok := vis.lookup[name]; ok {
			break
		}
		vis.lookup[name] = vis.next
		vis.next += 1
	}
	return vis
}

func (vis *SymbolLabels) VisitPost(node Node) { }
