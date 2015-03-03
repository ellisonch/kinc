package main

import "strings"

var CellTypes map[string]string

func KincInit() {
	CellTypes = make(map[string]string)
}

// --------------------------------------------

type Node interface {
	String() string
	// 36		Pos() token.Pos // position of first character belonging to the node
	// 37		End() token.Pos // position of first character immediately after the node
}

type Language struct {
	Configuration *Configuration
	Syntax []*Subsort
	Rules []*Rule
}

type Subsort struct {
	Sort string
	Subsort string
}

type Configuration struct {
	Cell *CCell
}

type CCell struct {
	Name string
	Attributes CellAttributes
	Children []*CCell
	Magic string
}

// temporary, gets put in above
type CCellBody struct {
	Children []*CCell
	Magic string
}

type CellAttributes struct {
	Table map[string]string
}

type NameLabel struct {
	Name string
}

type RewriteMapItem struct {
	LHS MapItem
	RHS MapItem
}

type RewriteLabel struct {
	LHS Label
	RHS Label
}
type InjectLabelType int
const (
	E_inject_error InjectLabelType = iota
	E_inject_integer
)
type InjectLabel struct {
	Type InjectLabelType
	Name string
	Int int64
}
type Label interface {
	Node
	labelNode()
	BuildKChecks(*CheckHelper, Reference)
	IsBuiltin() bool
}


func (l *NameLabel) IsBuiltin() bool {
	if strings.HasPrefix(l.Name, "#") {
		return true
	}
	return false
}
func (l *InjectLabel) IsBuiltin() bool {
	return true
}
func (l *RewriteLabel) IsBuiltin() bool {
	return l.LHS.IsBuiltin() || l.RHS.IsBuiltin()
}

func (*InjectLabel) labelNode() {}
func (*RewriteLabel) labelNode() {}
func (*NameLabel) labelNode() {}

type Cell interface {
	Node
	bagItemNode()
	BuildBagChecks(*CheckHelper)
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
	BuildTopMapItemChecks(ch *CheckHelper)
	// GetReference(ch *CheckHelper) Reference
}

type Mapping struct {
	LHS K
	RHS K
}

func (*Variable) mapItemNode() {}
func (*Mapping) mapItemNode() {}
func (*DotMap) mapItemNode() {}
func (*RewriteMapItem) mapItemNode() {}

type BagItem interface {
	Node
	bagItemNode()
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
	BuildKChecks(*CheckHelper, Reference, int)
}

type DotK struct { }
type DotMap struct { }

func (*DotK) kNode() {}
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

