package main

import "fmt"

type Check interface {
	String() string
	GetLoc() Reference
}

type Checks []Check

func (checks Checks) Len() int { return len(checks) }
func (checks Checks) Swap(i, j int) { checks[i], checks[j] = checks[j], checks[i] }
func (checks Checks) Less(i, j int) bool {
	l1 := checks[i].GetLoc()
	l2 := checks[j].GetLoc()
	c := l1.Compare(l2)
	if c <= 0 {
		return true
	}
	return false
}


type CheckNumArgs struct {
	Loc Reference
	Num int
	Exact bool
}
func NewCheckNumArgs (offset Offset, ref Reference, exact bool) *CheckNumArgs {
	if ko, ok := offset.(*KnownOffset); ok {
		return &CheckNumArgs{Num: ko.Offset, Loc: ref, Exact: exact}
	}
	panic("Can only call NewCheckNumArgs() with a known offset")
}

func (ck *CheckNumArgs) GetLoc() Reference {
	return ck.Loc
}
type CheckLabel struct {
	Loc Reference
	Label Label
}
func (ck *CheckLabel) GetLoc() Reference {
	return ck.Loc
}
// type CheckNumCellArgs struct {
// 	Loc Reference
// 	Num int
// 	Exact bool
// }
type CheckSort struct {
	Loc Reference
	Allowable []Label
	Reversed bool
}
func (ck *CheckSort) GetLoc() Reference {
	return ck.Loc
}
func (ch *CheckNumArgs) String() string {
	if ch.Exact {
		return fmt.Sprintf("CheckNumArgs: %s must have %d arguments\n", ch.Loc.String(), ch.Num)
	} else {
		return fmt.Sprintf("CheckNumArgs: %s must have at least %d arguments\n", ch.Loc.String(), ch.Num)
	}
}
func (ch *CheckLabel) String() string {
	return fmt.Sprintf("CheckLabel: %s must have the '%s label\n", ch.Loc.String(), ch.Label)
}
// func (ch *CheckNumCellArgs) String() string {
// 	if ch.Exact {
// 		return fmt.Sprintf("CheckNumCellArgs: %s must have exactly %d things in it\n", ch.Loc.String(), ch.Num)
// 	} else {
// 		return fmt.Sprintf("CheckNumCellArgs: %s must have at least %d things in it\n", ch.Loc.String(), ch.Num)
// 	}
// }
func (ch *CheckSort) String() string {
	reversed := ""
	if ch.Reversed {
		reversed = " NOT"
	}
	return fmt.Sprintf("CheckSort: %s must%s have the %v sort\n", ch.Loc.String(), reversed, ch.Allowable)
}