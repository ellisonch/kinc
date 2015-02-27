package main

import "fmt"
import "log"
// import "errors"
import "strings"

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

	n.When.BuildChecks()

	// for c := range vis.checks {
	// fmt.Printf("%v\n", ch)
	// }
}

func (n *When) BuildChecks() {
	if (n == nil) {
		fmt.Printf("No when clause")
	} else {
		fmt.Printf("checks on %s\n", n.String())
	}
}

type RefPart interface {
	refPart()
	String() string
}
type RefPartCell struct {
	Name string
}
type RefPartPosition struct {
	Offset int
}

func (*RefPartCell) refPart() {}
func (*RefPartPosition) refPart() {}

func (r *RefPartCell) String() string {
	return r.Name
}
func (r *RefPartPosition) String() string {
	return fmt.Sprintf("%d", r.Offset)
}

type Reference struct {
	Ref []RefPart
}

func (r *Reference) String() string {
	children := []string{}
	for _, c := range r.Ref {
		children = append(children, c.String())
	}
	return strings.Join(children, ".")
}

func (r *Reference) addCellEntry(s string) {
	r.Ref = append(r.Ref, &RefPartCell{s})
}
func (r *Reference) addPositionEntry(n int) {
	r.Ref = append(r.Ref, &RefPartPosition{n})
}
// func (r *Reference) setPositionEntry(n int) {
// 	r.Ref[len(r.Ref)-1] = &RefPartPosition{n}
// }

func (n ComputationCell) BuildBagChecks(ch *CheckHelper) {
	// fmt.Printf("Building checks for comp cell %s\n", n)
	_ = n.Name
	fmt.Printf("%s\n", n.String())
	ch.ref.addCellEntry(n.Name)
	// ch.ref.Ref = append(ch.ref.Ref, &RefPartCell{n.Name})
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
//---------------------------------------------------------------
func (n *DotK) BuildTopKChecks(ch *CheckHelper) {
	panic("Don't handle BuildTopKChecks DotK yet")
}
func (n *Kra) BuildTopKChecks(ch *CheckHelper) {
	if len(n.Children) == 0 {
		panic("Didn't expect size 0 kra")
	}

	allowMore := false
	lasti := len(n.Children) - 1
	fmt.Printf("lasti: %d\n", lasti)
	if lasti > 0 {
		switch c := n.Children[lasti].(type) {
		case *Variable:
			_ = c
			allowMore = true
			fmt.Printf("Ok, variable\n")
			n.Children = n.Children[:lasti]
		}
	}
	_ = allowMore

	if (allowMore) {
		fmt.Printf("%s must have at least %d things in it\n", ch.ref.String(), len(n.Children))
	} else {
		fmt.Printf("%s must have exactly %d things in it\n", ch.ref.String(), len(n.Children))
	}
	// ch.ref.addPositionEntry(0)
	for i, v := range n.Children {
		// ch.ref.setPositionEntry(i)
		v.BuildKChecks(ch, ch.ref, i)
		// fmt.Printf("asdf\n")
		fmt.Printf("%s\n", v.String())
	}
	// panic("Don't handle BuildTopKChecks Kra yet")
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
//---------------------------------------------------------------
func (n *DotK) BuildKChecks(ch *CheckHelper, ref Reference, i int) {
	panic("Don't handle BuildKChecks DotK yet")
}
func (n *Kra) BuildKChecks(ch *CheckHelper, ref Reference, i int) {
	panic("Don't handle BuildKChecks Kra yet")
}
func (n *Variable) BuildKChecks(ch *CheckHelper, ref Reference, i int) {
	ref.addPositionEntry(i)
	fmt.Printf("bind %s to %s\n", n.String(), ref.String())
	// panic("Don't handle BuildKChecks Variable yet")
}
func (n *Rewrite) BuildKChecks(ch *CheckHelper, ref Reference, i int) {
	n.LHS.BuildKChecks(ch, ref, i)

	ref.addPositionEntry(i)
	fmt.Printf("%s should be replaced with %s\n", ref.String(), n.RHS.String())

	// panic("Don't handle BuildKChecks Rewrite yet")
}
func (n *Appl) BuildKChecks(ch *CheckHelper, ref Reference, i int) {
	ref.addPositionEntry(i)
	fmt.Printf("%s must have the '%s label\n", ref.String(), n.Label.String())
	fmt.Printf("%s must have %d arguments\n", ref.String(), len(n.Body))
	for i, c := range n.Body {
		c.BuildKChecks(ch, ref, i)
	}
	// panic("Don't handle BuildKChecks Appl yet")
}
func (n *Paren) BuildKChecks(ch *CheckHelper, ref Reference, i int) {
	panic("Don't handle BuildKChecks Paren yet")
}
//---------------------------------------------------------------
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
