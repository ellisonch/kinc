package kinc

import "fmt"
// KincDefinition

// based on src/go/ast/walk.go

type Visitor interface {
	Visit(node Node) (w Visitor)
}

// Walk traverses an AST in depth-first order: It starts by calling
// v.Visit(node); node must not be nil. If the visitor w returned by
// v.Visit(node) is not nil, Walk is invoked recursively with visitor
// w for each of the non-nil children of node, followed by a call of
// w.Visit(nil).
//
func Walk(v Visitor, node Node) {
	if v = v.Visit(node); v == nil {
		return
	}

	// walk children
	// // (the order of the cases matches the order
	// // of the corresponding node types in ast.go)
	switch n := node.(type) {
		// Comments and fields
		// case *Comment:
		// 	// nothing to do

		// case *CommentGroup:
		// 	for _, c := range n.List {
		// 		Walk(v, c)
		// 	}

		// case *Field:
		// 	if n.Doc != nil {
		// 		Walk(v, n.Doc)
		// 	}
		// 	walkIdentList(v, n.Names)
		// 	Walk(v, n.Type)
		// 	if n.Tag != nil {
		// 		Walk(v, n.Tag)
		// 	}
		// 	if n.Comment != nil {
		// 		Walk(v, n.Comment)
		// 	}
		default:
			fmt.Printf("ast.Walk: unexpected node type %T", n)
			panic("ast.Walk")
	}
	v.Visit(nil)
}