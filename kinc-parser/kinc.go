package kinc

import "fmt"
import "strings"

var CellTypes map[string]string

func KincInit() {
	CellTypes = make(map[string]string)
}

type Rules []Rule

type KincDefinition struct {
	// Int int64
	Configuration Configuration
	Rules []Rule
}

func (def *KincDefinition) String() string {
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

// func (l CommaList) String() string {
// 	ss := []string{}
// 	for _, at := range l {
// 		ss = append(ss, at.String())
// 	}
// 	return strings.Join(ss, ",")
// }

type Configuration struct {
	Cell CCell
}

type CellType int
const (
	CellError = iota
	CellComputation
	CellMap
	CellBag
)

type CCell struct {
	Name string
	Attributes CellAttributes
	// Type CellType
	Children []CCell
}

type CellAttributes struct {
	Table map[string]string
}

type Cell struct {
	Name string
	// Type CellType
	Body *Term
}

func (c Cell) String() string {
	return fmt.Sprintf("<%s>%s</%s>", c.Name, c.Body.String(), c.Name)
}

type Rule struct {
	Bag []*BagItem
}

func (r Rule) String() string {
	children := []string{}
	for _, b := range r.Bag {
		children = append(children, b.String())
	}
	return strings.Join(children, " ")
}

// func (rules Rules) String() string {	
// 	children := "xxx"
// 	for _, rule := range rules {
// 		children += rule.String()
// 		// return fmt.Sprintf("<%s> %s </%s>\n", c.Name, c.Children, c.Name)
// 	}	
// 	return children
// }

type Term struct {
	Type TermType
	Variable Variable
	Int64 int64 
	Rewrite Rewrite
	// Cells []Cell
	Appl Appl
	Kra Kra
	Paren *Term
}

func (t *Term) String() string {
	switch t.Type {
		case TermError: return "Error"
		case TermVariable: return t.Variable.String()
		case TermInt64: return fmt.Sprintf("%d", t.Int64)
		case TermRewrite: return t.Rewrite.String()
		case TermAppl: return t.Appl.String()
		case TermKra: return t.Kra.String()
		// case TermCells: 
		// 	children := ""
		// 	for _, cell := range t.Cells {
		// 		children += cell.String()
		// 	}	
		// 	return children
		default: return "Error"
	}
}

type Variable struct {
	Name string
	Sort string
}

func (v Kra) String() string {
	return fmt.Sprintf("%s ~> %s", v.LHS, v.RHS)
}

func (v Variable) String() string {
	return fmt.Sprintf("%s:%s", v.Name, v.Sort)
}

type Appl struct {
	Label *Label
	Body []*Term
}

func (a Appl) String() string {
	children := []string{}
	for _, arg := range a.Body {
		children = append(children, arg.String())
	}
	return fmt.Sprintf("%s(%s)", a.Label, strings.Join(children, ","))
}

type Kra struct {
	LHS *Term
	RHS *Term
}

type Rewrite struct {
	LHS *Term
	RHS *Term
}

func (r Rewrite) String() string {
	return fmt.Sprintf("%s => %s", r.LHS.String(), r.RHS.String())
}

// func (r *Rewrite) String2(x int) string {
// 	fmt.Printf("Rewrite %d\n", x)
// 	// return fmt.Sprintf("%s => %s", r.LHS.String(), "RHS")
// 	return fmt.Sprintf("%p: %p => %s", r, r.LHS, r.RHS.String2(x+1))
// }

// func (r *Rewrite) Strin2() string {
// 	fmt.Printf("Rewrite %d")
// 	// return fmt.Sprintf("%s => %s", r.LHS.String(), "RHS")
// 	return fmt.Sprintf("%p: %p => %s", r, r.LHS.String, r.RHS.String())
// }

type Label struct {
	Type LabelType
	Name string
	Rewrite LabelRewrite
}

func (l *Label) String() string {
	switch l.Type {
		case E_LabelName: return l.Name
		case E_LabelRewrite: return l.Rewrite.String()
		default: return "ERROR"
	}
}

type LabelRewrite struct {
	LHS *Label
	RHS *Label
}

func (rw *LabelRewrite) String() string {
	return fmt.Sprintf("(%s => %s)", rw.LHS, rw.RHS)
}

type LabelType int
const (
	E_LabelError = iota
	E_LabelName
	E_LabelRewrite
)

type TermType int
const (
	TermError = iota
	TermVariable
	TermInt64
	TermRewrite
	TermAppl
	// TermCells
	TermKra
	TermParen
)

type BagType int
const (
	BagError = iota
	BagCell
)

type BagItem struct {
	Type BagType
	Cell Cell
}

func (rw *BagItem) String() string {
	return rw.Cell.String()
}

// type ATerm struct {
// 	Type ATermType
// 	Int int64
// 	Real float64
// 	Appl ATermAppl
// 	List ATermList
// }

// type ATermAppl struct {
// 	Name string
// 	Args CommaList
// }
// type ATermList CommaList
// type CommaList []ATerm

// func (at *ATerm) String() string {
// 	switch at.Type {
// 		case Error: return "---Error---"
// 		case Int: return fmt.Sprintf("%d", at.Int)
// 		case Real: return fmt.Sprintf("%f", at.Real)
// 		case Appl: return fmt.Sprintf("%s(%s)", at.Appl.Name, at.Appl.Args.String())
// 		case List: return fmt.Sprintf("[%s]", CommaList(at.List).String())
// 	}
// 	return "---Error---"
// }
// func (l CommaList) String() string {
// 	ss := []string{}
// 	for _, at := range l {
// 		ss = append(ss, at.String())
// 	}
// 	return strings.Join(ss, ",")
// }