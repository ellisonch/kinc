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

	for _, binding := range ch.bindings {
		compileBinding(res, binding)
	}

	for _, replacement := range ch.replacements {
		compileReplacement(res, replacement)
	}

	if ch.when != nil {
		panic("Not handling when")
	}

	return res
}


func (l *Language) Compile() string {
	symbolMap := l.CompleteLabelSymbols()

	cSymbols := []string{}
	for v, k := range symbolMap {
		cSymbols = append(cSymbols, fmt.Sprintf("#define symbol_%s %d", safeForC(v), k))
	}

	cConfig := compileConfiguration(l.Configuration)
	cnConfig := compileNewConfiguration(l.Configuration)

	cRules := []string{}
	for i, rule := range l.Rules {
		rule.CompleteVariableTypes()

		// fmt.Printf("\nrule: %s", rule.String())
		ch := rule.BuildChecks()
		// ch.symbolMap = symbolMap
		// fmt.Printf(ch.String())
		c := RuleToC(ch, rule, i)
		cRules = append(cRules, c)
		// fmt.Printf("Compilation:\n")
		// fmt.Printf(c)
	}

	ret := ""

	ret += `
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>

#include "lang.h"
`

	ret += fmt.Sprintf("struct Configuration {\n%s\n};\n", strings.Join(cConfig, "\n"))
	ret += strings.Join(cnConfig, "\n")
	ret += "\n"
	// FIXME hardcoded
	ret += fmt.Sprintf("char* get_state_string(Configuration* config) {\n\treturn kCellToString(config->k);\n}\n") 
	ret += "\n"
	ret += strings.Join(cSymbols, "\n")
	ret += "\n"
	ret += strings.Join(cRules, "\n")

	ret += dynamicInit()
	return ret
}

func dynamicInit() string {
	return `
void k_language_init() { }
	`
}

func compileConfiguration(config *Configuration) []string {
	cConfig := []string{}
	cell := config.Cell
	if len(cell.Children) > 0 {
		panic("Don't handle nested config cells yet")
	}
	if t, ok := cell.Attributes.Table["type"]; ok {
		if t == "computation" {
			s := fmt.Sprintf("\tComputationCell* %s;", cell.Name)
			cConfig = append(cConfig, s)
		} else {
			panic("Only handle computation cells!")
		}
	} else {
		panic("Cell needs a type attribute")
	}
	return cConfig
}

func compileNewConfiguration(config *Configuration) []string {
	cConfig := []string{}
	cConfig = append(cConfig, "Configuration* new_configuration(K* pgm) {")
	cConfig = append(cConfig, "\tConfiguration* config = malloc(sizeof(Configuration));")

	cell := config.Cell
	if len(cell.Children) > 0 {
		panic("Don't handle nested config cells yet")
	}
	// fmt.Printf(cell.String())
	var pgm string
	if t, ok := cell.Attributes.Table["type"]; ok {
		if t == "computation" {
			if cell.Magic == "$PGM" {
				pgm = fmt.Sprintf("config->%s", cell.Name)
			}
			s := fmt.Sprintf("\tconfig->%s = newComputationCell();", cell.Name)
			cConfig = append(cConfig, s)
		} else {
			panic("Only handle computation cells!")
		}
	} else {
		panic("Cell needs a type attribute")
	}

	if pgm != "" {
		cConfig = append(cConfig, fmt.Sprintf("\tcomputation_add_front(%s, pgm);", pgm))
	}

	cConfig = append(cConfig, "\treturn config;")
	cConfig = append(cConfig, "}")

	return cConfig
}

func compileReplacement(c *C, replacement Replacement) {
	// result = k_false();
	// K* newTop = result;
	// computation_set_elem(config->k, 0, newTop);
	switch n := replacement.(type) {
	case *TermChange:
		// FIXME: this is horrible
		// fmt.Printf("%s\n", n.Loc.String())
		if len(n.Loc.Ref) == 0 {
			panic("Empty loc?")
		} else if len(n.Loc.Ref) == 1 {
			panic("Trying to change a cell?")
		} else if len(n.Loc.Ref) == 2 {
			myloc := n.Loc
			myloc.Ref = myloc.Ref[:len(n.Loc.Ref)-1]
			r := compileRef(myloc)
			last := n.Loc.Ref[len(n.Loc.Ref)-1]
			// offset := compileRefPart(last, "", false)
			offset := last.String()
			_ = r
			rhs := compileTerm(n.Result)
			s := fmt.Sprintf("\tcomputation_set_elem(%s, %s, %s);", r, offset, rhs)
			c.Checks = append(c.Checks, s)
		} else {
			panic("Trying to change a term?")
		}
	case *MapAdd:
		panic("Don't handle mappadd")
	}

}

func compileBinding(c *C, binding Binding) {
	r := compileRef(binding.Loc)
	s := fmt.Sprintf("\tK* variable_%s = %s;", binding.Variable.Name, r)
	c.Checks = append(c.Checks, s)
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
			s := fmt.Sprintf("\tif (k_length(config->k) %s %d) { return 1; }\n", comparison, n.Num)
			c.Checks = append(c.Checks, s)
		case *CheckLabel:
			l, ok := n.Label.(*NameLabel)
			if !ok {
				panic("Expected NameLabel")
			}
			r := compileRef(n.Loc)
			s := fmt.Sprintf("\tif (%s->label->type != e_symbol) { return 1; }\n", r)
			sym := fmt.Sprintf("symbol_%s", safeForC(l.Name))
			s += fmt.Sprintf("\tif (%s->label->symbol_val != %s) { return 1; }\n", r, sym)
			c.Checks = append(c.Checks, s)

			
		case *CheckNumArgs:
			r := compileRef(n.Loc)
			s := fmt.Sprintf("\tif (k_num_args(%s) != %d) { return 1; }\n", r, n.Num)
			c.Checks = append(c.Checks, s)
			
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

func compileTerm(n Node) string {
	var mylabel string
	builtin := false
	// k_new(SymbolLabel(symbol_If), 3, guard, then, k_new_empty(SymbolLabel(symbol_Skip)))

	switch n := n.(type) {
	case *Appl:
		label := n.Label
		switch l := label.(type) {
		case *NameLabel:
			if strings.HasPrefix(l.Name, "#") {
				builtin = true
				mylabel = fmt.Sprintf("%s()", compileBuiltinLabel(l.Name))
			} else {
				mylabel = fmt.Sprintf("SymbolLabel(symbol_%s)", safeForC(l.Name))
			}
		case *InjectLabel:
			panic("Not handling RHS inject label")
		default: panic(fmt.Sprintf("Not handling RHS label %s", label))
		}

		args := []string{}
		for _, arg := range n.Body {
			args = append(args, compileTerm(arg))
		}

		if builtin {
			if len(args) > 0 {
				panic("not handling builtins with args yet")
			}
			return mylabel
		} else if len(args) == 0 {
			return fmt.Sprintf("k_new_empty(%s)", mylabel)
		} else {
			return fmt.Sprintf("k_new(%s, %d, %s)", mylabel, len(args), strings.Join(args, ", "))
		}

	case *Variable:
		return fmt.Sprintf("variable_%s", n.Name)
	default: panic(fmt.Sprintf("Do not handle case %s\n", n.String()))
	}
}

func compileBuiltinLabel(s string) string {
	s = strings.TrimPrefix(s, "#")
	return fmt.Sprintf("k_builtin_%s", s)
}