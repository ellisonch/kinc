package main

import "fmt"
// import "log"
// import "errors"
// import "strings"

func go_sucks_subsorts() {
	_ = fmt.Printf
}

func (l *Language) CompleteSubsorts() map[string][]Label {
	vis := NewSubsorts()
	// fmt.Printf("Starting walk...")
	Walk(vis, l)
	return vis.Lookup
}

func NewSubsorts() *SubsortsMap {
	r := new(SubsortsMap)
	r.Lookup = make(map[string][]Label)
	return r
}

type SubsortsMap struct {
	Lookup map[string][]Label
}

func (vis *SubsortsMap) VisitPre(node Node) Visitor {
	// fmt.Printf("Visiting %s\n", node)
	switch n := node.(type) {
	case *Subsort:
		var list []Label
		if a, ok := vis.Lookup[n.Sort]; ok {
			list = a
		} else {
			list = []Label{}
		}
		vis.Lookup[n.Sort] = append(list, &NameLabel{n.Subsort})
	}
	return vis
}

func (vis *SubsortsMap) VisitPost(node Node) { }
