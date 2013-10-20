package main
import "fmt"
import "strings"
// import "strconv"
import "time"
import "runtime/pprof"
import "os"
import . "github.com/ellisonch/k"

// need to deal with terms inside the store

const profile = true
const shouldCheck = false
const upto = 5000000
const printDebug = false

var label_semi KLabel 	= StringLabel("Semi")	
var label_var KLabel 	= StringLabel("Var")
var label_skip KLabel 	= StringLabel("Skip")
var label_plus KLabel 	= StringLabel("Plus")
var label_neg KLabel 	= StringLabel("Neg")
var label_assign KLabel = StringLabel("Assign")
var label_while KLabel 	= StringLabel("While")
var label_if KLabel 	= StringLabel("If")
var label_not KLabel 	= StringLabel("Not")
var label_lte KLabel 	= StringLabel("LTE")
var label_n KLabel 		= StringLabel("n")
var label_s KLabel 		= StringLabel("s")
var label_true KLabel 	= StringLabel("True")
var label_false KLabel 	= StringLabel("False")
var label_zero KLabel 	= Int64Label(0)
var label_one KLabel 	= Int64Label(1)

func k_true() *K { return NewKSpecial(label_true, nil, true, false) }
func k_false() *K { return NewKSpecial(label_false, nil, true, false) }
func k_zero() *K { return NewKSpecial(label_zero, nil, true, false) }
func k_one() *K { return NewKSpecial(label_one, nil, true, false) }
func k_skip() *K { return NewK(label_skip, nil) }
// func label_upto() *K { return Int64Label(upto) }

// var k_true = NewKSpecial(label_true, nil, true, false)
// var k_false = NewKSpecial(label_false, nil, true, false)
// var k_zero = NewKSpecial(label_zero, nil, true, false)
// var k_one = NewKSpecial(label_one, nil, true, false)
var label_upto KLabel 	= Int64Label(upto)
// var k_skip = NewK(label_skip, nil)


// var k_true = NewKSpecial(label_true, nil, true, false)
// var k_false = NewKSpecial(label_false, nil, true, false)
// var k_zero = NewKSpecial(label_zero, nil, true, false)
// var k_one = NewKSpecial(label_one, nil, true, false)
// var label_upto KLabel 	= Int64Label(upto)
// var k_skip = NewK(label_skip, nil)

// var names map[int64]string = map[int64]string{
// 	0: "hole",
// 	1: "fake",
// 	100: ";",
// 	101: "var",
// 	102: "skip",
// 	103: "plus",
// 	104: "-",
// 	105: "true",
// 	106: "false",
// 	107: ":=",
// 	108: "while",
// 	109: "if",
// 	110: "not",
// 	111: "<=",
// 	1000: "n",
// 	1001: "s",
// }

var stateCell map[string]*K = make(map[string]*K)
var kCell Continuation = nil

func stateString() string {
	var state []string = nil
	for variable, value := range stateCell {
		state = append(state, variable + " -> " + value.P_Label().String())
	}
	return "state(" + strings.Join(state, "\n      ") + ")\n" + kCell.String()
}

func main() {
	f, err := os.Create("profiling.dat")
    if err != nil { panic(err) }

	Init()

	appendK(prog1())
	t0 := time.Now()

	if profile {
    	pprof.StartCPUProfile(f)
    }
	repl()
	if profile {
		pprof.StopCPUProfile()
	}
	delta := time.Since(t0)
	fmt.Printf("Took %v\n", delta)
	result := stateCell[label_s.P_Str()].P_Label().P_Data()
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
	n := NewKSpecial(label_n, nil, false, true)
	s := NewKSpecial(label_s, nil, false, true)
	hundred := NewKSpecial(label_upto, nil, true, false)

	l1 := NewK(label_var, []*K{n, s})
	
	l2 := NewK(label_assign, []*K{n, hundred})
	
	l3 := NewK(label_assign, []*K{s, k_zero()})
	sPn := NewK(label_plus, []*K{s, n})
	l5 := NewK(label_assign, []*K{s, sPn})
	negOne := NewK(label_neg, []*K{k_one()})
	nPno := NewK(label_plus, []*K{n, negOne})
	l6 := NewK(label_assign, []*K{n, nPno})
	body := NewK(label_semi, []*K{l5, l6})

	nLTzero := NewK(label_lte, []*K{n, k_zero()})
	guard := NewK(label_not, []*K{nLTzero})
	l4 := NewK(label_while, []*K{guard, body})

	pgm := NewK(label_semi, []*K{l3, l4})
	pgm = NewK(label_semi, []*K{l2, pgm})
	pgm = NewK(label_semi, []*K{l1, pgm})
	return pgm
}



func trimK() {
	top := len(kCell)-1
	kCell[top].Dec()
	kCell = kCell[:top]
}

func setHead(k *K) {
	top := len(kCell)-1
	k.Inc()
	kCell[top].Dec()
	kCell[top] = k
}
func setPreHead(k *K) {
	pre := len(kCell)-2
	k.Inc()
	kCell[pre].Dec()
	kCell[pre] = k
}

func appendK(k *K) {
	k.Inc()
	kCell = append(kCell, k)
}

func handleValue() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	if len(kCell) == 1 {
		return change
	}
	next := kCell[topSpot - 1]
	for i, arg := range next.P_Args() {
		if arg.P_Label() == Label_hole {
			if printDebug { fmt.Printf("Applying 'cooling' rule\n") }
			change = true
			newTop := UpdateArg(next, i, top)
			trimK()
			setHead(newTop)
			break
		}
	}
	return change
}

func handleVariable() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	variable := top.P_Label()
	if value, ok := stateCell[variable.P_Str()]; ok {
		if printDebug { fmt.Printf("Applying 'lookup' rule\n") }
		change = true
		setHead(value)
	}
	return change
}

func handleSemi() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	if printDebug { fmt.Printf("Applying ';' rule\n") }
	change = true
	left := top.P_Args()[0]
	right := top.P_Args()[1]
	appendK(left)
	setPreHead(right)
	return change
}

func updateStore(keyK *K, value *K) {
	key := keyK.P_Label().P_Str()
	oldK := stateCell[key]
	stateCell[key] = value
	value.Inc()
	if oldK != nil {
		oldK.Dec()
	}
} 

func handleVar() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	if len(top.P_Args()) == 0 {
		if printDebug { fmt.Printf("Applying 'var-empty' rule\n") }
		change = true
		trimK()
	} else {
		if printDebug { fmt.Printf("Applying 'var-something' rule\n") }
		change = true
		updateStore(top.P_Args()[0], k_zero())
		newTop := UpdateTrimArgs(top, 1, len(top.P_Args()))
		setHead(newTop)
	}
	return change
}

func handleAssign() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	right := top.P_Args()[1]
	if !right.P_Value() {
		if printDebug { fmt.Printf("Applying ':=-heat' rule\n") }
		change = true
		appendK(right)
		newTop := UpdateArg(top, 1, Hole)
		setPreHead(newTop)
		// kCell[topSpot] = newTop
		// setHead(newTop)
	} else {
		if printDebug { fmt.Printf("Applying 'assign' rule\n") }
		change = true
		updateStore(top.P_Args()[0], right)
		trimK()
	}
	return change
}

func handleWhile() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	if printDebug { fmt.Printf("Applying 'while' rule\n") }
	change = true
	guard := top.P_Args()[0]
	body := top.P_Args()[1]
	then := NewK(label_semi, []*K{body, top})
	theIf := NewK(label_if, []*K{guard, then, k_skip()})
	setHead(theIf)

	return change
}

func handleIf() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	guard := top.P_Args()[0]
	if !guard.P_Value() {
		if printDebug { fmt.Printf("Applying 'if-heat' rule\n") }
		change = true
		appendK(guard)
		newTop := UpdateArg(top, 0, Hole)
		setPreHead(newTop)
	} else {
		if guard.P_Label() == label_true {
			if printDebug { fmt.Printf("Applying 'if-true' rule\n") }
			change = true
			setHead(top.P_Args()[1])
		} else if guard.P_Label() == label_false {
			if printDebug { fmt.Printf("Applying 'if-false' rule\n") }
			change = true
			setHead(top.P_Args()[2])
		}
	}

	return change
}

func handleNot() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	body := top.P_Args()[0]
	if !body.P_Value() {
		if printDebug { fmt.Printf("Applying 'not-heat' rule\n") }
		change = true
		appendK(body)
		newTop := UpdateArg(top, 0, Hole)
		setPreHead(newTop)
	} else {
		if top.P_Args()[0].P_Label() == label_false {
			if printDebug { fmt.Printf("Applying 'not-false' rule\n") }
			change = true
			setHead(k_true())
		} else if top.P_Args()[0].P_Label() == label_true {
			if printDebug { fmt.Printf("Applying 'not-true' rule\n") }
			change = true
			setHead(k_false())
		}
	}

	return change
}

func handleLTE() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	left := top.P_Args()[0]
	right := top.P_Args()[1]
	if !left.P_Value() {
		if printDebug { fmt.Printf("Applying '<=-heat-left' rule\n") }
		change = true
		appendK(left)
		newTop := UpdateArg(top, 0, Hole)
		setPreHead(newTop)
	} else if !right.P_Value() {
		if printDebug { fmt.Printf("Applying '<=-heat-right' rule\n") }
		change = true
		appendK(right)
		newTop := UpdateArg(top, 1, Hole)		
		setPreHead(newTop)
	} else {
		if printDebug { fmt.Printf("Applying '<=' rule\n") }
		change = true
		leftv := left.P_Label().P_Data()
		rightv := right.P_Label().P_Data()
		if leftv <= rightv {
			setHead(k_true())
		} else {
			setHead(k_false())
		}
	}

	return change
}

func handlePlus() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	left := top.P_Args()[0]
	right := top.P_Args()[1]
	if !left.P_Value() {
		if printDebug { fmt.Printf("Applying '+-heat-left' rule\n") }
		change = true
		appendK(left)
		newTop := UpdateArg(top, 0, Hole)
		setPreHead(newTop)
	} else if !right.P_Value() {
		if printDebug { fmt.Printf("Applying '+-heat-right' rule\n") }
		change = true
		appendK(right)
		newTop := UpdateArg(top, 1, Hole)
		setPreHead(newTop)
	} else {
		if printDebug { fmt.Printf("Applying '+' rule\n") }
		change = true
		leftv := left.P_Label().P_Data()
		rightv := right.P_Label().P_Data()
		newTop := NewKSpecial(Int64Label(leftv + rightv), nil, true, false)
		setHead(newTop)
	}

	return change
}

func handleNeg() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	body := top.P_Args()[0]
	if !body.P_Value() {
		if printDebug { fmt.Printf("Applying 'neg-heat' rule\n") }
		change = true
		appendK(body)
		newTop := UpdateArg(top, 0, Hole)
		setPreHead(newTop)
	} else {
		if printDebug { fmt.Printf("Applying 'neg' rule\n") }
		change = true
		value := body.P_Label().P_Data()
		newValue := -value
		newTop := NewKSpecial(Int64Label(newValue), nil, true, false)
		setHead(newTop)
	}

	return change
}

func handleSkip() bool {
	change := false

	if printDebug { fmt.Printf("Applying 'skip' rule\n") }
	change = true
	trimK()

	return change
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
			fmt.Printf(stateString() + "\n-----------------\n")
			panic("Safety check!")
		}
		if shouldCheck { kCell.Check(stateCell) }
		topSpot := len(kCell) - 1
		top := kCell[topSpot]

		if top.P_Value() == true {
			change = handleValue()
		} else if top.P_Variable() == true {
			change = handleVariable()
		} else {
			var topLabel KLabel = top.P_Label()
			switch {
				case topLabel.Equals(label_semi): change = handleSemi()
				case topLabel.Equals(label_var): change = handleVar()
				case topLabel.Equals(label_assign): change = handleAssign()
				case topLabel.Equals(label_while): change = handleWhile()
				case topLabel.Equals(label_if): change = handleIf()
				case topLabel.Equals(label_not): change = handleNot()
				case topLabel.Equals(label_lte): change = handleLTE()
				case topLabel.Equals(label_plus): change = handlePlus()
				case topLabel.Equals(label_neg): change = handleNeg()
				case topLabel.Equals(label_skip): change = handleSkip()
			}
		}
	}
}
