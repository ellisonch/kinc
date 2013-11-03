#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>

#include "uthash.h"
#include "k.h"

#define UPTO (100000)

#define panic(...) (_panic(__func__, __FILE__, __LINE__, __VA_ARGS__))

#define MAX_STATE 26
#define MAX_K 10
#define MAX_GARBAGE_KEPT 1000
#define MAX_GARBAGE_ARG_LEN 5

#define printRefCounts 1

#define printDebug 0
#define shouldCheck 0

char* givenLabels[] = {
	"_hole",
	"Assign",
	"Bool",
	"Div",
	"Id",
	"If",
	"Int",
	"LTE",
	"Neg",
	"Not",
	"Plus",
	"Program",
	"Skip",
	"Statements",
	"Var",
	"While",
	"True",
	"False",
};

int label_hole = 0;
int label_assign = 1;
int label_bool = 2;
int label_div = 3;
int label_id = 4;
int label_if = 5;
int label_int = 6;
int label_lte = 7;
int label_neg = 8;
int label_not = 9;
int label_plus = 10;
int label_program = 11;
int label_skip = 12;
int label_statements = 13;
int label_var = 14;
int label_while = 15;
int label_true = 16;
int label_false = 17;




// typedef struct K K;
// typedef struct ListK ListK;

typedef enum {
	e_symbol,
	e_string,
	e_i64,
} KLabelType;

// typedef struct K *ListK[];
typedef struct {
	KLabelType type;
	union {
		int64_t i64_val;
		int symbol_val;
		const char* string_val;
	};
} KLabel;

typedef struct K {
	KLabel* label;
	struct ListK {
		int cap;
		int len;
		struct K** a;
	}* args;
	int refs;
} K;

typedef struct ListK ListK;

const char* KToString(K* k);
void Dec(K* k);
void Inc(K* k);



K* deadK[MAX_GARBAGE_KEPT];
int deadlen = 0;

ListK* deadLists[MAX_GARBAGE_ARG_LEN][MAX_GARBAGE_KEPT];
int deadListsLen[MAX_GARBAGE_ARG_LEN];


// KLabel Int64Label(int64 i64) {
// 	return (KLabel){type = e_i64, {i64_val = i64}};
// 	// return KLabel{kind: e_i64, data: i64}
// }
KLabel* StringLabel(const char* s) {
	KLabel* newL = (KLabel*)malloc(sizeof(KLabel));
	newL->type = e_string;
	newL->string_val = s;
	return newL;
}
KLabel* SymbolLabel(int s) {
	KLabel* newL = (KLabel*)malloc(sizeof(KLabel));
	newL->type = e_symbol;
	newL->symbol_val = s;
	return newL;
}
KLabel* Int64Label(int64_t i64) {
	KLabel* newL = (KLabel*)malloc(sizeof(KLabel));
	newL->type = e_i64;
	newL->i64_val = i64;
	return newL;
}

_Noreturn void _panic(const char* func, const char* file, int line, const char* format, ...) {
    va_list va;
    va_start(va, format);
    fprintf(stderr, "PANIC! %s() (%s:%d): ", func, file, line);
    vfprintf(stderr, format, va);
    fprintf(stderr, "\n");
    exit(1);
}

// func getDeadList(reqLength int) ListK {
// 	if reqLength > MAX_GARBAGE_ARG_LEN { return nil }
// 	deadList := deadLists[reqLength-1]
// 	if len(deadList) == 0 { return nil }

// 	ret := deadList[len(deadList)-1]
// 	deadLists[reqLength-1] = deadList[:len(deadList)-1]

// 	if len(ret) != reqLength { panic(fmt.Sprintf("Expected list to be of length %d, but it was %d instead", reqLength, len(ret))) }

// 	if ret == nil { panic("Didn't expect ret to be nil") }
// 	if printDebug { fmt.Printf("Returning dead list of length %d: %s\n", reqLength, ret) }
// 	return ret
// }

ListK* newArgs(int count, ...) {
	ListK* args = malloc(sizeof(ListK));
	args->cap = count;
	args->len = count;
	args->a = malloc(sizeof(K*) * count);

    va_list ap;
    va_start(ap, count);
    for (int i = 0; i < count; i++) {
        args->a[i] = va_arg(ap, K*);
    }
    va_end(ap);

    return args;
}

// TODO unsafe
K* Inner(K* k) {
	return k->args->a[0];
}



void Inc(K* k) {
	k->refs++;
}

K* NewK(KLabel* label, ListK* args) {
	if (args == NULL) {
		args = malloc(sizeof(ListK));
		args->cap = 0;
		args->len = 0;
		args->a = malloc(1); // todo: unecessary
	} else {
		for (int i = 0; i < args->len; i++) {
	 		K* arg = args->a[i];
	 		if (arg == NULL) {
	 			panic("Didn't expect nil arg in NewK()");
	 		}
	 		Inc(arg);
	 	}
	}
	
	K* newK = NULL;
	if (deadlen > 0) {
		newK = deadK[deadlen - 1];
		deadlen--;
	} else {
		newK = (K*)malloc(sizeof(K));
	}
	newK->label = label;
	newK->args = args;
	newK->refs = 0;
	return newK;
}

// func NewK(label KLabel, args ListK) *K {
// 	for _, arg := range args {
// 		if arg == nil {
// 			panic("Didn't expect nil arg in NewK()")
// 		}
// 		arg.Inc()
// 	}
// 	var newK *K
// 	if len(deadK) > 0 {
// 		newK = deadK[len(deadK)-1]
// 		newK.label = label
// 		newK.args = args
// 		deadK = deadK[:len(deadK)-1]
// 	} else {
// 		newK = &K{label, args, 0}
// 	}
// 	return newK
// }

// TODO: leaks memory and is unsafe
const char* LabelToString(KLabel* label) {
	if (label->type == e_string) {
		return label->string_val;
	} else if (label->type == e_i64) {
		char* s = malloc(50);
		snprintf(s, 50, "%" PRId64, label->i64_val);
		return s;
	} else if (label->type == e_symbol) {
		return givenLabels[label->symbol_val];
	} else {
		panic("Some unknown label type found");
	}
	// return NULL;
}


// TODO: leaks memory and is unsafe
const char* ListKToString(ListK* args) {
	if (args == NULL) {
		return "";
	}
	char* s = malloc(3000);
	s[0] = '\0';

	// size_t len = strlen(s);
	for (int i = 0; i < args->len; i++) {
		// printf("len: %zu\n", len);
		// printf("%d\n", args->len);
		K* arg = args->a[i];
		strcat(s, KToString(arg));
		if (i < args->len - 1) {
			strcat(s, ", ");
		}
	}
	
	return s;
}


// TODO: leaks memory and is unsafe
const char* KToString(K* k) {
	char* s = malloc(300);
	if (printRefCounts) {
		snprintf(s, 300, "%s[%d](%s)", LabelToString(k->label), k->refs, ListKToString(k->args));
	} else {
		snprintf(s, 300, "%s(%s)", LabelToString(k->label), ListKToString(k->args));
	}
	return s;
}

K* k_true() { return NewK(SymbolLabel(label_bool), newArgs(1, NewK(SymbolLabel(label_true), NULL))); }
K* k_false() { return NewK(SymbolLabel(label_bool), newArgs(1, NewK(SymbolLabel(label_false), NULL))); }
K* k_zero() { return NewK(SymbolLabel(label_int), newArgs(1, NewK(Int64Label(0), NULL))); }
K* k_one() { return NewK(SymbolLabel(label_int), newArgs(1, NewK(Int64Label(1), NULL))); }
K* k_skip() { return NewK(SymbolLabel(label_skip), NULL); }

// Program(
// 	Statements(
// 		Var(Id("n"),Id("s"),Id("r"))
// 		,Assign(Id("n"), Int(1000000))
// 		,Assign(Id("s"), Int(0))
// 		,While(Not(Paren(LTE(Id("n"), Int(0)))), Statements(Assign(Id("s"), Plus(Id("s"), Id("n"))),Assign(Id("n"), Plus(Id("n"), Neg(Int(1))))))
// 	)
// )

K* prog1() {
	K* n = NewK(SymbolLabel(label_id), newArgs(1, NewK(StringLabel("n"), NULL)));
	K* s = NewK(SymbolLabel(label_id), newArgs(1, NewK(StringLabel("s"), NULL)));
	K* hundred = NewK(SymbolLabel(label_int), newArgs(1, NewK(Int64Label(UPTO), NULL)));

	K* l1 = NewK(SymbolLabel(label_var), newArgs(2, n, s));
	K* l2 = NewK(SymbolLabel(label_assign), newArgs(2, n, hundred));
	K* l3 = NewK(SymbolLabel(label_assign), newArgs(2, s, k_zero()));

	K* sPn = NewK(SymbolLabel(label_plus), newArgs(2, s, n));
	K* l5 = NewK(SymbolLabel(label_assign), newArgs(2, s, sPn));
	K* negOne = NewK(SymbolLabel(label_neg), newArgs(1, k_one()));
	K* nPno = NewK(SymbolLabel(label_plus), newArgs(2, n, negOne));
	K* l6 = NewK(SymbolLabel(label_assign), newArgs(2, n, nPno));
	K* body = NewK(SymbolLabel(label_statements), newArgs(2, l5, l6));

	K* nLTzero = NewK(SymbolLabel(label_lte), newArgs(2, n, k_zero()));
	K* guard = NewK(SymbolLabel(label_not), newArgs(1, nLTzero));
	K* l4 = NewK(SymbolLabel(label_while), newArgs(2, guard, body));

	K* pgm = NewK(SymbolLabel(label_statements), newArgs(4, l1, l2, l3, l4));
	return pgm;
	// return l1;
}

int isValue(K* k) {
	if (k->label->type != e_symbol) {
		panic("Expected calling isValue on symbol labels");
	}
	if (k->label->symbol_val == label_int || k->label->symbol_val == label_bool) {
		return 1;
	} else {
		return 0;
	}
}

// var stateCell map[string]*K = make(map[string]*K)
K *stateCell[MAX_STATE];

K *kCell[MAX_K];
int next = 0;

K* Hole() {
	return NewK(SymbolLabel(label_hole), NULL);
}

// FIXME: leaks memory, sucks
char* kCellToString() {
	char* s = malloc(10000);
	strcpy(s, "k(\n");
	for (int i = next - 1; i >= 0; i--) {
		strcat(s, "  ~> ");
		strcat(s, KToString(kCell[i]));
		strcat(s, "\n");
	}
	strcat(s, ")\n");
	return s;
}

// FIXME: leaks memory, sucks
char* stateString() {
	char* s = malloc(20000);
	strcpy(s, "state(\n"); 
	for (int i = 0; i < 26; i++) {
		if (stateCell[i] != NULL) {
			char var[] = "  x -> ";
 			var[2] = i + 'a';
			strcat(s, var);
			strcat(s, KToString(stateCell[i]));
			strcat(s, "\n");
		}
	}
	strcat(s, ")\n");
	strcat(s, kCellToString());
	return s;
}

void collectDeadTerm(K* k) {
	if (printDebug) { printf("Dead term {%s}\n", KToString(k)); }

	if (k->args->len > 10) {
		panic("Sanity check failed!");
	}
	for (int i = 0; i < k->args->len; i++) {
		K* arg = k->args->a[i];
		Dec(arg);
	}
	// free(k->args->a);
	// free(k->args);


	// lenkargs := len(k.args)
	// // don't garbage collect the "builtins"
	// if (lenkargs == 0 && k == Hole) {
	// 	return
	// }

	int lenkargs = k->args->len;
	if (lenkargs < MAX_GARBAGE_ARG_LEN && deadListsLen[lenkargs] < MAX_GARBAGE_KEPT) {
		deadLists[lenkargs][deadListsLen[lenkargs]] = k->args;
		deadListsLen[lenkargs]++;
	} else {
		free(k->args->a);
		free(k->args);
	}
	// if lenkargs < MAX_GARBAGE_ARG_LEN && lenkargs > 0 {
	// 	if len(deadLists[lenkargs-1]) < MAX_GARBAGE_KEPT {
	// 		deadLists[lenkargs-1] = append(deadLists[lenkargs-1], k.args)
	// 	}
	// }

	
	free(k->label);
	if (deadlen < MAX_GARBAGE_KEPT) {
		deadK[deadlen] = k;
		deadlen++;
	} else {
		free(k);
	}

	// if len(deadK) < MAX_GARBAGE_KEPT {
	// 	k.args = nil // not technically needed, just a bit safer
	// 	deadK = append(deadK, k)
	// }
}

void Dec(K* k) {
	// panic("don't handle dec");
	k->refs--;
	int newRefs = k->refs;
	if (newRefs < 0) {
		panic("Term %s has fewer than 0 refs :(", KToString(k));
	}
	if (newRefs == 0) {
		// panic("Dead term found: %s", KToString(k));
		collectDeadTerm(k);
	}
}

void trimK() {
	int top = next - 1;
	Dec(kCell[top]);
	next--;
	// kCell = kCell[:top]
}

void setHead(K* k) {
	int top = next - 1;
	Inc(k);
	Dec(kCell[top]);
	kCell[top] = k;
}

void setPreHead(K* k) {
	int pre = next - 2;
	Inc(k);
	Dec(kCell[pre]);
	kCell[pre] = k;
}


void appendK(K* k) {
	if (next >= MAX_K) {
		panic("Trying to add too many elements to the K Cell!");
	}
	Inc(k);
	kCell[next] = k;
	next++;
}


ListK* copyArgs(ListK* oldArgs) {
	ListK* args = malloc(sizeof(ListK));
	args->cap = oldArgs->cap;
	args->len = oldArgs->len;
	args->a = malloc(sizeof(K*) * oldArgs->cap);

    for (int i = 0; i < oldArgs->len; i++) {
        args->a[i] = oldArgs->a[i];
    }

    return args;
}

KLabel* copyLabel(KLabel* l) {
	KLabel* newL = (KLabel*)malloc(sizeof(KLabel));
	memcpy(newL, l, sizeof(KLabel));
	return newL;
}

K* copy(K* oldK) {
	ListK* newArgs = copyArgs(oldK->args);
	K* k = NewK(copyLabel(oldK->label), newArgs);
	if (printDebug) {
		printf("   New Old: %s\n", KToString(oldK));
		printf("   New Copy: %s\n", KToString(k));
	}
	return k;
}

K* UpdateArg(K* k, int arg, K* newVal) {
	if (printDebug) {
		printf("Updating %s's %d argument to %s\n", KToString(k), arg, KToString(newVal));
	}
	if (k->refs > 1) {
		if (printDebug) {
			printf("   Term is shared, need to copy\n");
		}
		k = copy(k);
	}
	Inc(newVal);
	Dec(k->args->a[arg]);
	k->args->a[arg] = newVal;
	if (printDebug) {
		printf("   After updating: %s\n", KToString(k));
	}
	return k;
}

int handleValue() {
	int change = 0;
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	if (next == 1) {
		return change;
	}

	K* next = kCell[topSpot - 1];
	for (int i = 0; i < next->args->len; i++) {
		K* arg = next->args->a[i];
		if (arg->label->type != e_symbol) {
			panic("Expected string type");
		}
		if (arg->label->symbol_val == label_hole){
			if (printDebug) {
				printf("Applying 'cooling' rule\n");
			}
			change = 1;
			K* newTop = UpdateArg(next, i, top);
			trimK();
			setHead(newTop);
			break;
		}
	}
	
	return change;
}

K* updateTrimArgs(K* k, int left, int right) {
	if (k->refs > 1) {
		if (printDebug) { 
			printf("   Term is shared, need to copy\n");
		}
		k = copy(k);
	}
	for (int i = 0; i < left; i++) {
		Dec(k->args->a[i]);
	}
	for (int i = right; i < k->args->len; i++) {
		Dec(k->args->a[i]);
	}
	// TODO: inefficient
	int newi = 0;
	for (int i = left; i < right; i++) {
		k->args->a[newi] = k->args->a[i];
		newi++;
	}
	k->args->len = right - left;
	// k.args = k.args[left:right];
	return k;
}


void updateStore(K* keyK, K* value) {
	if (keyK->label->type != e_string) {
		panic("Expected key to be string label");
	}
	int key = keyK->label->string_val[0] - 'a';
	K* oldK = stateCell[key];
	stateCell[key] = value;
	Inc(value);
	if (oldK != NULL) {
		Dec(oldK);
	}
}

// TODO: unsafe
int handleVariable() {
	int change = 0;
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	if (Inner(top)->label->type != e_string) {
		panic("Expected key to be string label");
	}
	int variable = Inner(top)->label->string_val[0] - 'a';
	K* value = stateCell[variable];
	if (value == NULL) {
		panic("Trying to read unassigned variable");
	}

	if (printDebug) { printf("Applying 'lookup' rule\n"); }
	change = 1;
	setHead(value);

	return change;
}

int handleStatements() {
	int change = 0;
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	if (top->args->len == 0) {
		if (printDebug) {
			printf("Applying 'statements-empty' rule\n");
		}
		change = 1;
		trimK();
	} else if (top->args->len == 1) {
		if (printDebug) {
			printf("Applying 'statements-one' rule\n");
		}
		change = 1;
		K* newTop = top->args->a[0];
		setHead(newTop);
	} else {
		if (printDebug) {
			printf("Applying 'statements-many' rule\n");
		}
		change = 1;
		appendK(top->args->a[0]);
		K* newPreHead = updateTrimArgs(top, 1, top->args->len);
		setPreHead(newPreHead);
	}

	return change;
}

int handleVar() {
	int change = 0;
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	if ((top->args->len) == 0) {
		if (printDebug) {
			printf("Applying 'var-empty' rule\n");
		}
		change = 1;
		trimK();
	} else {
		if (printDebug) {
			printf("Applying 'var-something' rule\n");
		}
		change = 1;
		updateStore(Inner(Inner(top)), k_zero());
		K* newTop = updateTrimArgs(top, 1, top->args->len);
		setHead(newTop);
	}
	return change;
}

int handleAssign() {
	int change = 0;
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	K* left = top->args->a[0];
	K* right = top->args->a[1];
	if (!isValue(right)) {
		if (printDebug) { 
			printf("Applying ':=-heat' rule\n");
		}
		change = 1;
		appendK(right);
		K* newTop = UpdateArg(top, 1, Hole());
		setPreHead(newTop);
	} else {
		if (printDebug) { 
			printf("Applying 'assign-rule' rule\n");
		}
		change = 1;
		updateStore(Inner(left), right);
		trimK();
	}
	return change;
}

int handleWhile() {
	int change = 0;
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	if (printDebug) { 
		printf("Applying 'while' rule\n");
	}
	change = 1;
	K* guard = top->args->a[0];
	K* body = top->args->a[1];
	K* then = NewK(SymbolLabel(label_statements), newArgs(2, body, top));
	K* theIf = NewK(SymbolLabel(label_if), newArgs(3, guard, then, k_skip()));
	setHead(theIf);

	return change;
}

int handleIf() {
	int change = 0;
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	K* guard = top->args->a[0];
	if (!isValue(guard)) {
		if (printDebug) {
			printf("Applying 'if-heat' rule\n");
		}
		change = 1;
		appendK(guard);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(newTop);
	} else {
		if (Inner(guard)->label->type != e_symbol) {
			panic("Expected key to be symbol label");
		}
		if (Inner(guard)->label->symbol_val == label_true) {
			if (printDebug) {
				printf("Applying 'if-true' rule\n");
			}
			change = 1;
			setHead(top->args->a[1]);
		} else if (Inner(guard)->label->symbol_val == label_false) {
			if (printDebug) {
				printf("Applying 'if-false' rule\n");
			}
			change = 1;
			setHead(top->args->a[2]);
		}
	}

	return change;
}

int handleNot() {
	int change = 0;
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	K* body = Inner(top);
	if (!isValue(body)) {
		if (printDebug) {
			printf("Applying 'not-heat' rule\n");
		}
		change = 1;
		appendK(body);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(newTop);
	} else {
		if (Inner(Inner(top))->label->type != e_symbol) {
			panic("Expected key to be symbol label");
		}
		if (Inner(Inner(top))->label->symbol_val == label_false) {
			if (printDebug) { 
				printf("Applying 'not-false' rule\n");
			}
			change = 1;
			setHead(k_true());
		} else if (Inner(Inner(top))->label->symbol_val == label_true) {
			if (printDebug) {
				printf("Applying 'not-true' rule\n");
			}
			change = 1;
			setHead(k_false());
		}
	}

	return change;
}


int handleLTE() {
	int change = 0;
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	K* left = top->args->a[0];
	K* right = top->args->a[1];
	if (!isValue(left)) {
		if (printDebug) { printf("Applying '<=-heat-left' rule\n"); }
		change = 1;
		appendK(left);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(newTop);
	} else if (!isValue(right)) {
		if (printDebug) { printf("Applying '<=-heat-right' rule\n"); }
		change = 1;
		appendK(right);
		K* newTop = UpdateArg(top, 1, Hole());	
		setPreHead(newTop);
	} else {
		if (printDebug) { printf("Applying '<=' rule\n"); }
		change = 1;
		int64_t leftv = Inner(left)->label->i64_val;
		int64_t rightv = Inner(right)->label->i64_val;
		if (leftv <= rightv) {
			setHead(k_true());
		} else {
			setHead(k_false());
		}
	}

	return change;
}

int handlePlus() {
	int change = 0;
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	K* left = top->args->a[0];
	K* right = top->args->a[1];
	if (!isValue(left)) {
		if (printDebug) { printf("Applying '+-heat-left' rule\n"); }
		change = 1;
		appendK(left);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(newTop);
	} else if (!isValue(right)) {
		if (printDebug) { printf("Applying '+-heat-right' rule\n"); }
		change = 1;
		appendK(right);
		K* newTop = UpdateArg(top, 1, Hole());
		setPreHead(newTop);
	} else {
		if (printDebug) { printf("Applying '+' rule\n"); }
		change = 1;
		int64_t leftv = Inner(left)->label->i64_val;
		int64_t rightv = Inner(right)->label->i64_val;
		K* newTop = NewK(SymbolLabel(label_int), newArgs(1, NewK(Int64Label(leftv + rightv), NULL)));
		setHead(newTop);
	}

	return change;
}

int handleNeg() {
	int change = 0;
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	K* body = Inner(top);
	if (!isValue(body)) {
		if (printDebug) { printf("Applying 'neg-heat' rule\n"); }
		change = 1;
		appendK(body);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(newTop);
	} else {
		if (printDebug) { printf("Applying 'neg' rule\n"); }
		change = 1;
		int64_t value = Inner(body)->label->i64_val;
		int64_t newValue = -value;
		K* newTop = NewK(SymbolLabel(label_int), newArgs(1, NewK(Int64Label(newValue), NULL)));
		setHead(newTop);
	}

	return change;
}

int handleSkip() {
	int change = 0;

	if (printDebug) { printf("Applying 'skip' rule\n"); }
	change = 1;
	trimK();

	return change;
}

void repl() {
	int change = 1;
	while (change) {
		change = 0;
		if (printDebug) {
			printf(stateString());
			printf("\n-----------------\n");
		}
		if (next == 0) {
			break;
		}
		if (next > 10) {
			printf(stateString());
			printf("\n-----------------\n");
			panic("Safety check!");
		}
		// if (shouldCheck) {
		// 	check(kCell, stateCell);
		// }
		int topSpot = next - 1;
		K* top = kCell[topSpot];
		if (top->label->type != e_symbol) {
			panic("Expected a symbol label");
		}
		int topLabel = top->label->symbol_val;
		// printf("label: %s\n", topLabel);

		if (isValue(top)) {
			change = handleValue();
		} else if (topLabel == label_id) {
			change = handleVariable();
		} else if (topLabel == label_statements) {
			change = handleStatements();
		} else if (topLabel == label_var) {
			change = handleVar();
		} else if (topLabel == label_assign) {
			change = handleAssign();
		} else if (topLabel == label_while) {
			change = handleWhile();
		} else if (topLabel == label_if) {
			change = handleIf();
		} else if (topLabel == label_not) {
			change = handleNot();
		} else if (topLabel == label_lte) {
			change = handleLTE();
		} else if (topLabel == label_plus) {
			change = handlePlus();
		} else if (topLabel == label_div) {
			panic("don't handle Div");
		} else if (topLabel == label_neg) {
			change = handleNeg();
		} else if (topLabel == label_program) {
			panic("don't handle Program");
		} else if (topLabel == label_skip) {
			change = handleSkip();
		} else {
			panic("unrecognized label");
		}

		// switch {
		// 	case topLabel.Equals(label_paren): change = handleParen()
		// }
	}
}

int main(void) {
	for (int i = 0; i < 26; i++) {
		stateCell[i] = NULL;
	}
	// printf("%s\n", stateString());
	// stateCell['r' - 'a'] = k_one();
	// printf("%s\n", stateString());
	K* prog = prog1();
	appendK(prog);
	// printf("%s\n", KToString(prog));

	repl();
	if (stateCell['s' - 'a'] != NULL) {
		int64_t result = Inner(stateCell['s' - 'a'])->label->i64_val;
		printf("Result: %" PRId64 "\n", result);
	} else {
		printf("'s' was not set!\n");
	}

	return 0;
}
