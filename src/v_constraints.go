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
	checks Checks
	replacements []Replacement
	bindings []Binding
	when *When
	// Parent Node
	ref Reference
	// symbolMap map[string]int
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
	// n.Computation.BuildTopKChecks(ch)
	n.Computation.BuildKChecksTermListHelper(ch, ch.ref)
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
// func (n *DotK) BuildTopKChecks(ch *CheckHelper) {
// 	panic("Don't handle BuildTopKChecks DotK yet")
// }

// // FIXME: probably should do something like start at beginning and describe "find a term matching foo, then all the terms between that and a term matching bar are bound to K"
// func (n *TermList) BuildTopKChecks(ch *CheckHelper) {
// 	if len(n.Children) == 0 {
// 		panic("Didn't expect size 0 TermList")
// 	}

// 	allowMore := false
// 	lasti := len(n.Children) - 1
// 	// fmt.Printf("lasti: %d\n", lasti)
// 	if lasti > 0 {
// 		switch c := n.Children[lasti].(type) {
// 		case *TermListKItem: 
// 			switch c := c.Item.(type) {
// 			case *Variable:
// 				_ = c
// 				allowMore = true
// 				// fmt.Printf("Ok, variable\n")

// 				n.Children = n.Children[:lasti]
// 			}
// 		case *TermListRewrite: 
// 			panic("BuildTopKChecks() Don't handle TermListRewrite")
// 		}
		
// 	}
// 	_ = allowMore

// 	check := &CheckNumCellArgs{Loc: ch.ref, Num: len(n.Children)}
// 	if (allowMore) {
// 		check.Exact = false
// 	} else {
// 		check.Exact = true
// 	}
// 	ch.AddCheck(check)
// 	// ch.ref.addPositionEntry(0)
// 	for i, v := range n.Children {
// 		// switch c := v.(type) {
// 		// case *TermListKItem: 
// 		// 	c.Item.BuildKChecks(ch, ch.ref, i)
// 		// case *TermListRewrite:
// 		// 	panic("BuildTopKChecks() Don't handle TermListRewrite part 2")
// 		// }
// 		v.BuildKChecks(ch, ch.ref, i)
		
// 		// fmt.Printf("asdf\n")
// 		// fmt.Printf("%s\n", v.String())
// 	}
// 	// panic("Don't handle BuildTopKChecks Kra yet")
// }
// func (n *Variable) BuildTopKChecks(ch *CheckHelper) {
// 	panic("Don't handle BuildTopKChecks Variable yet")
// }
// func (n *TermListRewrite) BuildTopKChecks(ch *CheckHelper) {
// 	n.BuildKChecks(ch, ch.ref, 0)
// }
// func (n *Appl) BuildTopKChecks(ch *CheckHelper) {
// 	check := &CheckNumCellArgs{Loc: ch.ref, Num: 1, Exact: true}
// 	ch.AddCheck(check)
	
// 	n.BuildKChecks(ch, ch.ref, 0)
// }
// func (n *Paren) BuildTopKChecks(ch *CheckHelper) {
// 	panic("Don't handle BuildTopKChecks Paren yet")
// }
//---------------------------------------------------------------
func (n *DotK) BuildKChecks(ch *CheckHelper, ref Reference, offset Offset) bool {
	panic("Don't handle BuildKChecks DotK yet")
}
func (n *Kra) BuildKChecks(ch *CheckHelper, ref Reference, offset Offset) bool {
	panic("Don't handle BuildKChecks Kra yet")
}
func (n *Variable) BuildKChecks(ch *CheckHelper, ref Reference, offset Offset) bool {
	ref.addPositionOffsetEntry(offset)
	// fmt.Printf("bind %s to %s\n", n.String(), ref.String())
	if n.ActualSort == "listk" {
		binding := Binding{Loc: ref, Variable: n, EndList: true} // assumes list is at end
		ch.AddBinding(binding)
		return true
	} else if n.ActualSort != "k" {
		if subs, ok := _subsortMap[n.ActualSort]; ok {
			ck := &CheckSort{Loc: ref, Allowable: subs}
			ch.AddCheck(ck)
		} else {
			ck := &CheckSort{Loc: ref, Allowable: []string{n.ActualSort}}
			ch.AddCheck(ck)
		}
	}
	binding := Binding{Loc: ref, Variable: n}
	ch.AddBinding(binding)
	// panic("Don't handle BuildKChecks Variable yet")
	return false
}
func (n *Appl) BuildKChecks(ch *CheckHelper, ref Reference, offset Offset) bool {
	ref.addPositionOffsetEntry(offset)
	n.Label.BuildKChecks(ch, ref)
	n.Body.BuildKChecksTermListHelper(ch, ref)
	return false
}
func (n *Paren) BuildKChecks(ch *CheckHelper, ref Reference, offset Offset) bool {
	panic("Don't handle BuildKChecks Paren yet")
}
// func (n *TermListRewrite) BuildKChecks(ch *CheckHelper, ref Reference, offset Offset) bool {
// 	panic("TermListRewrite.BuildKChecks(): Shouldn't get here")
// }
// func (n *TermListKItem) BuildKChecks(ch *CheckHelper, ref Reference, offset Offset) bool {
// 	panic("TermListKItem.BuildKChecks(): Shouldn't get here")
// }
//---------------------------------------------------------------
func (n *NameLabel) BuildKChecks(ch *CheckHelper, ref Reference) {
	checkLabel := &CheckLabel{Loc: ref, Label: n}
	// fmt.Printf("adding NameLabel %s", checkLabel.String())
	ch.AddCheck(checkLabel)
}
func (n *InjectLabel) BuildKChecks(ch *CheckHelper, ref Reference) {
	panic("Don't handle BuildKChecks InjectLabel yet")
}
func (n *RewriteLabel) BuildKChecks(ch *CheckHelper, ref Reference) {
	checkLabel := &CheckLabel{Loc: ref, Label: n.LHS}
	// fmt.Printf("adding RewriteLabel %s", checkLabel.String())
	ch.AddCheck(checkLabel)
	rep := &LabelChange{Loc: ref, Result: n.RHS}
	ch.AddReplacement(rep)
}

//---------------------------------------------------------------

// // returns true if was a list
// func (c *TermListKItem) BuildChecksInList(ch *CheckHelper, ref Reference, offset Offset) (isList bool) {
// 	isList = false

// 	switch c := c.Item.(type) {
// 	case *Variable:
// 		if c.ActualSort == "listk" {
// 			isList = true
			
// 			newref := ref
// 			newref.addPositionOffsetEntry(offset)
// 			binding := Binding{Loc: newref, Variable: c, EndList: true}
// 			ch.AddBinding(binding)
// 		} else {
// 			c.BuildKChecks(ch, ref, offset)
// 		}
// 	default:
// 		c.BuildKChecks(ch, ref, offset)
// 	}

// 	return
// }

// type TLChild struct {
// 	lhsOffset Offset
// 	rhsOffset Offset
// 	isList bool
// }

// func (c *TermListKItem) BuildCheckTermListItem(ch *CheckHelper, ref Reference, lhsOffset Offset, rhsOffset Offset) TLChild {
// 	// panic("need to update lhs and rhs")
// 	isList := c.BuildChecksInList(ch, ref, lhsOffset)
// 	// fmt.Printf("Checking item %s\n", c)
// 	// fmt.Printf("Offset is %s\n", lhsOffset)
// 	if !isList {
// 		// fmt.Printf("Not a list\n")
// 		lhsOffset.AddOne()
// 	} else {
// 		// FIXME: should update offset here
// 		// lhsOffset.Add
// 		// fmt.Printf("It's a list!\n")
// 	}
// 	// fmt.Printf("Offset is %s\n", lhsOffset)
// 	// if isList {
// 	// 	// sawList = true
// 	// 	// countArgs--
// 	// 	if i != len(n.Children) - 1 {
// 	// 		panic("Only handle a listk in the last position")
// 	// 	}
// 	// }
// 	return TLChild{lhsOffset: lhsOffset, rhsOffset: rhsOffset, isList: isList}
// }

// func (c *TermListRewrite) BuildCheckTermListItem(ch *CheckHelper, ref Reference, lhsOffset Offset, rhsOffset Offset) TLChild {
// 	// panic("BuildKChecks(): Not handling  TermListRewrite")
// 	for _, v := range c.LHS.Children {
// 		var c *TermListKItem
// 		var ok bool
// 		if c, ok = v.(*TermListKItem); !ok {
// 			panic("BuildKChecks(): Expect only a single rewrite per list for now")
// 		}
// 		// c.Item.BuildKChecks(ch, ref, i + o)
// 		// fmt.Printf("Offset before call: %s", lhsOffset)
// 		tlChild := c.BuildCheckTermListItem(ch, ref, lhsOffset, rhsOffset)
// 		// fmt.Printf("Offset after call: %s", lhsOffset)
// 		_ = tlChild
// 		// lhsOffset = tlChild.lhsOffset
// 		// rhsOffset = tlChild.rhsOffset
// 	}
// 	if len(c.RHS.Children) == 0 {
// 		panic("BuildKChecks() should have at least 1 rhs child")
// 	}
// 	if len(c.RHS.Children) > 1 {
// 		panic("BuildKChecks() not yet handling rhs children > 1")
// 	}

// 	if rhs, ok := c.RHS.Children[0].(*TermListKItem); ok {
// 		myRef := ref
// 		myRef.addPositionOffsetEntry(rhsOffset)
// 		rep := &TermChange{Loc: myRef, Result: rhs.Item} // FIXME hardcoded based on above check
// 		// fmt.Printf(rep.String())
// 		ch.AddReplacement(rep)
// 	} else {
// 		panic("BuildKChecks() no idea what's going on")
// 	}

// 	return TLChild{}
// }

/*
(a => b), c, d
a, (b => c), c
a, b, (c => d)
(((a, b) => c), d)

((a, List) => c, d, e)

((a, b) => List1, (d => List2), f)

*/

func (n *TermListKItem) collectItemInfo(ch *CheckHelper, ref Reference, offset Offset) bool {
	// fmt.Printf("collectItemInfo TermListKItem: %s\n", n.String())
	return n.Item.BuildKChecks(ch, ref, offset)
}

func (n *TermListRewrite) collectItemInfo(ch *CheckHelper, ref Reference, offset Offset) bool {
	// fmt.Printf("collectItemInfo TermListRewrite: %s\n", n.String())

	hasList := n.LHS.BuildKChecksTermListHelper(ch, ref)

	if len(n.RHS.Children) == 0 {
		panic("BuildKChecks() should have at least 1 rhs child")
	}
	if len(n.RHS.Children) > 1 {
		panic("BuildKChecks() not yet handling rhs children > 1")
	}
	if rhs, ok := n.RHS.Children[0].(*TermListKItem); ok {
		myRef := ref
		myRef.addPositionOffsetEntry(offset)
		rep := &TermChange{Loc: myRef, Result: rhs.Item} // FIXME hardcoded based on above check
		// fmt.Printf(rep.String())
		ch.AddReplacement(rep)
	} else {
		panic("BuildKChecks() no idea what's going on")
	}

	return hasList
}

func (n *TermList) BuildKChecksTermListHelper(ch *CheckHelper, ref Reference) bool {
	lasti := len(n.Children) - 1
	hasList := false
	for i, c := range n.Children {

		isList := c.collectItemInfo(ch, ref, &KnownOffset{i})
		if isList {
			hasList = true
		}
		if isList && (i != lasti) {
			panic("Only allowing lists at the end of a ListK for now")
		}
	}

	numArgs := len(n.Children)
	if hasList {
		numArgs--
	}
	checkArgs := NewCheckNumArgs(&KnownOffset{numArgs}, ref, !hasList)		
	ch.AddCheck(checkArgs)

	return hasList
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
