package main
import "fmt"
import "strings"
import "strconv"
import "time"
import "runtime/pprof"
import "os"

const printDebug = false

type K struct {
	label KLabel
	args ListK
	value bool
	variable bool
}
func (k K) String() string {
	return k.label.String() + "(" + k.args.String() + ")"
}
func (k *K) Copy() *K {
	var newArgs ListK = nil
	for _, arg := range k.args {
		newArgs = append(newArgs, arg.Copy())
	}
	copy := &K{k.label, newArgs, k.value, k.variable}
	return copy
}

type KLabel string
func (kl KLabel) String() string {
	return string(kl)
}

type ListK []*K
func (lk ListK) String() string {
	var args []string = nil
	for _, item := range lk {
		args = append(args, item.String())
	}
	return strings.Join(args, ",")
}

type Continuation []*K
func (c Continuation) String() string {
	var args []string = nil
	if len(c) == 0 {
		return "k()"
	}
	for i := len(c) - 1; i >= 0; i-- {
		args = append(args, c[i].String())
	}
	return "k(" + strings.Join(args, "\n  ~> ") + ")"
}


var stateCell map[KLabel]*K = make(map[KLabel]*K)
var kCell Continuation = nil

func stateString() string {
	var state []string = nil
	for variable, value := range stateCell {
		state = append(state, string(variable) + " -> " + string(value.label))
	}
	return "state(" + strings.Join(state, "\n      ") + ")\n" + kCell.String()
}

func main() {
	fmt.Printf("foo\n")

	f, err := os.Create("profiling.dat")
    if err != nil { panic(err) }

	kCell = append(kCell, prog1())
	t0 := time.Now()

    pprof.StartCPUProfile(f)
	repl()
	pprof.StopCPUProfile()
	delta := time.Since(t0)
	fmt.Printf("Took %v\n", delta)
}

/*
var n, s ;
n := 100 ;
s := 0 ;
while not(n <= 0) do (
	s := s + n ;
	n := n + -1
)
*/
func prog1() *K {
	n := &K{"n", nil, false, true}
	s := &K{"s", nil, false, true}
	l1 := &K{"var", []*K{n, s}, false, false}
	hundred := &K{"100000", nil, true, false}
	l2 := &K{":=", []*K{n, hundred}, false, false}
	zero := &K{"0", nil, true, false}
	l3 := &K{":=", []*K{s, zero}, false, false}
	sPn := &K{"+", []*K{s, n}, false, false}
	l5 := &K{":=", []*K{s, sPn}, false, false}
	negOne := &K{"-", []*K{&K{"1", nil, true, false}}, false, false}
	nPno := &K{"+", []*K{n, negOne}, false, false}
	l6 := &K{":=", []*K{n, nPno}, false, false}
	body := &K{";", []*K{l5, l6}, false, false}

	nLTzero := &K{"<=", []*K{n, zero}, false, false}
	guard := &K{"not", []*K{nLTzero}, false, false}
	l4 := &K{"while", []*K{guard, body}, false, false}

	pgm := &K{";", []*K{l3, l4}, false, false}
	pgm = &K{";", []*K{l2, pgm}, false, false}
	pgm = &K{";", []*K{l1, pgm}, false, false}
	return pgm
}

func repl() {
	change := true
	for change {
		change = false
		if printDebug {
			fmt.Printf(stateString() + "\n-----------------\n")
		}
		if len(kCell) == 0 {
			break
		}
		if len(kCell) > 10 {
			panic("Safety check!")
		}
		topSpot := len(kCell) - 1
		top := kCell[topSpot]		

		if top.value == true {
			if len(kCell) == 1 {
				continue
			}
			next := kCell[topSpot - 1]
			for i, arg := range next.args {
				if arg.label == "hole" {
					if printDebug { fmt.Printf("Applying 'cooling' rule\n") }
					change = true
					kCell = kCell[:topSpot]
					kCell[topSpot-1] = next.Copy()
					kCell[topSpot-1].args[i] = top
					break
				}
			}
		} else if top.variable == true {
			variable := top.label
			if value, ok := stateCell[variable]; ok {
				if printDebug { fmt.Printf("Applying 'lookup' rule\n") }
				change = true
				kCell[topSpot] = value
			}
		}
		if top.label == ";" {
			if printDebug { fmt.Printf("Applying ';' rule\n") }
			change = true
			kCell = append(kCell, top.args[0])
			kCell[topSpot] = top.args[1]
		} else if top.label == "var" {
			if len(top.args) == 0 {
				if printDebug { fmt.Printf("Applying 'var-empty' rule\n") }
				change = true
				kCell = kCell[:topSpot]
			} else {
				if printDebug { fmt.Printf("Applying 'var-something' rule\n") }
				change = true
				stateCell[top.args[0].label] = &K{"0", nil, true, false}
				newTop := top.Copy()
				newTop.args = newTop.args[1:]
				kCell[topSpot] = newTop
			}
		} else if top.label == ":=" {
			right := top.args[1]
			if !right.value {
				if printDebug { fmt.Printf("Applying ':=-heat' rule\n") }
				change = true
				kCell = append(kCell, right)
				newTop := top.Copy()
				newTop.args[1] = &K{"hole", nil, false, false}
				kCell[topSpot] = newTop
			} else {
				if printDebug { fmt.Printf("Applying 'assign' rule\n") }
				change = true
				variable := top.args[0].label
				stateCell[variable] = right
				kCell = kCell[:topSpot]
			}
		} else if top.label == "while" {
			if printDebug { fmt.Printf("Applying 'while' rule\n") }
			change = true
			skip := &K{"skip", nil, false, false}
			guard := top.args[0]
			body := top.args[1]
			then := &K{";", []*K{body, top}, false, false}
			theIf := &K{"if", []*K{guard, then, skip}, false, false}
			kCell[topSpot] = theIf
		} else if top.label == "if" {
			guard := top.args[0]
			if !guard.value {
				if printDebug { fmt.Printf("Applying 'if-heat' rule\n") }
				change = true
				kCell = append(kCell, guard)
				newTop := top.Copy()
				newTop.args[0] = &K{"hole", nil, false, false}
				kCell[topSpot] = newTop
			} else {
				if guard.label == "true" {
					if printDebug { fmt.Printf("Applying 'if-true' rule\n") }
					change = true
					kCell[topSpot] = top.args[1]
				} else if guard.label == "false" {
					if printDebug { fmt.Printf("Applying 'if-false' rule\n") }
					change = true
					kCell[topSpot] = top.args[2]
				}
			}
		} else if top.label == "not" {
			body := top.args[0]
			if !body.value {
				if printDebug { fmt.Printf("Applying 'not-heat' rule\n") }
				change = true
				kCell = append(kCell, body)
				newTop := top.Copy()
				newTop.args[0] = &K{"hole", nil, false, false}
				kCell[topSpot] = newTop
			} else {
				if top.args[0].label == "false" {
					if printDebug { fmt.Printf("Applying 'not-false' rule\n") }
					change = true
					kCell[topSpot] = &K{"true", nil, true, false}
				} else if top.args[0].label == "true" {
					if printDebug { fmt.Printf("Applying 'not-true' rule\n") }
					change = true
					kCell[topSpot] = &K{"false", nil, true, false}
				}
			}
		} else if top.label == "<=" {
			left := top.args[0]
			right := top.args[1]
			if !left.value {
				if printDebug { fmt.Printf("Applying '<=-heat-left' rule\n") }
				change = true
				kCell = append(kCell, left)
				newTop := top.Copy()
				newTop.args[0] = &K{"hole", nil, false, false}
				kCell[topSpot] = newTop
			} else if !right.value {
				if printDebug { fmt.Printf("Applying '<=-heat-right' rule\n") }
				change = true
				kCell = append(kCell, right)
				newTop := top.Copy()
				newTop.args[1] = &K{"hole", nil, false, false}
				kCell[topSpot] = newTop
			} else {
				if printDebug { fmt.Printf("Applying '<=' rule\n") }
				change = true
				leftv, err := strconv.ParseInt(string(left.label), 10, 64)
				if err != nil { panic(err) }
				rightv, err := strconv.ParseInt(string(right.label), 10, 64)
				if err != nil { panic(err) }
				if leftv <= rightv {
					kCell[topSpot] = &K{"true", nil, true, false}
				} else {
					kCell[topSpot] = &K{"false", nil, true, false}
				}
			}
		} else if top.label == "+" {
			left := top.args[0]
			right := top.args[1]
			if !left.value {
				if printDebug { fmt.Printf("Applying '+-heat-left' rule\n") }
				change = true
				kCell = append(kCell, left)
				newTop := top.Copy()
				newTop.args[0] = &K{"hole", nil, false, false}
				kCell[topSpot] = newTop
			} else if !right.value {
				if printDebug { fmt.Printf("Applying '+-heat-right' rule\n") }
				change = true
				kCell = append(kCell, right)
				newTop := top.Copy()
				newTop.args[1] = &K{"hole", nil, false, false}
				kCell[topSpot] = newTop
			} else {
				if printDebug { fmt.Printf("Applying '+' rule\n") }
				change = true
				leftv, err := strconv.ParseInt(string(left.label), 10, 64)
				if err != nil { panic(err) }
				rightv, err := strconv.ParseInt(string(right.label), 10, 64)
				if err != nil { panic(err) }
				kCell[topSpot] = &K{KLabel(fmt.Sprintf("%d", leftv + rightv)), nil, true, false}
			}
		} else if top.label == "-" {
			body := top.args[0]
			if !body.value {
				if printDebug { fmt.Printf("Applying 'neg-heat' rule\n") }
				change = true
				kCell = append(kCell, body)
				newTop := top.Copy()
				newTop.args[0] = &K{"hole", nil, false, false}
				kCell[topSpot] = newTop
			} else {
				if printDebug { fmt.Printf("Applying 'minus' rule\n") }
				change = true
				value, err := strconv.ParseInt(string(body.label), 10, 64)
				if err != nil { panic(err) }
				newValue := fmt.Sprintf("%d", -value)
				kCell[topSpot] = &K{KLabel(newValue), nil, true, false}
			}
		} else if top.label == "skip" {
			if printDebug { fmt.Printf("Applying 'skip' rule\n") }
			change = true
			kCell = kCell[:topSpot]
		}
	}
}
