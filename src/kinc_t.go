package kinc

var CellTypes map[string]string

func KincInit() {
	CellTypes = make(map[string]string)
}

// --------------------------------------------

type Node interface {
	// 36		Pos() token.Pos // position of first character belonging to the node
	// 37		End() token.Pos // position of first character immediately after the node
}

type KincDefinition struct {
	Configuration Configuration
	Rules []Rule
}

type Configuration struct {
	Cell CCell
}

// type CellType int
// const (
// 	CellError = iota
// 	CellComputation
// 	CellMap
// 	CellBag
// )

type CCell struct {
	Name string
	Attributes CellAttributes
	Children []CCell
}

type CellAttributes struct {
	Table map[string]string
}

type Cell interface {
	Node
	cellNode()
	String() string
}

// type Cell struct {
// 	Type CellType
// 	Name string
// 	Computation *Term
// 	Bag Bag
// 	Map Map
// }

type BagCell struct {
	Name string
	Bag Bag
}
type MapCell struct {
	Name string
	Map Map
}
type ComputationCell struct {
	Name string
	Computation *Term
}

func (*BagCell) cellNode() {}
func (*MapCell) cellNode() {}
func (*ComputationCell) cellNode() {}

type Map []*MapItem
type Bag []*BagItem


type Rule struct {
	Bag Bag
	When *When
}

type Term struct {
	Type TermType
	Variable Variable
	Int64 int64 
	Rewrite Rewrite
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
	E_BagError = iota
	E_BagCell
	E_BagVariable
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

