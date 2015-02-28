package main

import "fmt"
import "strings"
import "log"

type C struct {
	Checks []string
}
func (c *C) String() string {
	return strings.Join(c.Checks, "\n")
}

func checksToC(ch *CheckHelper) *C {
	res := &C{}

	for _, check := range ch.checks {
		compileCheck(res, check)
	}

	return res
}

func compileCheck(c *C, check Check) {

	switch n := check.(type) {
		case *CheckNumCellArgs:
			// fmt.Printf("CheckNumCellArgs\n")
			if n.Loc.String() != "k" {
				panic("Only handle checking k length for now")
			}
			var comparison string
			if (n.Exact) {
				comparison = "!="
			} else {
				comparison = "<"
			}
			s := fmt.Sprintf("if (k_length(config->k) %s %d) { return 1; }\n", comparison, n.Num)
			c.Checks = append(c.Checks, s)
		case *CheckLabel:
			r := compileRef(n.Loc)
			s := fmt.Sprintf("if (%s->label->type != e_symbol) { return 1; }\n", r)
			sym := n.Label
			s += fmt.Sprintf("if (%s->label->symbol_val != %s) { return 1; }\n", r, sym)
			c.Checks = append(c.Checks, s)

			
		case *CheckNumArgs:
			fmt.Printf("CheckNumArgs\n")
			
		default: log.Panicf("Don't handle case %s", n)
	}
}

func compileRef(r Reference) string {
	if len(r.Ref) == 0 {
		panic("Empty ref")
	}

	head := r.Ref[0]

	return compileRefHead(head, r.Ref[1:])

	// for _, rp := range r.Ref {
	// 	switch
	// 	k_get_item(config->k, 1)
	// }
}

func compileRefHead(rp RefPart, parts []RefPart) string {
	var root string
	switch n := rp.(type) {
		case *RefPartCell:
			root = fmt.Sprintf("config->%s", n.Name)
		default: panic(fmt.Sprintf("Don't handle compileRefHead case %s", n))
	}
	ret := root
	first := true
	for _, part := range parts {
		ret = compileRefPart(part, ret, first)
		first = false
	}
	return ret
}

func compileRefPart(rp RefPart, root string, inCell bool) string {
	switch n := rp.(type) {
		case *RefPartCell:
			panic("Didn't expect another cell")
		case *RefPartPosition:
			var fun string
			if inCell {
				fun = "k_get_item"
			} else {
				fun = "k_get_arg"
			}
			return fmt.Sprintf("%s(%s, %d)", fun, root, n.Offset)
		default: panic(fmt.Sprintf("Don't handle compileRefHead case %s", n))
	}
}
