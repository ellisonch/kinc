package main

import "fmt"

type Check interface {
	String() string	
}
type CheckNumArgs struct {
	Loc Reference
	Num int
	Exact bool
}
type CheckLabel struct {
	Loc Reference
	Label Label
}
type CheckNumCellArgs struct {
	Loc Reference
	Num int
	Exact bool
}
type CheckSort struct {
	Loc Reference
	Allowable []string
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
func (ch *CheckNumCellArgs) String() string {
	if ch.Exact {
		return fmt.Sprintf("CheckNumCellArgs: %s must have exactly %d things in it\n", ch.Loc.String(), ch.Num)
	} else {
		return fmt.Sprintf("CheckNumCellArgs: %s must have at least %d things in it\n", ch.Loc.String(), ch.Num)
	}
}
func (ch *CheckSort) String() string {
	return fmt.Sprintf("CheckSort: %s must have the %v sort\n", ch.Loc.String(), ch.Allowable)
}