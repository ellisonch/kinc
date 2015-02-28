package main

import "fmt"

type Binding struct {
	Loc Reference
	Variable *Variable
}

func (ch *Binding) String() string {
	return fmt.Sprintf("Binding: bind %s to %s\n", ch.Variable.String(), ch.Loc.String())
}
