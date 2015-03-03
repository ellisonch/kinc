package main

import "fmt"
import "strings"
import "log"
import "sort"

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

type kvp struct {
	k string
	v int
}
type kvps []kvp

func (a kvps) Len() int {
	return len(a)
}
func (a kvps) Swap(i int, j int) {
	a[i], a[j] = a[j], a[i]
}
func (a kvps) Less(i int, j int) bool {
	return a[i].v < a[j].v
}


func RuleToC(ch *CheckHelper, r *Rule, i int) string {
	c := checksToC(ch)
	return fmt.Sprintf("/*\n%s\n*/\nint rule%d(Configuration* config) {\n%s\n\treturn 0;\n}\n", r.String(), i, c)
}

var _subsortMap map[string][]string

func (l *Language) Compile() string {
	symbolMap := l.CompleteLabelSymbols()
	subsortMap := l.CompleteSubsorts()
	_subsortMap = subsortMap

	// fmt.Printf("%v\n", _subsortMap)

	cSymbols := []string{}
	symbols := kvps{}
	for k, v := range symbolMap {
		cSymbols = append(cSymbols, fmt.Sprintf("#define symbol_%s %d", safeForC(k), v))
		symbols = append(symbols, kvp{k, v})
	}

	sort.Sort(symbols)
	orderedSymbolNames := []string{}
	for _, el := range symbols {
		orderedSymbolNames = append(orderedSymbolNames, fmt.Sprintf("\"%s\"", el.k))
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
	ret += fmt.Sprintf("char* givenLabels[] = {\n\t%s\n};\n", strings.Join(orderedSymbolNames, ",\n\t"))
	ret += strings.Join(cSymbols, "\n")
	ret += "\n"
	ret += fmt.Sprintf("int num_labels() {\n\treturn %d;\n}\n", len(symbolMap))
	ret += fmt.Sprintf("char** label_names() {\n\treturn &givenLabels[0];\n}\n")
	ret += strings.Join(cRules, "\n")

	ret += "void repl(Configuration* config) {\n"
	ret += "\tint change;\n"
	ret += "\tdo {\n"
	ret += "\tchange = 0;\n"
	for i := range cRules {
		ret += fmt.Sprintf("\t\tif (rule%d(config) == 0) { change = 1; continue; }\n", i)
	}
	ret += "\t} while (change);\n"
	ret += "}\n"

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
			rhs := compileTerm(n.Result)
			s := fmt.Sprintf("\tcomputation_set_elem(%s, %s, %s);", r, offset, rhs)
			c.Checks = append(c.Checks, s)
		} else {
			panic("Trying to change a term?")
		}
	case *LabelChange:
		if len(n.Loc.Ref) == 0 {
			panic("Empty loc?")
		} else if len(n.Loc.Ref) == 1 {
			panic("Trying to change a cell?")
		} else if len(n.Loc.Ref) == 2 {
			r := compileRef(n.Loc)
			rhs := compileLabel(n.Result)
			s := fmt.Sprintf("\tk_set_label(%s, %s);", r, rhs)
			c.Checks = append(c.Checks, s)
		} 
	case *MapAdd:
		panic("Don't handle mappadd")
	default:
		panic(fmt.Sprintf("Not handling compileReplacement case %s\n", n)) 
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

		case *CheckSort:
			r := compileRef(n.Loc)
			s1 := fmt.Sprintf("\tif (%s->label->type != e_symbol) { return 1; }\n", r)

			cases := []string{}
			for _, s := range n.Allowable {
				this := fmt.Sprintf("(%s->label->symbol_val != symbol_%s)", r, safeForC(s))
				cases = append(cases, this)
			}

			s2 := fmt.Sprintf("\tif (%s) { return 1; }\n", strings.Join(cases, " && "))
			c.Checks = append(c.Checks, s1)
			c.Checks = append(c.Checks, s2)
			
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

func compileLabel(l Label) string {
	switch l := l.(type) {
	case *NameLabel:
		if strings.HasPrefix(l.Name, "#") {
			return fmt.Sprintf("%s()", compileBuiltinLabel(l.Name))
		} else {
			return fmt.Sprintf("SymbolLabel(symbol_%s)", safeForC(l.Name))
		}
	case *InjectLabel:
		panic("Not handling RHS inject label")
	default: panic(fmt.Sprintf("Not handling compileLabel label %s", l))
	}
}


func compileTerm(n Node) string {
	var mylabel string
	// k_new(SymbolLabel(symbol_If), 3, guard, then, k_new_empty(SymbolLabel(symbol_Skip)))

	switch n := n.(type) {
	case *Appl:
		mylabel = compileLabel(n.Label)

		args := []string{}
		for _, arg := range n.Body {
			args = append(args, compileTerm(arg))
		}

		if n.Label.IsBuiltin() {
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