package main

import "fmt"
// KincDefinition

// based on src/go/ast/walk.go

type Visitor interface {
	VisitPre(node Node) (w Visitor)
	VisitPost(node Node)
}

// Walk traverses an AST in depth-first order: It starts by calling
// v.Visit(node); node must not be nil. If the visitor w returned by
// v.Visit(node) is not nil, Walk is invoked recursively with visitor
// w for each of the non-nil children of node, followed by a call of
// w.Visit(nil).
//
func Walk(v Visitor, node Node) {
	// if v = v.VisitPre(node); v == nil {
	// 	return
	// }
	// fmt.Printf("Previsiting %s\n", node)
	v.VisitPre(node)

	// walk children
	switch n := node.(type) {
		case *Language:
			Walk(v, n.Configuration)
			for _, c := range n.Rules {
				Walk(v, c)
			}
		case *Configuration:
			Walk(v, n.Cell)

		case *CCell:
			for _, c := range n.Children {
				Walk(v, c)
			}
		case *MapCell:
			Walk(v, n.Map)
		case *ComputationCell:
			Walk(v, n.Computation)

		case *Rule:
			Walk(v, n.Bag)
			if (n.When != nil) {
				Walk(v, n.When)
			}

		case Bag:
			for _, c := range n {
				Walk(v, c)
			}
		case Map:
			for _, c := range n {
				Walk(v, c)
			}

		case *Mapping:
			Walk(v, n.LHS)
			Walk(v, n.RHS)

		case *Kra:
			for _, c := range n.Children {
				Walk(v, c)
			}

		case *Rewrite:
			Walk(v, n.LHS)
			Walk(v, n.RHS)

		case *When:
			Walk(v, n.Term)

		case *Appl:
			Walk(v, n.Label)
			for _, c := range n.Body {
				Walk(v, c)
			}

		case *Variable:
			// fmt.Printf("Var: %s\n", n)

		case *DotK:
		case *DotMap:			

		case *Paren:
			Walk(v, n.Body)

		case *RewriteMapItem:
			Walk(v, n.LHS)
			Walk(v, n.RHS)

		case *NameLabel:

		case *RewriteLabel:
			Walk(v, n.LHS)
			Walk(v, n.RHS)

		case *InjectLabel:

		default:
			fmt.Printf("ast.Walk: unexpected node type %T\n", n)
			panic("ast.Walk")
	}
	// if v = v.VisitPost(node); v == nil {
	// 	return
	// }
	v.VisitPost(node)
	
	// v.Visit(nil)
}