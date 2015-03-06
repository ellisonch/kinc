package main

import "fmt"
import "strings"

type RefPart interface {
	refPart()
	String() string
	Compare(rp RefPart) int
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


func (r1 *RefPartCell) Compare(r2 RefPart) int {
	switch r2 := r2.(type) {
	case *RefPartCell:
		if r1.Name < r2.Name {
			return -1
		} else if r1.Name == r2.Name {
			return 0
		} else {
			return 1
		}
	default: return -1
	}
}
func (r1 *RefPartPosition) Compare(r2 RefPart) int {
	switch r2 := r2.(type) {
	case *RefPartPosition:
		if r1.Offset < r2.Offset {
			return -1
		} else if r1.Offset == r2.Offset {
			return 0
		} else {
			return 1
		}
	default: return -1
	}
}
func (r1 *RefPartMapLookup) Compare(r2 RefPart) int {
	panic("Can't sort K yet")
}

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


func (r *Reference) Parent() Reference {
	if (len(r.Ref) < 2) {
		panic(fmt.Sprintf("Trying to get parent of ref %s that's too small", r.String()))
	}
	ret := *r
	ret.Ref = r.Ref[:len(r.Ref)-1]
	return ret
}
func (r *Reference) Suffix() int {
	if (len(r.Ref) < 2) {
		panic(fmt.Sprintf("Trying to get suffix of ref %s that's too small", r.String()))
	}
	last := r.Ref[len(r.Ref)-1]
	if l, ok := last.(*RefPartPosition); ok {
		return l.Offset
	} else {
		panic(fmt.Sprintf("Trying to get suffix of %s that isn't a RefPartPosition", r.String()))
	}
}
func (r *Reference) LastPart() RefPart {
	if len(r.Ref) == 0 {
		panic("Trying to get last part of empty ref")
	}
	return r.Ref[len(r.Ref)-1]
}

func (r *Reference) IsCell() bool {
	p := r.LastPart()
	if _, ok := p.(*RefPartCell); ok {
		return true
	} else {
		return false
	}
}

func (r1 *Reference) Compare(r2 Reference) int {
	if len(r1.Ref) < len(r2.Ref) {
		return -1
	}
	if len(r1.Ref) > len(r2.Ref) {
		return 1
	}

	for i, r1v := range r1.Ref {
		r2v := r2.Ref[i]
		c := r1v.Compare(r2v)
		if c < 0 {
			return -1
		} else if c > 0 {
			return 1
		}
	}
	return 0
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