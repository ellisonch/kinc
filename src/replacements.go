package main

import "fmt"

type Replacement interface {
	String() string
}

type TermChange struct {
	Loc Reference
	Result Node
}
type MapAdd struct {
	Loc Reference
	Entry *Mapping
}

func (ch *TermChange) String() string {
	return fmt.Sprintf("Replacement: %s should be replaced with %s\n", ch.Loc.String(), ch.Result.String())
}
func (ch *MapAdd) String() string {
	return fmt.Sprintf("Replacement: %s should be added to the map at %s\n", ch.Entry.String(), ch.Loc.String(), )
}


