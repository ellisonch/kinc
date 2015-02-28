package main

import "fmt"
import "log"
// import "errors"
// import "strings"

func go_sucks_constraints() {
	_ = fmt.Printf
	_ = log.Ldate
}


type CheckHelper struct {
	checks []Check
	replacements []Replacement
	bindings []Binding
	when *When
	// Parent Node
	ref Reference
}

func (ch *CheckHelper) String() string {
	s := ""
	s += fmt.Sprintf("Checks:\n")
	for _, checks := range ch.checks {
		s += fmt.Sprintf("  %s", checks.String())
	}
	s += fmt.Sprintf("Replacements:\n")
	for _, replacement := range ch.replacements {
		s += fmt.Sprintf("  %s", replacement.String())
	}
	s += fmt.Sprintf("Bindings:\n")
	for _, binding := range ch.bindings {
		s += fmt.Sprintf("  %s", binding.String())
	}

	if (ch.when == nil) {
		s += fmt.Sprintf("When:\n  No when clause\n")
	} else {
		s += fmt.Sprintf("When:\n  checks on %s\n", ch.when.String())
	}

	return s
}

func (ch *CheckHelper) AddCheck(v Check) {
	ch.checks = append(ch.checks, v)
}
func (ch *CheckHelper) AddReplacement(v Replacement) {
	ch.replacements = append(ch.replacements, v)
}
func (ch *CheckHelper) AddBinding(v Binding) {
	ch.bindings = append(ch.bindings, v)
}

func (n *Rule) BuildChecks() *CheckHelper {
	// vis := new(getChecks)
	ch := &CheckHelper{}
	// Walk(vis, l)
	// ch.Parent = n

	for _, bi := range n.Bag {
		bi.BuildBagChecks(ch)
	}

	ch.when = n.When
	// n.When.BuildChecks()

	return ch
	// for c := range vis.checks {
	// fmt.Printf("%v\n", ch)
	// }
}

// func (n *When) BuildChecks() {
// 	if (n == nil) {
// 		fmt.Printf("No when clause\n")
// 	} else {
// 		fmt.Printf("checks on %s\n", n.String())
// 	}
// }


func (n ComputationCell) BuildBagChecks(ch *CheckHelper) {
	// fmt.Printf("Building checks for comp cell %s\n", n)
	// _ = n.Name
	// fmt.Printf("%s\n", n.String())
	ch.ref.addCellEntry(n.Name)
	// ch.ref.Ref = append(ch.ref.Ref, &RefPartCell{n.Name})
	n.Computation.BuildTopKChecks(ch)
	// fmt.Printf("Building checks for comp cell %s\n", n)
}
func (n MapCell) BuildBagChecks(ch *CheckHelper) {
	// fmt.Printf("Building checks for map cell %s\n", n)
	ch.ref.addCellEntry(n.Name)
	// _ = n.Name
	for _, m := range n.Map {
		m.BuildTopMapItemChecks(ch)
	}
}

func (n *Variable) BuildTopMapItemChecks(ch *CheckHelper) {
	// fmt.Printf("Building checks for map item %s\n", n)
	// FIXME: should be binding and checking all this
}
func (n *Mapping) BuildTopMapItemChecks(ch *CheckHelper) {
	fmt.Printf("Building checks for map item %s\n", n)
}
func (n *DotMap) BuildTopMapItemChecks(ch *CheckHelper) {
	fmt.Printf("Building checks for map item %s\n", n)
}
func (n *RewriteMapItem) BuildTopMapItemChecks(ch *CheckHelper) {
	// ref := ch.ref
	// ref.addMapLookup(n.LHS)

	// ref := n.LHS.GetReference(ch)

	switch l := n.LHS.(type) {
	case *Variable:
		_ = l
		// rep := Replacement{Loc: ref, Result: n.RHS}
		// ch.AddReplacement(rep)
		panic("Don't handle variable for GetReference")
	case *Mapping: 
		panic("Don't handle Mapping for GetReference")
	case* DotMap:
		switch r := n.RHS.(type) {
		case *Mapping:
			change := &MapAdd{Loc: ch.ref, Entry: r}	
			ch.AddReplacement(change)
		default: 
			panic(fmt.Sprintf("Don't handle rewriting to %s in map", r.String()))
		}
	case* RewriteMapItem: 
		panic("Don't handle RewriteMapItem for GetReference")
	}

	
	// fmt.Printf("Building checks for map item %s\n", n)
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

// FIXME: probably should do something like start at beginning and describe "find a term matching foo, then all the terms between that and a term matching bar are bound to K"
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
			// fmt.Printf("Ok, variable\n")

			n.Children = n.Children[:lasti]
		}
	}
	_ = allowMore

	check := &CheckNumCellArgs{Loc: ch.ref, Num: len(n.Children)}
	if (allowMore) {
		check.Exact = false
	} else {
		check.Exact = true
	}
	ch.AddCheck(check)
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
	// fmt.Printf("bind %s to %s\n", n.String(), ref.String())
	binding := Binding{Loc: ref, Variable: n}
	ch.AddBinding(binding)
	// panic("Don't handle BuildKChecks Variable yet")
}
func (n *Rewrite) BuildKChecks(ch *CheckHelper, ref Reference, i int) {
	n.LHS.BuildKChecks(ch, ref, i)

	ref.addPositionEntry(i)
	rep := &TermChange{Loc: ref, Result: n.RHS}
	ch.AddReplacement(rep)
	// fmt.Printf("%s should be replaced with %s\n", ref.String(), n.RHS.String())

	// panic("Don't handle BuildKChecks Rewrite yet")
}
func (n *Appl) BuildKChecks(ch *CheckHelper, ref Reference, i int) {
	ref.addPositionEntry(i)
	// fmt.Printf("%s must have the '%s label\n", ref.String(), n.Label.String())
	checkLabel := &CheckLabel{Loc: ref, Label: n.Label}
	ch.AddCheck(checkLabel)
	checkArgs := &CheckNumArgs{Num: len(n.Body), Loc: ref}
	ch.AddCheck(checkArgs)
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
