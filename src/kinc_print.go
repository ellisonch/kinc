package main

import "fmt"
import "strings"


func (def *Language) String() string {
	children := ""
	for _, rule := range def.Rules {
		children += rule.String() + "\n"
		// return fmt.Sprintf("<%s> %s </%s>\n", c.Name, c.Children, c.Name)
	}
	return fmt.Sprintf("%s\n%s", def.Configuration, children)
}

func (c Configuration) String() string {
	return fmt.Sprintf("configuration %s", c.Cell.String())
}

func (c CCell) String() string {	
	children := ""
	for _, cell := range c.Children {
		children += cell.String()
		// return fmt.Sprintf("<%s> %s </%s>\n", c.Name, c.Children, c.Name)
	}	
	return fmt.Sprintf("<%s %s>%s</%s>", c.Name, c.Attributes, children, c.Name)
}

func (a CellAttributes) String() string {
	children := []string{}
	for k, v := range a.Table {
		children = append(children, fmt.Sprintf("%s=\"%s\"", k, v))
	}	
	return strings.Join(children, " ")
}

func (c *BagCell) String() string {
	return fmt.Sprintf("<%s>%s</%s>", c.Name, c.Bag.String(), c.Name)
}
func (c *MapCell) String() string {
	return fmt.Sprintf("<%s>%s</%s>", c.Name, c.Map.String(), c.Name)
}
func (c *ComputationCell) String() string {
	return fmt.Sprintf("<%s>%s</%s>", c.Name, c.Computation.String(), c.Name)
}

func (r Bag) String() string {
	children := []string{}
	for _, b := range r {
		children = append(children, b.String())
	}
	return strings.Join(children, " ")
}
func (r Map) String() string {
	children := []string{}
	for _, b := range r {
		children = append(children, b.String())
	}
	return strings.Join(children, " ")
}

// func (t *Term) String() string {
// 	switch t.Type {
// 		case TermError: return "*Term Error"
// 		case TermVariable: return t.Variable.String()
// 		case TermInt64: return fmt.Sprintf("%d", t.Int64)
// 		case TermRewrite: return t.Rewrite.String()
// 		case TermAppl: return t.Appl.String()
// 		case TermKra: return t.Kra.String()
// 		// case TermCells: 
// 		// 	children := ""
// 		// 	for _, cell := range t.Cells {
// 		// 		children += cell.String()
// 		// 	}	
// 		// 	return children
// 		default: return "*Term Missing case"
// 	}
// }

func (rw *Paren) String() string {
	return fmt.Sprintf("(%s)", rw.Term)
}


func (v *Kra) String() string {
	return fmt.Sprintf("%s ~> %s", v.LHS, v.RHS)
}

func (v *Variable) String() string {
	return fmt.Sprintf("%s:%s", v.Name, v.Sort)
}

func (a *Appl) String() string {
	children := []string{}
	for _, arg := range a.Body {
		children = append(children, arg.String())
	}
	return fmt.Sprintf("%s(%s)", a.Label, strings.Join(children, ","))
}


func (r *Rewrite) String() string {
	return fmt.Sprintf("%s => %s", r.LHS.String(), r.RHS.String())
}

func (rw *NameLabel) String() string {
	return fmt.Sprintf("%s", rw.Name)
}
func (rw *RewriteLabel) String() string {
	return fmt.Sprintf("(%s => %s)", rw.LHS, rw.RHS)
}


func (r *When) String() string {
	if (r == nil) { 
		return ""
	}
	return fmt.Sprintf("when %s", r.Term.String())
}

// func (rw *BagItem) String() string {
// 	switch rw.Type {
// 		case E_BagError: return "*BagItem ERROR"
// 		case E_BagCell: return rw.Cell.String()
// 		case E_BagVariable: return rw.Variable.String()
// 		default: return "*BagItem Missing Case"
// 	}
// }

func (r *Mapping) String() string {
	return fmt.Sprintf("%s |-> %s", r.LHS.String(), r.RHS.String())
}


func (r Rule) String() string {
	return fmt.Sprintf("rule %s\n%s", r.Bag.String(), r.When.String())
}