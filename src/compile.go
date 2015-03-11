package main

import "fmt"
import "strings"
import "log"
import "sort"

type C struct {
	Checks []string
	Cleanup []string
}
func (c *C) String() string {
	checks := strings.Join(c.Checks, "\n")
	cleanup := strings.Join(c.Cleanup, "\n")

	s := ""
	s += fmt.Sprintf("\t// Computation:\n%s\n", checks)
	if len(cleanup) > 0 {
		s += fmt.Sprintf("\t// Cleanup:\n%s\n", cleanup)
	}
	return s
}

func checksToC(ch *CheckHelper) *C {
	res := &C{}

	sort.Sort(ch.checks)

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
	return fmt.Sprintf(`
/*
%s
*/
int rule%d(Configuration* config) {
	if (printDebug) { printf("Attempting to apply rule%d\n"); }
%s
	if (printDebug) { printf("Applied rule%d, kcell is\n%%s\n", kCellToString(config->k)); }
	return 0;
}
`, r.String(), i, i, c, i)
}

var _subsortMap map[string][]Label

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

		// for _, check := range ch.checks {
		// 	fmt.Printf("%s\n", check.String())
		// }
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
	c.Checks = append(c.Checks, fmt.Sprintf("\n\t// %s", replacement.String()))
	switch n := replacement.(type) {
	case *TermChange:
		// FIXME: this is horrible
		// fmt.Printf("%s\n", n.Loc.String())
		if len(n.Loc.Ref) == 0 {
			panic("Empty loc?")
		} else if len(n.Loc.Ref) == 1 {
			panic(fmt.Sprintf("Trying to change a cell? %s", replacement.String()))
		}

		var repFunction string
		myloc := n.Loc.Parent()
		r := compileRef(myloc)
		offset := n.Loc.Suffix()

		// top of a cell
		if myloc.RefersToCell() {
			repFunction = "computation_insert_elems"
		} else {
			repFunction = "k_insert_elems"
		}

		numVarargs := fmt.Sprintf("%d", len(n.Result))

		// if len(n.Result) > 1 {
		// 	panic("not handling rhs > 1 just yet")
		// }

		howManyOverwrites := compileOffset(n.OverwriteCount)

		actualLength := ""
		actualLengthCount := 0
		allHelpers := ""
		allRHS := []string{}
		for _, term := range n.Result {
			helpers, rhs, isList := compileTerm(term)	
			allHelpers += helpers

			var lst string
			if isList {
				actualLength += fmt.Sprintf(" + k_num_args(%s)", rhs)
				lst = "E_LIST"
			} else {
				actualLengthCount++
				lst = "E_NOT_LIST"	
			}

			allRHS = append(allRHS, fmt.Sprintf("%s, %s", lst, rhs))
		}

		actualLength = fmt.Sprintf("%d%s", actualLengthCount, actualLength)
		
		s := fmt.Sprintf(allHelpers)
		s += fmt.Sprintf("\t%s(%s, %s, %s, %s, %s, %s);", repFunction, r, offset.String(), howManyOverwrites, actualLength, numVarargs, strings.Join(allRHS, ", "))
		c.Checks = append(c.Checks, s)
	case *LabelChange:
		// panic("label change!")
		if len(n.Loc.Ref) == 0 {
			panic("Empty loc?")
		} else if len(n.Loc.Ref) == 1 {
			panic("Trying to change a cell?")
		} else {
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
	var s string
	if binding.EndList {
		// fmt.Printf("Don't handle binding for endlist: %s\n", binding.String())
		// panic("")
		parent := binding.Loc.Parent()
		r := compileRef(parent)

		suffix := binding.Loc.Suffix()
		varname := binding.Variable.CompiledName()
		if parent.RefersToCell() {
			s = fmt.Sprintf("\tK* %s = computation_without_first_n_arg(%s, %s);", varname, r, suffix.String())
		} else {
			s = fmt.Sprintf("\tK* %s = k_without_first_n_arg(%s, %s);", varname, r, suffix.String())
		}
		c.Cleanup = append(c.Cleanup, fmt.Sprintf("\tk_dispose(%s);", varname))
	} else {
		r := compileRef(binding.Loc)
		s = fmt.Sprintf("\tK* %s = %s;", binding.Variable.CompiledName(), r)
	}
	c.Checks = append(c.Checks, s)
}

func compileCheck(c *C, check Check) {
	switch n := check.(type) {
		// case *CheckNumCellArgs:
		// 	// fmt.Printf("CheckNumCellArgs\n")
		// 	if n.Loc.String() != "k" {
		// 		panic("Only handle checking k length for now")
		// 	}
		// 	var comparison string
		// 	if (n.Exact) {
		// 		comparison = "!="
		// 	} else {
		// 		comparison = "<"
		// 	}
		// 	s := fmt.Sprintf("\n\t// checking %s", n.String())
		// 	s += fmt.Sprintf("\tif (k_length(config->k) %s %d) { return 1; }", comparison, n.Num)
		// 	c.Checks = append(c.Checks, s)

		case *CheckNumArgs:
			if n.Loc.IsCell() {
				if n.Loc.String() != "k" {
					panic("Only handle checking k length for now")
				}
				var comparison string
				if (n.Exact) {
					comparison = "!="
				} else {
					comparison = "<"
				}
				s := fmt.Sprintf("\n\t// checking %s", n.String())
				s += fmt.Sprintf("\tif (k_length(config->k) %s %d) { return 1; }", comparison, n.Num)
				c.Checks = append(c.Checks, s)
			} else {
				r := compileRef(n.Loc)
				s := fmt.Sprintf("\n\t// checking %s", n.String())
				if n.Exact {
					s += fmt.Sprintf("\tif (k_num_args(%s) != %d) { return 1; }", r, n.Num)
				} else {
					s += fmt.Sprintf("\tif (k_num_args(%s) < %d) { return 1; }", r, n.Num)
				}
				c.Checks = append(c.Checks, s)
			}

		case *CheckLabel:
			switch l := n.Label.(type) {
			case *NameLabel:
				r := compileRef(n.Loc)
				s := fmt.Sprintf("\n\t// checking %s", n.String())
				s += fmt.Sprintf("\tif (%s->label->type != e_symbol) { return 1; }\n", r)
				sym := fmt.Sprintf("symbol_%s", safeForC(l.Name))
				s += fmt.Sprintf("\tif (%s->label->symbol_val != %s) { return 1; }", r, sym)
				c.Checks = append(c.Checks, s)
			// FIXME: i don't like that this is in checklabel and isn't a binding
			case *Variable:
				r := compileRef(n.Loc)
				varname := l.CompiledName()
				s := fmt.Sprintf("\n\t// variable checking %s", n.String())
				s += fmt.Sprintf("\tKLabel* %s = %s->label;", varname, r)

				c.Checks = append(c.Checks, s)
			default:
				panic("compileCheck not handling case")
			}

		case *CheckSort:
			r := compileRef(n.Loc)
			// FIXME: not sure if this is right
			s1 := fmt.Sprintf("\n\t// checking %s", n.String())
			s1 += fmt.Sprintf("\tif (%s->label->type != e_symbol) { return 1; }", r)

			cases := []string{}
			for _, s := range n.Allowable {
				this := fmt.Sprintf("(%s->label->symbol_val != %s)", r, compileSymbolName(s))
				cases = append(cases, this)
			}

			reversed := ""
			if n.Reversed {
				reversed = "!"
			}

			s2 := fmt.Sprintf("\tif (%s(%s)) { return 1; }\n", reversed, strings.Join(cases, " && "))
			c.Checks = append(c.Checks, s1)
			c.Checks = append(c.Checks, s2)
			
		default: log.Panicf("Don't handle case %s", n)
	}
}

func compileOffset(o Offset) string {
	switch o := o.(type) {
	case *KnownOffset:
		return fmt.Sprintf("%d", o.Offset)
	case *LengthOffset:
		if o.Loc.RefersToCell() {
			return fmt.Sprintf("(k_length(%s)-%d)", compileRef(o.Loc), o.Minus)
		} else {
			return fmt.Sprintf("(k_num_args(%s)-%d)", compileRef(o.Loc), o.Minus)
		}
	default: panic("Don't handle whatever other case in compileOffset()")
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
			return fmt.Sprintf("%s(%s, %s)", fun, root, n.Offset.String())
		default: panic(fmt.Sprintf("Don't handle compileRefHead case %s", n))
	}
}

func compileLabel(l Label) string {
	switch l := l.(type) {
	case *NameLabel:
		return fmt.Sprintf("SymbolLabel(%s)", compileSymbolName(l))
	case *InjectLabel:
		panic("Not handling RHS inject label")
	case *Variable:
		return l.CompiledName()
	default: panic(fmt.Sprintf("Not handling compileLabel label %s", l))
	}
}

func compileSymbolName(l Label) string {
	switch l := l.(type) {
	case *NameLabel:
		if strings.HasPrefix(l.Name, "#") {
			return fmt.Sprintf("%s()", compileBuiltinLabel(l.Name))
		} else {
			return fmt.Sprintf("symbol_%s", safeForC(l.Name))
		}
	default: panic(fmt.Sprintf("Not handling compileSymbolName label %s", l))
	}
}


func compileTerm(n Node) (aux string, result string, isList bool) {
	a, r, l := compileTermAux(n, "")
	isList = l
	aux = strings.Join(a, "\n")
	if len(a) > 0 {
		aux += "\n"
	}
	result = r
	return
}

func compileTermAux(n Node, namePrefix string) (aux []string, result string, isList bool) {
	isList = false
	var mylabel string
	// k_new(SymbolLabel(symbol_If), 3, guard, then, k_new_empty(SymbolLabel(symbol_Skip)))

	aux = []string{}

	switch n := n.(type) {
	case *Appl:
		mylabel = compileLabel(n.Label)

		// arrayName := fmt.Sprintf("array_%d", depth)
		// array := fmt.Sprintf("\tK** %s = malloc(1000 * sizeof(K*));\n", arrayName)
		// helpers := []string{}
		argNames := []string{}
		// haveList := false

		// var body *TermListList
		// if body, ok := n.Body.(TermListList); !ok {
		// 	panic("compileTermAux() Only handling TermListList body")
		// }

		lists := []int{}
		for i, arg := range n.Body.Children {
			argHelpers, argResult, childIsList := compileTermAux(arg, fmt.Sprintf("%s_%d", namePrefix, i))
			if childIsList {
				if len(lists) > 0 {
					panic("Too many lists, only handling at most 1 now")
				}
				lists = append(lists, i)
				// haveList = true
			}
			aux = append(aux, argHelpers...)
			argName := fmt.Sprintf("arg%s_arg_%d", namePrefix, i)
			aux = append(aux, fmt.Sprintf("\tK* %s = %s; // isList = %t", argName, argResult, childIsList))
			argNames = append(argNames, fmt.Sprintf(argName))
		}
		// aux = append(aux, helpers)
		// aux = append(aux, array)

		// array += fmt.Sprintf("\t%s[%d] = %s;\n", arrayName, i, argName)

		if n.Label.IsBuiltin() {
			if len(n.Body.Children) == 0 {
				// panic("not handling builtins with args yet")
				result = mylabel
			} else {
				var builtinFunction string
				switch n.Label.String() {
					case "#not": builtinFunction = "k_builtin_bool_not"
					default: panic(fmt.Sprintf("Not yet handling %s builtin", n.Label))
				}
				children := []string{}
				for _, arg := range n.Body.Children {
					childAux, child, isList := compileTerm(arg)
					if isList {
						panic("Not handling lists that are arguments to builtins")
					}
					aux = append(aux, childAux)
					children = append(children, child)
				}
				result = fmt.Sprintf("%s(%s)", builtinFunction, strings.Join(children, ", "))
			}
			
		} else if len(n.Body.Children) == 0 {
			result = fmt.Sprintf("k_new_empty(%s)", mylabel)
		} else {
			if len(lists) > 1 {
				panic("not handling multiple lists yet")
			} else if len(lists) == 1 {
				listName := argNames[lists[0]]
				// fmt.Printf("list is %s\n", listName)
				for i := lists[0] - 1; i >= 0; i-- {
					panic("haven't handled prepending")
					// fmt.Printf("prepending %d to %s\n", i, listName)
				}
				for i := lists[0] + 1; i < len(argNames); i++ {
					panic("haven't handled appending")
					// fmt.Printf("prepending %d to %s\n", i, listName)
				}
				result = fmt.Sprintf("k_new_from_k_args(%s, %s)", mylabel, listName)
				// FIXME probably need a Dec here?
			} else {
				result = fmt.Sprintf("k_new(%s, %d, %s)", mylabel, len(argNames), strings.Join(argNames, ", "))
				// result = fmt.Sprintf("k_new_array(%s, %d, arg_array)", mylabel, len(args))
			}
		}

	case *Variable:
		if n.ActualSort == "listk" {
			isList = true//panic("Not yet compileTermAux listk")
		} 
		result = n.CompiledName()
	// case *Kra:
	// 	panic(fmt.Sprintf("compileTermAux(): Don't yet handle kra"))
	// 	// compileTermAux(n.Body, namePrefix)
	case *Paren:
		return compileTermAux(n.Body, namePrefix)

	case *TermListKItem:
		return compileTermAux(n.Item, namePrefix)
		// panic(fmt.Sprintf("Should not be compiling a TermListKItem, but are: %s", n.String()))

	default: panic(fmt.Sprintf("compileTermAux(): Do not handle case %s\n", n.String()))
	}
	return
}

func (v *Variable) CompiledName() string {
	return fmt.Sprintf("variable_%s", v.Name)
}

func compileBuiltinLabel(s string) string {
	s = strings.TrimPrefix(s, "#")
	return fmt.Sprintf("k_builtin_%s_symbol", s)
}