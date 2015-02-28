package main

import "fmt"

type Replacement struct {
	Loc Reference
	Result Node
}

func (ch *Replacement) String() string {
	return fmt.Sprintf("Replacement: %s should be replaced with %s\n", ch.Loc.String(), ch.Result.String())
}
