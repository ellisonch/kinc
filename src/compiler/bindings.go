package main

import "fmt"

type Binding struct {
	Loc Reference
	Variable *Variable
	EndList bool
}

func (ch *Binding) String() string {
	var endl string
	if ch.EndList {
		endl = " (is end list)"
	}
	return fmt.Sprintf("Binding: bind %s to %s %s\n", ch.Variable.String(), ch.Loc.String(), endl)
}
