package main

import "fmt"
// import "log"
// import "errors"
// import "strings"

func go_sucks_subsorts() {
	_ = fmt.Printf
}

func (l *Language) CompleteSubsorts() map[string][]string {
	vis := NewSubsorts()
	// fmt.Printf("Starting walk...")
	Walk(vis, l)
	return vis.Lookup
}

func NewSubsorts() *SubsortsMap {
	r := new(SubsortsMap)
	r.Lookup = make(map[string][]string)
	return r
}

type SubsortsMap struct {
	Lookup map[string][]string
}

func (vis *SubsortsMap) VisitPre(node Node) Visitor {
	// fmt.Printf("Visiting %s\n", node)
	switch n := node.(type) {
	case *Subsort:
		var list []string
		if a, ok := vis.Lookup[n.Sort]; ok {
			list = a
		} else {
			list = []string{}
		}
		vis.Lookup[n.Sort] = append(list, n.Subsort)
	}
	return vis
}

func (vis *SubsortsMap) VisitPost(node Node) { }
