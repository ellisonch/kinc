package kinc

import "fmt"

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
type Node interface {
    // 36		Pos() token.Pos // position of first character belonging to the node
    // 37		End() token.Pos // position of first character immediately after the node
}

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
	Type CellType
	Name string
	// Type CellType
	Computation *Term
	Bag Bag
	Map Map
}


type Map []*MapItem
type Bag []*BagItem


type Rule struct {
	Bag Bag
	When *When
}

func (r Rule) String() string {
	return fmt.Sprintf("rule %s\n%s", r.Bag.String(), r.When.String())
}


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


type Variable struct {
	Name string
	Sort string
}

type Appl struct {
	Label *Label
	Body []*Term
}

type Kra struct {
	LHS *Term
	RHS *Term
}

type Rewrite struct {
	LHS *Term
	RHS *Term
}


type Label struct {
	Type LabelType
	Name string
	Rewrite LabelRewrite
}

type LabelRewrite struct {
	LHS *Label
	RHS *Label
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

type MapItemType int
const (
	MapError = iota
	MapVariable
	MapMapping
)

type BagItemType int
const (
	BagError = iota
	BagCell
	BagVariable
)

type BagItem struct {
	Type BagItemType
	Cell Cell
	Variable Variable
}

type MapItem struct {
	Type MapItemType
	Variable Variable
	Mapping Mapping
}

type When struct {
	Term *Term
}

type Mapping struct {
	LHS *Term
	RHS *Term
}

