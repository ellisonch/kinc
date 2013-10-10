package main
import "fmt"
import "strings"
// import "strconv"
import "time"
import "runtime/pprof"
import "os"

const printDebug = false
const upto = Int64Label(1000000)

var label_hole StringLabel = StringLabel(0)
var label_semi StringLabel = StringLabel(100)
var label_var StringLabel = StringLabel(101)
var label_skip StringLabel = StringLabel(102)
var label_plus StringLabel = StringLabel(103)
var label_neg StringLabel = StringLabel(104)
var label_true StringLabel = StringLabel(105)
var label_false StringLabel = StringLabel(106)
var label_assign StringLabel = StringLabel(107)
var label_while StringLabel = StringLabel(108)
var label_if StringLabel = StringLabel(109)
var label_not StringLabel = StringLabel(110)
var label_lte StringLabel = StringLabel(111)
var label_n StringLabel = StringLabel(1000)
var label_s StringLabel = StringLabel(1001)

var names map[StringLabel]string = map[StringLabel]string{
	0: "hole",
	100: ";",
	101: "var",
	102: "skip",
	103: "plus",
	104: "-",
	105: "true",
	106: "false",
	107: ":=",
	108: "while",
	109: "if",
	110: "not",
	111: "<=",
	1000: "n",
	1001: "s",
}

var hole *K = &K{label_hole, nil, false, false}

type KLabel interface{
	String() string
}

type StringLabel uint32
type Int64Label int64

func (s StringLabel) String() string {
	return names[s]
}
func (s Int64Label) String() string {
	return fmt.Sprintf("%d", s)
}

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
	if len(k.args) == 0 {
		return k
	}
	newArgs := make(ListK, len(k.args))
	for i, arg := range k.args {
		newArgs[i] = arg.Copy()
	}
	copy := &K{k.label, newArgs, k.value, k.variable}
	return copy
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
		state = append(state, variable.String() + " -> " + value.label.String())
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
	result := stateCell[label_s].label.(Int64Label)
	fmt.Printf("Result: %d\n", result)
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
	n := &K{label_n, nil, false, true}
	s := &K{label_s, nil, false, true}
	l1 := &K{label_var, []*K{n, s}, false, false}
	hundred := &K{upto, nil, true, false}
	l2 := &K{label_assign, []*K{n, hundred}, false, false}
	zero := &K{Int64Label(0), nil, true, false}
	l3 := &K{label_assign, []*K{s, zero}, false, false}
	sPn := &K{label_plus, []*K{s, n}, false, false}
	l5 := &K{label_assign, []*K{s, sPn}, false, false}
	negOne := &K{label_neg, []*K{&K{Int64Label(1), nil, true, false}}, false, false}
	nPno := &K{label_plus, []*K{n, negOne}, false, false}
	l6 := &K{label_assign, []*K{n, nPno}, false, false}
	body := &K{label_semi, []*K{l5, l6}, false, false}

	nLTzero := &K{label_lte, []*K{n, zero}, false, false}
	guard := &K{label_not, []*K{nLTzero}, false, false}
	l4 := &K{label_while, []*K{guard, body}, false, false}

	pgm := &K{label_semi, []*K{l3, l4}, false, false}
	pgm = &K{label_semi, []*K{l2, pgm}, false, false}
	pgm = &K{label_semi, []*K{l1, pgm}, false, false}
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
				if arg.label == label_hole {
					if printDebug { fmt.Printf("Applying 'cooling' rule\n") }
					change = true
					kCell = kCell[:topSpot]
					newTop := next
					newTop.args[i] = top
					kCell[topSpot-1] = newTop
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
		} else {
			var topLabel StringLabel = top.label.(StringLabel)
			if topLabel == label_semi {
				if printDebug { fmt.Printf("Applying ';' rule\n") }
				change = true
				kCell = append(kCell, top.args[0])
				kCell[topSpot] = top.args[1]
			} else if topLabel == label_var {
				if len(top.args) == 0 {
					if printDebug { fmt.Printf("Applying 'var-empty' rule\n") }
					change = true
					kCell = kCell[:topSpot]
				} else {
					if printDebug { fmt.Printf("Applying 'var-something' rule\n") }
					change = true
					stateCell[top.args[0].label] = &K{Int64Label(0), nil, true, false}
					newTop := top
					newTop.args = newTop.args[1:]
					kCell[topSpot] = newTop
				}
			} else if topLabel == label_assign {
				right := top.args[1]
				if !right.value {
					if printDebug { fmt.Printf("Applying ':=-heat' rule\n") }
					change = true
					kCell = append(kCell, right)
					newTop := top
					newTop.args[1] = hole
					kCell[topSpot] = newTop
				} else {
					if printDebug { fmt.Printf("Applying 'assign' rule\n") }
					change = true
					variable := top.args[0].label
					stateCell[variable] = right
					kCell = kCell[:topSpot]
				}
			} else if topLabel == label_while {
				if printDebug { fmt.Printf("Applying 'while' rule\n") }
				change = true
				skip := &K{label_skip, nil, false, false}
				guard := top.args[0].Copy()
				body := top.args[1].Copy()
				then := &K{label_semi, []*K{body, top}, false, false}
				// then := &K{label_semi, []*K{body, top}, false, false}
				theIf := &K{label_if, []*K{guard, then, skip}, false, false}
				kCell[topSpot] = theIf
			} else if topLabel == label_if {
				guard := top.args[0]
				if !guard.value {
					if printDebug { fmt.Printf("Applying 'if-heat' rule\n") }
					change = true
					kCell = append(kCell, guard)
					newTop := top
					newTop.args[0] = hole
					kCell[topSpot] = newTop
				} else {
					if guard.label == label_true {
						if printDebug { fmt.Printf("Applying 'if-true' rule\n") }
						change = true
						kCell[topSpot] = top.args[1]
					} else if guard.label == label_false {
						if printDebug { fmt.Printf("Applying 'if-false' rule\n") }
						change = true
						kCell[topSpot] = top.args[2]
					}
				}
			} else if topLabel == label_not {
				body := top.args[0]
				if !body.value {
					if printDebug { fmt.Printf("Applying 'not-heat' rule\n") }
					change = true
					kCell = append(kCell, body)
					newTop := top
					newTop.args[0] = hole
					kCell[topSpot] = newTop
				} else {
					if top.args[0].label == label_false {
						if printDebug { fmt.Printf("Applying 'not-false' rule\n") }
						change = true
						kCell[topSpot] = &K{label_true, nil, true, false}
					} else if top.args[0].label == label_true {
						if printDebug { fmt.Printf("Applying 'not-true' rule\n") }
						change = true
						kCell[topSpot] = &K{label_false, nil, true, false}
					}
				}
			} else if topLabel == label_lte {
				left := top.args[0]
				right := top.args[1]
				if !left.value {
					if printDebug { fmt.Printf("Applying '<=-heat-left' rule\n") }
					change = true
					kCell = append(kCell, left)
					newTop := top
					newTop.args[0] = hole
					kCell[topSpot] = newTop
				} else if !right.value {
					if printDebug { fmt.Printf("Applying '<=-heat-right' rule\n") }
					change = true
					kCell = append(kCell, right)
					newTop := top
					newTop.args[1] = hole
					kCell[topSpot] = newTop
				} else {
					if printDebug { fmt.Printf("Applying '<=' rule\n") }
					change = true
					leftv := left.label.(Int64Label)
					rightv := right.label.(Int64Label)
					if leftv <= rightv {
						kCell[topSpot] = &K{label_true, nil, true, false}
					} else {
						kCell[topSpot] = &K{label_false, nil, true, false}
					}
				}
			} else if topLabel == label_plus {
				left := top.args[0]
				right := top.args[1]
				if !left.value {
					if printDebug { fmt.Printf("Applying '+-heat-left' rule\n") }
					change = true
					kCell = append(kCell, left)
					newTop := top
					newTop.args[0] = hole
					kCell[topSpot] = newTop
				} else if !right.value {
					if printDebug { fmt.Printf("Applying '+-heat-right' rule\n") }
					change = true
					kCell = append(kCell, right)
					newTop := top
					newTop.args[1] = hole
					kCell[topSpot] = newTop
				} else {
					if printDebug { fmt.Printf("Applying '+' rule\n") }
					change = true
					leftv := left.label.(Int64Label)
					rightv := right.label.(Int64Label)
					kCell[topSpot] = &K{Int64Label(leftv + rightv), nil, true, false}
				}
			} else if topLabel == label_neg {
				body := top.args[0]
				if !body.value {
					if printDebug { fmt.Printf("Applying 'neg-heat' rule\n") }
					change = true
					kCell = append(kCell, body)
					newTop := top
					newTop.args[0] = hole
					kCell[topSpot] = newTop
				} else {
					if printDebug { fmt.Printf("Applying 'minus' rule\n") }
					change = true
					value := body.label.(Int64Label)
					newValue := -value
					kCell[topSpot] = &K{Int64Label(newValue), nil, true, false}
				}
			} else if topLabel == label_skip {
				if printDebug { fmt.Printf("Applying 'skip' rule\n") }
				change = true
				kCell = kCell[:topSpot]
			}
		}
	}
}
