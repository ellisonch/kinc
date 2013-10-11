package main
import "fmt"
import "strings"
// import "strconv"
import "time"
import "runtime/pprof"
import "os"

// need to deal with terms inside the store


const profile = true
const printDebug = false
const shouldCheck = false
const upto = 5000000

var label_hole KLabel   = StringLabel(0)
var label_fake KLabel   = StringLabel(1)
var label_semi KLabel 	= StringLabel(100)	
var label_var KLabel 	= StringLabel(101)
var label_skip KLabel 	= StringLabel(102)
var label_plus KLabel 	= StringLabel(103)
var label_neg KLabel 	= StringLabel(104)
var label_true KLabel 	= StringLabel(105)
var label_false KLabel 	= StringLabel(106)
var label_assign KLabel = StringLabel(107)
var label_while KLabel 	= StringLabel(108)
var label_if KLabel 	= StringLabel(109)
var label_not KLabel 	= StringLabel(110)
var label_lte KLabel 	= StringLabel(111)
var label_n KLabel 		= StringLabel(1000)
var label_s KLabel 		= StringLabel(1001)
var label_zero KLabel 	= Int64Label(0)
var label_one KLabel 	= Int64Label(1)
var label_upto KLabel 	= Int64Label(upto)

var hole = NewK(label_hole, nil)
var k_true = NewKSpecial(label_true, nil, true, false)
var k_false = NewKSpecial(label_false, nil, true, false)
var k_zero = NewKSpecial(label_zero, nil, true, false)
var k_one = NewKSpecial(label_one, nil, true, false)
var k_skip = NewK(label_skip, nil)

var names map[int64]string = map[int64]string{
	0: "hole",
	1: "fake",
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

var deadK []*K

func NewKSpecial(label KLabel, args ListK, value bool, variable bool) *K {
	for _, arg := range args {
		arg.Inc()
	}
	var newK *K
	if len(deadK) > 0 {
		newK = deadK[len(deadK)-1]
		newK.label = label
		newK.args = args
		newK.value = value
		newK.variable = variable
		deadK = deadK[:len(deadK)-1]
	} else {
		newK = &K{label, args, value, variable, 0}
	}
	return newK
}
func NewK(label KLabel, args ListK) *K {
	for _, arg := range args {
		arg.Inc()
	}
	var newK *K
	if len(deadK) > 0 {
		newK = deadK[len(deadK)-1]
		newK.label = label
		newK.args = args
		newK.value = false
		newK.variable = false
		deadK = deadK[:len(deadK)-1]
	} else {
		newK = &K{label, args, false, false, 0}
	}
	return newK
}

func Int64Label(i64 int64) KLabel {
	return KLabel{e_i64, i64}
}
func StringLabel(i64 int64) KLabel {
	return KLabel{e_string, i64}
}

const (
	e_string = iota
	e_i64
)

type KLabel struct {
	kind int
	data int64
}

func (kl KLabel) String() string {
	if kl.kind == e_string {
		return names[kl.data]
	} else {
		return fmt.Sprintf("%d", kl.data)
	}
}

type K struct {
	label KLabel
	args ListK
	value bool
	variable bool
	refs int
}
func (k *K) Inc() {
	k.refs++
}
func (k *K) Dec() {
	k.refs--
	if k.refs < 0 {
		panic(fmt.Sprintf("Term %s has fewer than 0 refs :(", k))
	}
	if (k.refs == 0) {
		if printDebug { fmt.Printf("Dead term {%s}\n", k) }
		for _, arg := range k.args {
			arg.Dec()
		}
		if len(deadK) < 100 {
			if k != hole && k != k_true && k != k_false && k != k_zero && k != k_one && k != k_skip {
				deadK = append(deadK, k)
			}
		}
	}
}
func (k K) String() string {
	// return k.label.String() + "(" + k.args.String() + ")"
	return fmt.Sprintf("%s[%d](%s)", k.label.String(), k.refs, k.args.String())
}
// func (k *K) Copy() *K {
// 	if len(k.args) == 0 {
// 		return k
// 	}
// 	newArgs := make(ListK, len(k.args))
// 	for i, arg := range k.args {
// 		newArgs[i] = arg.Copy()
// 	}
// 	copy := NewKSpecial(k.label, newArgs, k.value, k.variable)
// 	return copy
// }

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



var stateCell map[int64]*K = make(map[int64]*K)
var kCell Continuation = nil

func stateString() string {
	var state []string = nil
	for variable, value := range stateCell {
		state = append(state, names[variable] + " -> " + value.label.String())
	}
	return "state(" + strings.Join(state, "\n      ") + ")\n" + kCell.String()
}

func main() {
	fmt.Printf("foo\n")

	deadK = make([]*K, 0)

	f, err := os.Create("profiling.dat")
    if err != nil { panic(err) }

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
	result := stateCell[label_s.data].label
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
	
	l3 := NewK(label_assign, []*K{s, k_zero})
	sPn := NewK(label_plus, []*K{s, n})
	l5 := NewK(label_assign, []*K{s, sPn})
	negOne := NewK(label_neg, []*K{k_one})
	nPno := NewK(label_plus, []*K{n, negOne})
	l6 := NewK(label_assign, []*K{n, nPno})
	body := NewK(label_semi, []*K{l5, l6})

	nLTzero := NewK(label_lte, []*K{n, k_zero})
	guard := NewK(label_not, []*K{nLTzero})
	l4 := NewK(label_while, []*K{guard, body})

	pgm := NewK(label_semi, []*K{l3, l4})
	pgm = NewK(label_semi, []*K{l2, pgm})
	pgm = NewK(label_semi, []*K{l1, pgm})
	return pgm
}

func (k *K) subterms() map[*K]bool {
	subterms := make(map[*K]bool)
	subterms[k] = true
	for _, arg := range k.args {
		subsubterms := arg.subterms()
		for subsubterm := range subsubterms {
			subterms[subsubterm] = true
		}
	}
	return subterms
}

func (k *K) counts_aux(counts map[*K]int, seen map[*K]bool) {
	if _, ok := counts[k]; !ok { counts[k] = 0 }
	counts[k]++
	if _, ok := seen[k]; ok { return }
	seen[k] = true
	for _, arg := range k.args {
		arg.counts_aux(counts, seen)
	}
}

func (k *K) counts() map[*K]int {
	counts := make(map[*K]int)
	seen := make(map[*K]bool)
	k.counts_aux(counts, seen)
	return counts
}
func (c Continuation) check() {
	allValues := make([]*K, len(c)) // 0, len(stateCell) + len(c)
	copy(allValues, ListK(c))
	for _, val := range stateCell {
		allValues = append(allValues, val)
	}
	
	specialk := &K{label_fake, allValues, false, false, 1}
	counts := specialk.counts()

	fmt.Printf("%+v\n", counts)
	bad := false
	for k, count := range counts {
		if k.refs != count {
			bad = true
			fmt.Printf("Count for %s should be %d!\n", k, count)
		}
	}
	if bad { panic("Bad check()!") }
}

func updateArg(k *K, arg int, newVal *K) *K {
	if printDebug {
		fmt.Printf("Updating %s's %d argument to %s\n", k, arg, newVal)
	}
	if k.refs > 1 {
		if printDebug { fmt.Printf("   Term is shared, need to copy\n") }
		// k.Dec()

		newArgs := make(ListK, len(k.args))
		for i, arg := range k.args {
			newArgs[i] = arg
			// arg.Inc()
		}
		oldK := k
		k = NewKSpecial(k.label, newArgs, k.value, k.variable)
		if printDebug {
			fmt.Printf("   New Old: %s\n", oldK)
			fmt.Printf("   New Copy: %s\n", k)
		}
		// newk.args[arg] = newVal
		// return newk
		// panic(fmt.Sprintf("Trying to modify %s, but already has %d references\n", k, k.refs))
	}
	newVal.Inc()
	k.args[arg].Dec()
	k.args[arg] = newVal
	if printDebug { fmt.Printf("   After updating: %s\n", k) }
	return k
}
func updateTrimArgs(k *K, left int, right int) *K {
	// newk := NewKSpecial(k.label, k.args, k.value, k.variable)
	// // newk := k.Copy()
	// newk.args = newk.args[left:right]
	// return newk
	for i := 0; i < left; i++ {
		k.args[i].Dec()
	}
	for i := right; i < len(k.args); i++ {
		k.args[i].Dec()
	}
	k.args = k.args[left:right]
	return k
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
	for i, arg := range next.args {
		if arg.label == label_hole {
			if printDebug { fmt.Printf("Applying 'cooling' rule\n") }
			change = true
			newTop := updateArg(next, i, top)
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

	variable := top.label
	if value, ok := stateCell[variable.data]; ok {
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
	left := top.args[0]
	right := top.args[1]
	appendK(left)
	setPreHead(right)
	return change
}

func updateStore(keyK *K, value *K) {
	key := keyK.label.data
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

	if len(top.args) == 0 {
		if printDebug { fmt.Printf("Applying 'var-empty' rule\n") }
		change = true
		trimK()
	} else {
		if printDebug { fmt.Printf("Applying 'var-something' rule\n") }
		change = true
		updateStore(top.args[0], k_zero)
		newTop := updateTrimArgs(top, 1, len(top.args))
		setHead(newTop)
	}
	return change
}

func handleAssign() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	right := top.args[1]
	if !right.value {
		if printDebug { fmt.Printf("Applying ':=-heat' rule\n") }
		change = true
		appendK(right)
		newTop := updateArg(top, 1, hole)
		setPreHead(newTop)
		// kCell[topSpot] = newTop
		// setHead(newTop)
	} else {
		if printDebug { fmt.Printf("Applying 'assign' rule\n") }
		change = true
		updateStore(top.args[0], right)
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
	guard := top.args[0]
	body := top.args[1]
	then := NewK(label_semi, []*K{body, top})
	theIf := NewK(label_if, []*K{guard, then, k_skip})
	setHead(theIf)

	return change
}

func handleIf() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	guard := top.args[0]
	if !guard.value {
		if printDebug { fmt.Printf("Applying 'if-heat' rule\n") }
		change = true
		appendK(guard)
		newTop := updateArg(top, 0, hole)
		setPreHead(newTop)
	} else {
		if guard.label == label_true {
			if printDebug { fmt.Printf("Applying 'if-true' rule\n") }
			change = true
			setHead(top.args[1])
		} else if guard.label == label_false {
			if printDebug { fmt.Printf("Applying 'if-false' rule\n") }
			change = true
			setHead(top.args[2])
		}
	}

	return change
}

func handleNot() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	body := top.args[0]
	if !body.value {
		if printDebug { fmt.Printf("Applying 'not-heat' rule\n") }
		change = true
		appendK(body)
		newTop := updateArg(top, 0, hole)
		setPreHead(newTop)
	} else {
		if top.args[0].label == label_false {
			if printDebug { fmt.Printf("Applying 'not-false' rule\n") }
			change = true
			setHead(k_true)
		} else if top.args[0].label == label_true {
			if printDebug { fmt.Printf("Applying 'not-true' rule\n") }
			change = true
			setHead(k_false)
		}
	}

	return change
}

func handleLTE() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	left := top.args[0]
	right := top.args[1]
	if !left.value {
		if printDebug { fmt.Printf("Applying '<=-heat-left' rule\n") }
		change = true
		appendK(left)
		newTop := updateArg(top, 0, hole)
		setPreHead(newTop)
	} else if !right.value {
		if printDebug { fmt.Printf("Applying '<=-heat-right' rule\n") }
		change = true
		appendK(right)
		newTop := updateArg(top, 1, hole)		
		setPreHead(newTop)
	} else {
		if printDebug { fmt.Printf("Applying '<=' rule\n") }
		change = true
		leftv := left.label.data
		rightv := right.label.data
		if leftv <= rightv {
			setHead(k_true)
		} else {
			setHead(k_false)
		}
	}

	return change
}

func handlePlus() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	left := top.args[0]
	right := top.args[1]
	if !left.value {
		if printDebug { fmt.Printf("Applying '+-heat-left' rule\n") }
		change = true
		appendK(left)
		newTop := updateArg(top, 0, hole)
		setPreHead(newTop)
	} else if !right.value {
		if printDebug { fmt.Printf("Applying '+-heat-right' rule\n") }
		change = true
		appendK(right)
		newTop := updateArg(top, 1, hole)
		setPreHead(newTop)
	} else {
		if printDebug { fmt.Printf("Applying '+' rule\n") }
		change = true
		leftv := left.label.data
		rightv := right.label.data
		newTop := NewKSpecial(Int64Label(leftv + rightv), nil, true, false)
		setHead(newTop)
	}

	return change
}

func handleNeg() bool {
	change := false
	topSpot := len(kCell) - 1
	top := kCell[topSpot]

	body := top.args[0]
	if !body.value {
		if printDebug { fmt.Printf("Applying 'neg-heat' rule\n") }
		change = true
		appendK(body)
		newTop := updateArg(top, 0, hole)
		setPreHead(newTop)
	} else {
		if printDebug { fmt.Printf("Applying 'neg' rule\n") }
		change = true
		value := body.label.data
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
		if shouldCheck { kCell.check() }
		topSpot := len(kCell) - 1
		top := kCell[topSpot]

		if top.value == true {
			change = handleValue()
		} else if top.variable == true {
			change = handleVariable()
		} else {
			var topLabel KLabel = top.label
			switch topLabel {
				case label_semi: change = handleSemi()
				case label_var: change = handleVar()
				case label_assign: change = handleAssign()
				case label_while: change = handleWhile()
				case label_if: change = handleIf()
				case label_not: change = handleNot()
				case label_lte: change = handleLTE()
				case label_plus: change = handlePlus()
				case label_neg: change = handleNeg()
				case label_skip: change = handleSkip()
			}
		}
	}
}
