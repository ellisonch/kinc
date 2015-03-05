package main

import "fmt"

type Replacement interface {
	String() string
}

type LabelChange struct {
	Loc Reference
	Result Label
}
type TermChange struct {
	Loc Reference
	Result Node
}
type MapAdd struct {
	Loc Reference
	Entry *Mapping
}

func (ch *LabelChange) String() string {
	return fmt.Sprintf("Replacement: label at %s should be replaced with %s", ch.Loc.String(), ch.Result.String())
}
func (ch *TermChange) String() string {
	return fmt.Sprintf("Replacement: %s should be replaced with %s", ch.Loc.String(), ch.Result.String())
}
func (ch *MapAdd) String() string {
	return fmt.Sprintf("Replacement: %s should be added to the map at %s", ch.Entry.String(), ch.Loc.String(), )
}


