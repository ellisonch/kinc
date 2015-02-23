package kinc

import "fmt"
// import "strings"

type Rules []Rule

type KincDefinition struct {
	// Int int64
	Configuration Configuration
	Rules []Rule
}

func (at *KincDefinition) String() string {	
	return fmt.Sprintf("%s\n%s", at.Configuration, at.Rules)
	// switch at.Type {
	// 	case Error: return "---Error---"
	// 	case Int: return fmt.Sprintf("%d", at.Int)
	// 	case Real: return fmt.Sprintf("%f", at.Real)
	// 	case Appl: return fmt.Sprintf("%s(%s)", at.Appl.Name, at.Appl.Args.String())
	// 	case List: return fmt.Sprintf("[%s]", CommaList(at.List).String())
	// }
	// return "---Error---"
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
	return fmt.Sprintf("<%s>%s</%s>", c.Name, children, c.Name)
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
	// Type CellType
	Children []CCell
}

type Cell struct {
	Name string
	// Type CellType
	Body *Term
}

type Rule struct {
	Term *Term
}

func (r Rule) String() string {
	return fmt.Sprintf("rule %s", r.Term.String())
}

func (rules Rules) String() string {	
	children := ""
	for _, rule := range rules {
		children += rule.String()
		// return fmt.Sprintf("<%s> %s </%s>\n", c.Name, c.Children, c.Name)
	}	
	return children
}

type Term struct {
	Type TermType
	Variable string
	Int64 int64 
	Rewrite *Rewrite
	Cells []Cell
	Appl *Appl
}

func (t *Term) String() string {
	switch t.Type {
		case TermError: return "Error"
		case TermVariable: return t.Variable
		case TermInt64: return "Int64"
		case TermRewrite: return "Rewrite"
		case TermAppl: return "Appl"
		case TermCells: return "Cells"
		default: return "Error"
	}
}

type Appl struct {
	Label *Label
	Body []Term
}

type Rewrite struct {
	LHS *Term
	RHS *Term
}

type Label struct {
	Type LabelType
	Name string
	Rewrite *LabelRewrite
}

type LabelRewrite struct {
	LHS Label
	RHS Label
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
	TermCells
)

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