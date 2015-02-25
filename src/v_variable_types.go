package main

import "fmt"
import "log"
// import "errors"
// import "strings"

func go_sucks_variable_types() {
	_ = fmt.Printf
}

func (l *Rule) CompleteVariableTypes() {
	vis := new(getVariableTypes)
	// fmt.Printf("Starting walk...")
	Walk(vis, l)

	types := make(map[string]string)

	// fmt.Printf("\n%v\n%v\n", vis.explicitVariables, vis.implicitVariables)

	for _, variable := range vis.explicitVariables {
		if v, ok := types[variable.Name]; ok {
			if v != variable.Sort {
				log.Panicf("%s should have type %s in %s", variable, v, l)
			}
		}
		types[variable.Name] = variable.Sort
	}
	for _, variable := range vis.implicitVariables {
		if _, ok := types[variable.Name]; ok {
			continue
		}
		types[variable.Name] = variable.Sort
	}

	// fmt.Printf("%v\n", types)
	svis := new(setVariableTypes)
	svis.types = types
	Walk(svis, l)
}


func (vis *getVariableTypes) VisitPre(node Node) Visitor {
	// fmt.Printf("Visiting %s\n", node)
	switch n := node.(type) {
	case *Variable:
		// fmt.Printf("Handling %s\n", n)
		if n.Default {
			vis.implicitVariables = append(vis.implicitVariables, n)
		} else {
			vis.explicitVariables = append(vis.explicitVariables, n)
		}
		// pp.s += fmt.Sprintf("%s:%s", n.Name, n.Sort)
	}
	return vis
}

func (vis *getVariableTypes) VisitPost(node Node) { }

type getVariableTypes struct {
	explicitVariables []*Variable
	implicitVariables []*Variable
	err error
}


func (vis *setVariableTypes) VisitPre(node Node) Visitor {
	switch n := node.(type) {
	case *Variable:
		if sort, ok := vis.types[n.Name]; ok {
			n.ActualSort = sort
			// fmt.Printf("%s\n", n.String())	
		} else {
			log.Panicf("%v doesn't contain %s", vis.types, n.Name)
		}		
	}
	return vis
}

func (vis *setVariableTypes) VisitPost(node Node) { }

type setVariableTypes struct {
	types map[string]string
}


