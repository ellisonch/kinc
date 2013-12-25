package main
import "fmt"
import "strings"
// import "strconv"
import "time"
import "runtime/pprof"
import "os"
import . "github.com/ellisonch/k"
import "github.com/ellisonch/aterm"

// TODO/Notes
// need to add static pass
// for most rules, we know what the next rule to take place will be

// need to deal with terms inside the store

const profile = true
const shouldCheck = false
const printDebug = false
// const upto = 1


var label_statements KLabel = StringLabel("Statements")	
var label_var KLabel 	= StringLabel("Var")
var label_skip KLabel 	= StringLabel("Skip")
var label_plus KLabel 	= StringLabel("Plus")
var label_div KLabel 	= StringLabel("Div")
var label_neg KLabel 	= StringLabel("Neg")
var label_assign KLabel = StringLabel("Assign")
var label_while KLabel 	= StringLabel("While")
var label_if KLabel 	= StringLabel("If")
var label_not KLabel 	= StringLabel("Not")
var label_lte KLabel 	= StringLabel("LTE")
var label_true KLabel 	= StringLabel("True")
var label_false KLabel 	= StringLabel("False")
var label_id KLabel		= StringLabel("Id")
var label_bool KLabel	= StringLabel("Bool")
var label_program KLabel= StringLabel("Program")
var label_paren KLabel	= StringLabel("Paren")
var label_zero KLabel 	= Int64Label(0)

func k_true() *K { return NewK(label_bool, []*K{NewK(label_true, nil)}) }
func k_false() *K { return NewK(label_bool, []*K{NewK(label_false, nil)}) }
func k_zero() *K { return NewK(Label_int, []*K{NewK(label_zero, nil)}) }
func k_skip() *K { return NewK(label_skip, nil) }


func isValue(k *K) bool {
	if k.P_Label() == Label_int || k.P_Label() == label_bool {
		return true
	} else {
		return false
	}
}
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
		state = append(state, variable + " -> " + value.String())
	}
	return "state(" + strings.Join(state, "\n      ") + ")\n" + kCell.String()
}

func getProgram() *K {
	l := aterm.NewATermLexer(os.Stdin)
	ret := aterm.ATermParse(l)
	if ret != 0 { 
		panic("Couldn't parse ATerm!")
		// fmt.Printf("%s\n\n", aterm.FinalTerm.String())		
	}
	newk := ATermToK(&aterm.FinalTerm)
	// if newk != nil {
	// 	fmt.Printf("%s\n", newk.String())
	// } else {
	// 	fmt.Printf("nil")
	// }
	return newk
}

func main() {
	f, err := os.Create("profiling.dat")
    if err != nil { panic(err) }
    p1 := getProgram()
    // fmt.Printf("%s\n", p1.String())
	appendK(p1)
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
	if resultk, ok := stateCell["s"]; ok {
		result := resultk.Inner().P_Label().P_Data()
		fmt.Printf("Result: %d\n", result)
	}
	
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
	// fmt.Printf("Handling variable")
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	variable := top.Inner().P_Label()
	if value, ok := stateCell[variable.P_Str()]; ok {
		if printDebug { fmt.Printf("Applying 'lookup' rule\n") }
		change = true
		setHead(value)
	}
	return change
}

func handleStatements() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	if len(top.P_Args()) == 0 {
		if printDebug { fmt.Printf("Applying 'statements-empty' rule\n") }
		change = true
		trimK()
	} else if len(top.P_Args()) == 1 {
		if printDebug { fmt.Printf("Applying 'statements-one' rule\n") }
		change = true
		newTop := top.P_Args()[0]		
		setHead(newTop)
	} else {
		if printDebug { fmt.Printf("Applying 'statements-many' rule\n") }
		change = true
		appendK(top.P_Args()[0])
		newPreHead := UpdateTrimArgs(top, 1, len(top.P_Args()))
		setPreHead(newPreHead)
	}

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
		updateStore(top.Inner().Inner(), k_zero())
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
	if !isValue(right) {
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
		updateStore(top.Inner().P_Args()[0], right)
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
	then := NewK(label_statements, []*K{body, top})
	theIf := NewK(label_if, []*K{guard, then, k_skip()})
	setHead(theIf)

	return change
}

func handleIf() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	guard := top.P_Args()[0]
	if !isValue(guard) {
		if printDebug { fmt.Printf("Applying 'if-heat' rule\n") }
		change = true
		appendK(guard)
		newTop := UpdateArg(top, 0, Hole)
		setPreHead(newTop)
	} else {
		if guard.Inner().P_Label() == label_true {
			if printDebug { fmt.Printf("Applying 'if-true' rule\n") }
			change = true
			setHead(top.P_Args()[1])
		} else if guard.Inner().P_Label() == label_false {
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

	body := top.Inner()
	if !isValue(body) {
		if printDebug { fmt.Printf("Applying 'not-heat' rule\n") }
		change = true
		appendK(body)
		newTop := UpdateArg(top, 0, Hole)
		setPreHead(newTop)
	} else {
		if top.Inner().Inner().P_Label() == label_false {
			if printDebug { fmt.Printf("Applying 'not-false' rule\n") }
			change = true
			setHead(k_true())
		} else if top.Inner().Inner().P_Label() == label_true {
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
	if !isValue(left) {
		if printDebug { fmt.Printf("Applying '<=-heat-left' rule\n") }
		change = true
		appendK(left)
		newTop := UpdateArg(top, 0, Hole)
		setPreHead(newTop)
	} else if !isValue(right) {
		if printDebug { fmt.Printf("Applying '<=-heat-right' rule\n") }
		change = true
		appendK(right)
		newTop := UpdateArg(top, 1, Hole)		
		setPreHead(newTop)
	} else {
		if printDebug { fmt.Printf("Applying '<=' rule\n") }
		change = true
		leftv := left.Inner().P_Label().P_Data()
		rightv := right.Inner().P_Label().P_Data()
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
	if !isValue(left) {
		if printDebug { fmt.Printf("Applying '+-heat-left' rule\n") }
		change = true
		appendK(left)
		newTop := UpdateArg(top, 0, Hole)
		setPreHead(newTop)
	} else if !isValue(right) {
		if printDebug { fmt.Printf("Applying '+-heat-right' rule\n") }
		change = true
		appendK(right)
		newTop := UpdateArg(top, 1, Hole)
		setPreHead(newTop)
	} else {
		if printDebug { fmt.Printf("Applying '+' rule\n") }
		change = true
		leftv := left.Inner().P_Label().P_Data()
		rightv := right.Inner().P_Label().P_Data()
		newTop := NewK(Label_int, []*K{NewK(Int64Label(leftv + rightv), nil)})
		setHead(newTop)
	}

	return change
}

func handleDiv() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	left := top.P_Args()[0]
	right := top.P_Args()[1]
	if !isValue(left) {
		if printDebug { fmt.Printf("Applying '/-heat-left' rule\n") }
		change = true
		appendK(left)
		newTop := UpdateArg(top, 0, Hole)
		setPreHead(newTop)
	} else if !isValue(right) {
		if printDebug { fmt.Printf("Applying '/-heat-right' rule\n") }
		change = true
		appendK(right)
		newTop := UpdateArg(top, 1, Hole)
		setPreHead(newTop)
	} else {
		if printDebug { fmt.Printf("Applying '/' rule\n") }
		change = true
		leftv := left.Inner().P_Label().P_Data()
		rightv := right.Inner().P_Label().P_Data()
		newTop := NewK(Label_int, []*K{NewK(Int64Label(leftv / rightv), nil)})
		setHead(newTop)
	}

	return change
}

func handleNeg() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	body := top.Inner()
	if !isValue(body) {
		if printDebug { fmt.Printf("Applying 'neg-heat' rule\n") }
		change = true
		appendK(body)
		newTop := UpdateArg(top, 0, Hole)
		setPreHead(newTop)
	} else {
		if printDebug { fmt.Printf("Applying 'neg' rule\n") }
		change = true
		value := body.Inner().P_Label().P_Data()
		newValue := -value
		newTop := NewK(Label_int, []*K{NewK(Int64Label(newValue), nil)})
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

func handleProgram() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	if printDebug { fmt.Printf("Applying 'program' rule\n") }
	change = true
	setHead(top.Inner())

	return change
}

func handleParen() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	if printDebug { fmt.Printf("Applying 'paren' rule\n") }
	change = true
	setHead(top.Inner())

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

		var topLabel KLabel = top.P_Label()
		switch {
			case isValue(top): change = handleValue()
			case topLabel.Equals(label_id): change = handleVariable()
			case topLabel.Equals(label_statements): change = handleStatements()
			case topLabel.Equals(label_var): change = handleVar()
			case topLabel.Equals(label_assign): change = handleAssign()
			case topLabel.Equals(label_while): change = handleWhile()
			case topLabel.Equals(label_if): change = handleIf()
			case topLabel.Equals(label_not): change = handleNot()
			case topLabel.Equals(label_lte): change = handleLTE()
			case topLabel.Equals(label_plus): change = handlePlus()
			case topLabel.Equals(label_div): change = handleDiv()
			case topLabel.Equals(label_neg): change = handleNeg()
			case topLabel.Equals(label_program): change = handleProgram()
			case topLabel.Equals(label_paren): change = handleParen()
			// case topLabel.Equals(label_neg): change = handleNeg()
			case topLabel.Equals(label_skip): change = handleSkip()

			/*
rule
	<k> spawn(S) => . ...</k> 
	. => <k> S </k>



			*/

		}
	}
}
