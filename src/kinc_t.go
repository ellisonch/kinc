package main

var CellTypes map[string]string

func KincInit() {
	CellTypes = make(map[string]string)
}

// --------------------------------------------

type Node interface {
	// 36		Pos() token.Pos // position of first character belonging to the node
	// 37		End() token.Pos // position of first character immediately after the node
}

type Language struct {
	Configuration *Configuration
	Rules []*Rule
}

type Configuration struct {
	Cell *CCell
}

type CCell struct {
	Name string
	Attributes CellAttributes
	Children []*CCell
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
	bagItemNode()
	BuildBagChecks(*CheckHelper)
	String() string
}

type Map []MapItem
type Bag []BagItem

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
	Computation K
}

func (*BagCell) bagItemNode() {}
func (*MapCell) bagItemNode() {}
func (*ComputationCell) bagItemNode() {}

type MapItem interface {
	Node
	mapItemNode()
	String() string
}

type Mapping struct {
	LHS K
	RHS K
}

func (*Variable) mapItemNode() {}
func (*Mapping) mapItemNode() {}

type BagItem interface {
	Node
	bagItemNode()
	String() string
	BuildBagChecks(*CheckHelper)
}

func (*Variable) bagItemNode() {}

type Rule struct {
	Bag Bag
	When *When
}

type K interface {
	Node
	kNode()
	BuildTopKChecks(*CheckHelper)
	String() string
}

func (*Variable) kNode() {}
func (*Rewrite) kNode() {}
func (*Appl) kNode() {}
func (*Kra) kNode() {}
func (*Paren) kNode() {}

type Paren struct {
	Body K
}

type Variable struct {
	Name string
	Sort string
	Default bool

	ActualSort string
}

type Appl struct {
	Label Label
	Body []K
}

type Kra struct {
	Children []K
}

type Rewrite struct {
	LHS K
	RHS K
}

type When struct {
	Term K
}


