package main

import "fmt"
import "log"
// import "errors"
// import "strings"

func go_sucks_variable_types() {
	_ = fmt.Printf
}

func (l *Rule) VariableTypes() (map[string]string, error) {
	vis := new(variableTypes)
	Walk(vis, l)

	ret := make(map[string]string)

	// fmt.Printf("\n%v\n%v\n", vis.explicitVariables, vis.implicitVariables)

	for _, variable := range vis.explicitVariables {
		if v, ok := ret[variable.Name]; ok {
			if v != variable.Sort {
				log.Panicf("%s should have type %s in %s", variable, v, l)
			}
		}
		ret[variable.Name] = variable.Sort
	}
	for _, variable := range vis.implicitVariables {
		if _, ok := ret[variable.Name]; ok {
			continue
		}
		ret[variable.Name] = variable.Sort
	}

	return ret, nil
}

func (vis *variableTypes) VisitPre(node Node) Visitor {
	switch n := node.(type) {
	case *Variable:
		if n.Default {
			vis.implicitVariables = append(vis.implicitVariables, n)
		} else {
			vis.explicitVariables = append(vis.explicitVariables, n)
		}
		// pp.s += fmt.Sprintf("%s:%s", n.Name, n.Sort)
	}
	return vis
}

func (vis *variableTypes) VisitPost(node Node) { }

type variableTypes struct {
	explicitVariables []*Variable
	implicitVariables []*Variable
	err error
}


