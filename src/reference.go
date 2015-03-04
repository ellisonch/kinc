package main

import "fmt"
import "strings"

type RefPart interface {
	refPart()
	String() string
}
type RefPartCell struct {
	Name string
}
type RefPartPosition struct {
	Offset int
}
type RefPartMapLookup struct {
	Key K
}

func (*RefPartCell) refPart() {}
func (*RefPartPosition) refPart() {}
func (*RefPartMapLookup) refPart() {}

func (r *RefPartCell) String() string {
	return r.Name
}
func (r *RefPartPosition) String() string {
	return fmt.Sprintf("%d", r.Offset)
}
func (r *RefPartMapLookup) String() string {
	return fmt.Sprintf("map[%s]", r.Key.String())
}

type Reference struct {
	Ref []RefPart
}

func (r *Reference) String() string {
	children := []string{}
	for _, c := range r.Ref {
		children = append(children, c.String())
	}
	return strings.Join(children, ".")
}

// FIXME arrggg, this is so dumb
func (r *Reference) addCellEntry(s string) {
	newSlice := make([]RefPart, len(r.Ref), len(r.Ref))
	copy(newSlice, r.Ref)
	r.Ref = newSlice
	r.Ref = append(r.Ref, &RefPartCell{s})
}
func (r *Reference) addPositionEntry(n int) {
	newSlice := make([]RefPart, len(r.Ref), len(r.Ref))
	copy(newSlice, r.Ref)
	r.Ref = newSlice
	r.Ref = append(r.Ref, &RefPartPosition{n})
}
func (r *Reference) addMapLookup(k K) {
	newSlice := make([]RefPart, len(r.Ref), len(r.Ref))
	copy(newSlice, r.Ref)
	r.Ref = newSlice
	r.Ref = append(r.Ref, &RefPartMapLookup{k})
}
// func (r *Reference) setPositionEntry(n int) {
// 	r.Ref[len(r.Ref)-1] = &RefPartPosition{n}
// }