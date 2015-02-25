package main

import "fmt"
import "log"
// import "errors"
// import "strings"

func go_sucks_constraints() {
	_ = fmt.Printf
	_ = log.Ldate
}

type Check struct {
	
}

type CheckHelper struct {
	checks []Check
	// Parent Node
	ref Reference
}

func (n *Rule) BuildChecks() {
	// vis := new(getChecks)
	ch := &CheckHelper{}
	// Walk(vis, l)
	// ch.Parent = n

	for _, bi := range n.Bag {
		bi.BuildBagChecks(ch)
	}

	// for c := range vis.checks {
	// fmt.Printf("%v\n", ch)
	// }
}

type RefPart interface {
	refPart()
}
type RefPartCell struct {
	Name string
}

func (*RefPartCell) refPart() {}


type Reference struct {
	Ref []RefPart
}

func (n ComputationCell) BuildBagChecks(ch *CheckHelper) {
	// fmt.Printf("Building checks for comp cell %s\n", n)
	_ = n.Name
	fmt.Printf("%s\n", n.String())
	ch.ref.Ref = append(ch.ref.Ref, &RefPartCell{n.Name})
	n.Computation.BuildTopKChecks(ch)
	// fmt.Printf("Building checks for comp cell %s\n", n)
}
func (n MapCell) BuildBagChecks(ch *CheckHelper) {
	fmt.Printf("Building checks for map cell %s\n", n)
}

func (n BagCell) BuildBagChecks(ch *CheckHelper) {
	panic("Don't handle Bag Cells yet")
}
func (n *Variable) BuildBagChecks(ch *CheckHelper) {
	panic("Don't handle Bag Variables yet")
}

func (n *Kra) BuildTopKChecks(ch *CheckHelper) {
	panic("Don't handle BuildTopKChecks Kra yet")
}
func (n *Variable) BuildTopKChecks(ch *CheckHelper) {
	panic("Don't handle BuildTopKChecks Variable yet")
}
func (n *Rewrite) BuildTopKChecks(ch *CheckHelper) {
	panic("Don't handle BuildTopKChecks Rewrite yet")
}
func (n *Appl) BuildTopKChecks(ch *CheckHelper) {
	panic("Don't handle BuildTopKChecks Appl yet")
}
func (n *Paren) BuildTopKChecks(ch *CheckHelper) {
	panic("Don't handle BuildTopKChecks Paren yet")
}

// func (vis *getChecks) VisitPre(node Node) Visitor {
// 	// fmt.Printf("Visiting %s\n", node)
// 	switch n := node.(type) {
// 	case *Variable:
// 		// fmt.Printf("Handling %s\n", n)
// 		if n.Default {
// 			vis.implicitVariables = append(vis.implicitVariables, n)
// 		} else {
// 			vis.explicitVariables = append(vis.explicitVariables, n)
// 		}
// 		// pp.s += fmt.Sprintf("%s:%s", n.Name, n.Sort)
// 	}
// 	return vis
// }

// func (vis *getChecks) VisitPost(node Node) { }

// type getChecks struct {
// 	checks []Check
// 	err error
// }
