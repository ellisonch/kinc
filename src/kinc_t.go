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

type CCell struct {
	Name string
	Attributes CellAttributes
	Children []CCell
}

type CellAttributes struct {
	Table map[string]string
}

type NameLabel struct {
	Name string
}

type RewriteLabel struct {
	LHS Label
	RHS Label
}
type Label interface {
	Node
	labelNode()
	String() string
}
func (*RewriteLabel) labelNode() {}
func (*NameLabel) labelNode() {}

type Cell interface {
	Node
	cellNode()
	bagItemNode()
	String() string
}

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
	Computation Term
}

func (*BagCell) cellNode() {}
func (*MapCell) cellNode() {}
func (*ComputationCell) cellNode() {}

func (*BagCell) bagItemNode() {}
func (*MapCell) bagItemNode() {}
func (*ComputationCell) bagItemNode() {}

type MapItem interface {
	Node
	mapItemNode()
	String() string
}

type Mapping struct {
	LHS Term
	RHS Term
}

func (*Variable) mapItemNode() {}
func (*Mapping) mapItemNode() {}

type BagItem interface {
	Node
	bagItemNode()
	String() string
}


func (*Variable) bagItemNode() {}

type Map []MapItem
type Bag []BagItem


type Rule struct {
	Bag Bag
	When *When
}

type Term interface {
	Node
	termNode()
	String() string
}

func (*Variable) termNode() {}
// func (*Int64) termNode() {}
func (*Rewrite) termNode() {}
func (*Appl) termNode() {}
func (*Kra) termNode() {}
func (*Paren) termNode() {}

// type Int64 struct {
// 	Value int64
// }

type Paren struct {
	Term Term
}

type Variable struct {
	Name string
	Sort string
}

type Appl struct {
	Label Label
	Body []Term
}

type Kra struct {
	LHS Term
	RHS Term
}

type Rewrite struct {
	LHS Term
	RHS Term
}

type When struct {
	Term Term
}


