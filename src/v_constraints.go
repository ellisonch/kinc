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
func (n *DotK) BuildKChecks(ch *CheckHelper, ref Reference, i int) {
	panic("Don't handle BuildKChecks DotK yet")
}
func (n *Kra) BuildKChecks(ch *CheckHelper, ref Reference, i int) {
	panic("Don't handle BuildKChecks Kra yet")
}
func (n *Variable) BuildKChecks(ch *CheckHelper, ref Reference, i int) {
	ref.addPositionEntry(i)
	// fmt.Printf("bind %s to %s\n", n.String(), ref.String())
	if n.ActualSort == "listk" {
		panic("Shouldn't be able to get here")
	}
	if n.ActualSort != "k" {
		if subs, ok := _subsortMap[n.ActualSort]; ok {
			ck := &CheckSort{Loc: ref, Allowable: subs}
			ch.AddCheck(ck)
			// fmt.Printf("Subsorts of %s are %v\n", n.ActualSort, subs)
		} else {
			// fmt.Printf("No subsorts of %s found\n", n.ActualSort)
			ck := &CheckSort{Loc: ref, Allowable: []string{n.ActualSort}}
			ch.AddCheck(ck)
		}
		// panic(fmt.Sprintf("Only handle variable of sort k; saw: %s", n.ActualSort))
	}
	binding := Binding{Loc: ref, Variable: n}
	ch.AddBinding(binding)
	// panic("Don't handle BuildKChecks Variable yet")
}
func (n *TermListRewrite) BuildKChecks(ch *CheckHelper, ref Reference, i int) {
	panic("BuildKChecks(): Don't handle TermListRewrite yet")
	// for o, v := range n.LHS.Children {
	// 	var c *TermListKItem
	// 	var ok bool
	// 	if c, ok = v.(*TermListKItem); !ok {
	// 		panic("BuildKChecks(): Expect only a single rewrite per list for now")
	// 	}
	// 	c.Item.BuildKChecks(ch, ref, i + o)
	// }
	// if len(n.RHS.Children) == 0 {
	// 	panic("BuildKChecks() should have at least 1 rhs child")
	// }
	// if len(n.RHS.Children) > 1 {
	// 	panic("BuildKChecks() not yet handling rhs children == 1")
	// }

	// if rhs, ok := n.RHS.Children[0].(*TermListKItem); ok {
	// 	ref.addPositionEntry(i)
	// 	rep := &TermChange{Loc: ref, Result: rhs.Item} // FIXME hardcoded based on above check
	// 	// fmt.Printf(rep.String())
	// 	ch.AddReplacement(rep)
	// } else {
	// 	panic("BuildKChecks() no idea what's going on")
	// }
}
func (n *TermListKItem) BuildKChecks(ch *CheckHelper, ref Reference, i int) {
	panic("never get here?")
	// n.Item.BuildKChecks(ch, ref, i)
}



// returns true if was a list
func (c *TermListKItem) BuildChecksInList(ch *CheckHelper, ref Reference, i int) (isList bool) {
	isList = false

	switch c := c.Item.(type) {
	case *Variable:
		if c.ActualSort == "listk" {
			isList = true
			
			newref := ref
			newref.addPositionEntry(i)
			binding := Binding{Loc: newref, Variable: c, EndList: true}
			ch.AddBinding(binding)
		} else {
			c.BuildKChecks(ch, ref, i)
		}
	default:
		c.BuildKChecks(ch, ref, i)
	}

	return
}


func (n *TermList) BuildKChecksTermListHelper(ch *CheckHelper, ref Reference) {
	// fmt.Printf("helper with ref %s\n", ref.String())
	countArgs := 0
	sawList := false
	for i, c := range n.Children {
		countArgs++

		switch c := c.(type) {
		case *TermListKItem:
			isList := c.BuildChecksInList(ch, ref, i)
			if isList {
				sawList = true
				countArgs--
				if i != len(n.Children) - 1 {
					panic("Only handle a listk in the last position")
				}
			} 
		case *TermListRewrite:
			// panic("BuildKChecks(): Not handling  TermListRewrite")
			for o, v := range c.LHS.Children {
				var c *TermListKItem
				var ok bool
				if c, ok = v.(*TermListKItem); !ok {
					panic("BuildKChecks(): Expect only a single rewrite per list for now")
				}
				c.Item.BuildKChecks(ch, ref, i + o)
			}
			if len(c.RHS.Children) == 0 {
				panic("BuildKChecks() should have at least 1 rhs child")
			}
			if len(c.RHS.Children) > 1 {
				panic("BuildKChecks() not yet handling rhs children > 1")
			}

			if rhs, ok := c.RHS.Children[0].(*TermListKItem); ok {
				myRef := ref
				myRef.addPositionEntry(i)
				rep := &TermChange{Loc: myRef, Result: rhs.Item} // FIXME hardcoded based on above check
				// fmt.Printf(rep.String())
				ch.AddReplacement(rep)
			} else {
				panic("BuildKChecks() no idea what's going on")
			}
		}
		
		// c.BuildKChecks(ch, ref, i)
	}

	checkArgs := &CheckNumArgs{Num: countArgs, Loc: ref, Exact: !sawList}
	ch.AddCheck(checkArgs)
}

func (n *Appl) BuildKChecks(ch *CheckHelper, ref Reference, i int) {
	ref.addPositionEntry(i)
	n.Label.BuildKChecks(ch, ref)
	n.Body.BuildKChecksTermListHelper(ch, ref)
}
func (n *Paren) BuildKChecks(ch *CheckHelper, ref Reference, i int) {
	panic("Don't handle BuildKChecks Paren yet")
}
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
