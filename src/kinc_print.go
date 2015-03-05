package main

import "fmt"
import "strings"


func (def *Language) String() string {
	syntax := ""
	for _, sub := range def.Syntax {
		syntax += fmt.Sprintf("\t%s", sub.String())
	}

	rules := ""
	for _, rule := range def.Rules {
		rules += rule.String() + "\n"
	}
	return fmt.Sprintf("%s\n%s\n%s", def.Configuration, syntax, rules)
}

func (c Configuration) String() string {
	return fmt.Sprintf("configuration %s", c.Cell.String())
}

func (n *Subsort) String() string {
	return fmt.Sprintf("%s : %s", n.Subsort, n.Sort)
}

func (c CCell) String() string {
	var inside string

	if c.Magic == "" {
		children := ""
		for _, cell := range c.Children {
			children += cell.String()
		}
		inside = children
	} else {
		inside = c.Magic
	}
	return fmt.Sprintf("<%s %s>%s</%s>", c.Name, c.Attributes, inside, c.Name)
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

func (rw *Paren) String() string {
	return fmt.Sprintf("(%s)", rw.Body)
}

func (v *DotK) String() string {
	return ".k"
}
func (v *DotMap) String() string {
	return ".map"
}

func (v *Kra) String() string {
	children := []string{}
	for _, arg := range v.Children {
		children = append(children, arg.String())
	}
	return fmt.Sprintf("(%s)", strings.Join(children, " ~> "))
}

func (v *Variable) String() string {
	if (v.ActualSort != "") {
		return fmt.Sprintf("%s:%s", v.Name, v.ActualSort)
	}

	var sort string
	if (v.Default) {
		sort = ""
	} else {
		sort = fmt.Sprintf(":%s", v.Sort)
	}
	return fmt.Sprintf("%s%s", v.Name, sort)
}

func (a *Appl) String() string {
	return fmt.Sprintf("%s(%s)", a.Label, a.Body.String())
}

func (a *TermList) String() string {
	children := []string{}
	for _, arg := range a.Elements {
		children = append(children, arg.String())
	}
	return fmt.Sprintf("%s", strings.Join(children, ","))
}


func (r *Rewrite) String() string {
	return fmt.Sprintf("(%s => %s)", r.LHS.String(), r.RHS.String())
}

func (rw *NameLabel) String() string {
	return fmt.Sprintf("%s", rw.Name)
}
func (rw *RewriteLabel) String() string {
	return fmt.Sprintf("(%s => %s)", rw.LHS, rw.RHS)
}

func (rw *InjectLabel) String() string {
	switch rw.Type {
		case E_inject_error: panic("Inject label error")
		case E_inject_integer: return fmt.Sprintf("%s{%d}", rw.Name, rw.Int)
		default: panic("Inject label missing case")
	}
}

func (rw *RewriteMapItem) String() string {
	return fmt.Sprintf("(%s => %s)", rw.LHS, rw.RHS)
}



func (r *When) String() string {
	if (r == nil) { 
		return ""
	}
	return fmt.Sprintf("when %s", r.Term.String())
}

func (r *Mapping) String() string {
	return fmt.Sprintf("%s |-> %s", r.LHS.String(), r.RHS.String())
}

func (r Rule) String() string {
	return fmt.Sprintf("rule %s\n%s", r.Bag.String(), r.When.String())
}