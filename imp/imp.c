#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>

#include "k.h"

#define UPTO (5000000)

#define panic(...) (_panic(__func__, __FILE__, __LINE__, __VA_ARGS__))

#define MAX_STATE 26
#define MAX_K 10
#define MAX_GARBAGE_KEPT 10000
#define MAX_GARBAGE_ARG_LEN 5

// when printing k terms, print the ref counts as well
#define printRefCounts 1

#define printDebug 0
#define shouldCheck 0

// technically not needed, but good to be safe
#define checkTypeSafety 0
#define checkRefCounting 0
#define checkGC 0
#define checkTermSize 0
#define checkStackSize 0

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
	"_fake",
};

#define symbol_hole 0
#define symbol_assign 1
#define symbol_bool 2
#define symbol_div 3
#define symbol_id 4
#define symbol_if 5
#define symbol_int 6
#define symbol_lte 7
#define symbol_neg 8
#define symbol_not 9
#define symbol_plus 10
#define symbol_program 11
#define symbol_skip 12
#define symbol_statements 13
#define symbol_var 14
#define symbol_while 15
#define symbol_true 16
#define symbol_false 17
#define symbol_fake 18


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
const char* ListKToString(ListK* args);

void handleIf(int* change);

KLabel* deadLabels[MAX_GARBAGE_KEPT];
int deadLabelLen = 0;

K* deadK[MAX_GARBAGE_KEPT];
int deadlen = 0;

ListK* deadLists[MAX_GARBAGE_ARG_LEN+1][MAX_GARBAGE_KEPT];
int deadListsLen[MAX_GARBAGE_ARG_LEN+1];


// KLabel Int64Label(int64 i64) {
// 	return (KLabel){type = e_i64, {i64_val = i64}};
// 	// return KLabel{kind: e_i64, data: i64}
// }

int mallocedLabels = 0;

KLabel* mallocKLabel() {
	mallocedLabels++;
	return (KLabel*)malloc(sizeof(KLabel));
}
KLabel* StringLabel(const char* s) {
	if (printDebug) { printf("Str DeadLabelLen: %d\n", deadLabelLen); }
	if (printDebug) { printf("Creating string label %s\n", s); }
	KLabel* newL;
	if (deadLabelLen > 0) {
		newL = deadLabels[deadLabelLen - 1];
		deadLabelLen--;
	} else {
		newL = mallocKLabel();
	}
	newL->type = e_string;
	newL->string_val = s;
	return newL;
}
KLabel* symbolLabels[50];
KLabel* SymbolLabel(int s) {
	if (symbolLabels[s] != NULL) {
		return symbolLabels[s];
	}
	if (printDebug) { printf("Sym DeadLabelLen: %d\n", deadLabelLen); }
	if (printDebug) { printf("Creating symbol label %s\n", givenLabels[s]); }
	KLabel* newL;
	if (deadLabelLen > 0) {
		newL = deadLabels[deadLabelLen - 1];
		deadLabelLen--;
	} else {
		newL = mallocKLabel();
	}
	newL->type = e_symbol;
	newL->symbol_val = s;
	symbolLabels[s] = newL;

	return newL;
}
// int intcount = 0;
KLabel* Int64Label(int64_t i64) {
	// intcount++;
	if (printDebug) { printf("Int DeadLabelLen: %d\n", deadLabelLen); }
	if (printDebug) { printf("Creating int label %" PRId64 "\n", i64); }
	KLabel* newL;
	if (deadLabelLen > 0) {
		newL = deadLabels[deadLabelLen - 1];
		deadLabelLen--;
	} else {
		newL = mallocKLabel();
	}
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

ListK* getDeadList(int reqLength) {
	// return NULL;
	if (reqLength > MAX_GARBAGE_ARG_LEN) {
		return NULL;
	}
	ListK** deadList = deadLists[reqLength];
	if (deadListsLen[reqLength] == 0) {
		if (printDebug) { printf("No dead stuff of length %d\n", reqLength); }
		return NULL;
	}

	ListK* ret = deadList[deadListsLen[reqLength] - 1];
	deadListsLen[reqLength]--;

	if (checkGC) {
		if (ret == NULL) {
			panic("Didn't expect ret to be nil");
		}
		if (ret->cap < reqLength) {
			panic("Expected list to be of cap %d, but it was %d instead", reqLength, ret->cap);
		}
	}
	ret->len = reqLength;

	if (printDebug) {
		printf("Returning dead list of length %d\n", reqLength);
	}
	return ret;
}

int mallocedArgs;
ListK* mallocArgs() {
	mallocedArgs++;
	return malloc(sizeof(ListK));
}
int malloced[10];
K** mallocArgsA(int count) {
	malloced[count]++;
	return malloc(sizeof(K*) * count);
}

ListK* newArgs(int count, ...) {
	ListK* args = getDeadList(count);
	if (args == NULL) {
		args = mallocArgs();
		args->a = mallocArgsA(count);
		args->cap = count;
		args->len = count;
	}
	if (checkGC) {
		if (count != args->len) {
			panic("Expected %d == %d\n", count, args->len);
		}
	}

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

int mallocedK = 0;
K* mallocK() {
	mallocedK++;
	return malloc(sizeof(K));
}

ListK* emptyArgs() {
	ListK* args = getDeadList(0);
	if (args == NULL) {
		args = mallocArgs();
		args->cap = 0;
		args->len = 0;
	}
	return args;
}

K* NewK(KLabel* label, ListK* args) {
	if (args == NULL) {
		args = emptyArgs();
	}
	for (int i = 0; i < args->len; i++) {
 		K* arg = args->a[i];
 		if (arg == NULL) {
 			panic("Didn't expect nil arg in NewK()");
 		}
 		Inc(arg);
 	}
	
	K* newK = NULL;
	if (deadlen > 0) {
		newK = deadK[deadlen - 1];
		deadlen--;
	} else {
		newK = mallocK();
	}
	newK->label = label;
	newK->args = args;
	newK->refs = 0;
	return newK;
}

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
		panic("Some unknown label type %d found", label->type);
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

K* k_true() { return NewK(SymbolLabel(symbol_bool), newArgs(1, NewK(SymbolLabel(symbol_true), NULL))); }
K* k_false() { return NewK(SymbolLabel(symbol_bool), newArgs(1, NewK(SymbolLabel(symbol_false), NULL))); }
K* k_zero() { return NewK(SymbolLabel(symbol_int), newArgs(1, NewK(Int64Label(0), NULL))); }
K* k_one() { return NewK(SymbolLabel(symbol_int), newArgs(1, NewK(Int64Label(1), NULL))); }
K* k_skip() { return NewK(SymbolLabel(symbol_skip), NULL); }

K* prog1() {
	K* n = NewK(SymbolLabel(symbol_id), newArgs(1, NewK(StringLabel("n"), NULL)));
	K* s = NewK(SymbolLabel(symbol_id), newArgs(1, NewK(StringLabel("s"), NULL)));
	K* hundred = NewK(SymbolLabel(symbol_int), newArgs(1, NewK(Int64Label(UPTO), NULL)));

	K* l1 = NewK(SymbolLabel(symbol_var), newArgs(2, n, s));
	K* l2 = NewK(SymbolLabel(symbol_assign), newArgs(2, n, hundred));
	K* l3 = NewK(SymbolLabel(symbol_assign), newArgs(2, s, k_zero()));

	K* sPn = NewK(SymbolLabel(symbol_plus), newArgs(2, s, n));
	K* l5 = NewK(SymbolLabel(symbol_assign), newArgs(2, s, sPn));
	K* negOne = NewK(SymbolLabel(symbol_neg), newArgs(1, k_one()));
	K* nPno = NewK(SymbolLabel(symbol_plus), newArgs(2, n, negOne));
	K* l6 = NewK(SymbolLabel(symbol_assign), newArgs(2, n, nPno));
	K* body = NewK(SymbolLabel(symbol_statements), newArgs(2, l5, l6));

	K* nLTzero = NewK(SymbolLabel(symbol_lte), newArgs(2, n, k_zero()));
	K* guard = NewK(SymbolLabel(symbol_not), newArgs(1, nLTzero));
	K* l4 = NewK(SymbolLabel(symbol_while), newArgs(2, guard, body));

	K* pgm = NewK(SymbolLabel(symbol_statements), newArgs(4, l1, l2, l3, l4));
	return pgm;
	// return l1;
}

int isValue(K* k) {
	if (checkTypeSafety) {
		if (k->label->type != e_symbol) {
			panic("Expected calling isValue on symbol labels");
		}
	}
	int val = k->label->symbol_val;
	if (val == symbol_int || val == symbol_bool) {
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
	return NewK(SymbolLabel(symbol_hole), NULL);
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

	if (checkTermSize) {
		if (k->args->len > 10) {
			panic("Sanity check failed!");
		}
	}
	for (int i = 0; i < k->args->len; i++) {
		K* arg = k->args->a[i];
		Dec(arg);
	}

	// lenkargs := len(k.args)
	

	int lenkargs = k->args->cap;

	// don't garbage collect the "builtins"
	// if (lenkargs == 0 && k == Hole) {
	// 	return
	// }

	if (lenkargs >= MAX_GARBAGE_ARG_LEN) {
		panic("MAX_GARBAGE_ARG_LEN is not enough");
	}
	// if (deadListsLen[lenkargs] >= MAX_GARBAGE_KEPT) {
	// 	panic("garbage overflow");
	// }
	if (lenkargs < MAX_GARBAGE_ARG_LEN && deadListsLen[lenkargs] < MAX_GARBAGE_KEPT) {
		deadLists[lenkargs][deadListsLen[lenkargs]] = k->args;
		deadListsLen[lenkargs]++;
		if (printDebug) { printf("Saving args\n"); }
	} else {
		mallocedArgs--;
		if (k->args->cap > 0) {
			malloced[k->args->cap]--;
			free(k->args->a);
		}
		if (printDebug) { printf("Freeing args\n"); }		
		free(k->args);
	}
	// if lenkargs < MAX_GARBAGE_ARG_LEN && lenkargs > 0 {
	// 	if len(deadLists[lenkargs-1]) < MAX_GARBAGE_KEPT {
	// 		deadLists[lenkargs-1] = append(deadLists[lenkargs-1], k.args)
	// 	}
	// }

	if (k->label->type != e_symbol) {
		if (deadLabelLen < MAX_GARBAGE_KEPT) {
			deadLabels[deadLabelLen++] = k->label;
			if (printDebug) { printf("Saving label\n"); }
		} else {
			if (printDebug) { printf("Freeing label\n"); }
			mallocedLabels--;
			free(k->label);
		}
	}
	if (deadlen < MAX_GARBAGE_KEPT) {
		if (printDebug) { printf("Saving k\n"); }
		deadK[deadlen++] = k;
	} else {
		if (printDebug) { printf("Freeing k\n"); }
		mallocedK--;
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
	if (checkRefCounting) {
		if (newRefs < 0) {
			panic("Term %s has fewer than 0 refs :(", KToString(k));
		}
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
	if (checkStackSize) {
		if (next >= MAX_K) {
			panic("Trying to add too many elements to the K Cell!");
		}
	}
	Inc(k);
	kCell[next] = k;
	next++;
}


ListK* copyArgs(ListK* oldArgs) {
	ListK* args = getDeadList(oldArgs->cap);
	if (args == NULL) {
		args = mallocArgs();
		args->cap = oldArgs->cap;
		args->len = oldArgs->len;
		args->a = mallocArgsA(oldArgs->cap);
	}

	for (int i = 0; i < oldArgs->len; i++) {
		args->a[i] = oldArgs->a[i];
	}

	return args;
}

KLabel* copyLabel(KLabel* l) {
	if (printDebug) { printf("Cpy DeadLabelLen: %d\n", deadLabelLen); }
	if (printDebug) { printf("Creating copy label %s\n", LabelToString(l)); }
	KLabel* newL;
	if (deadLabelLen > 0) {
		newL = deadLabels[deadLabelLen - 1];
		deadLabelLen--;
	} else {
		newL = mallocKLabel();
	}
	memcpy(newL, l, sizeof(KLabel));
	return newL;
}

K* copy(K* oldK) {
	ListK* newArgs = copyArgs(oldK->args);
	// K* k = NewK(copyLabel(oldK->label), newArgs);
	K* k = NewK(oldK->label, newArgs);
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
		// K* oldk = k;
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

static void handleValue(int* change) {
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	if (next == 1) {
		return;
	}

	K* next = kCell[topSpot - 1];
	for (int i = 0; i < next->args->len; i++) {
		K* arg = next->args->a[i];
		if (checkTypeSafety) {
			if (arg->label->type != e_symbol) {
				panic("Expected string type");
			}
		}
		if (arg->label->symbol_val == symbol_hole){
			if (printDebug) {
				printf("Applying 'cooling' rule\n");
			}
			*change = 1;
			K* newTop = UpdateArg(next, i, top);
			trimK();
			setHead(newTop);
			break;
		}
	}
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
	if (checkTypeSafety) {
		if (keyK->label->type != e_string) {
			panic("Expected key to be string label");
		}
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
void handleVariable(int* change) {
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	if (checkTypeSafety) {
		if (Inner(top)->label->type != e_string) {
			panic("Expected key to be string label");
		}
	}
	int variable = Inner(top)->label->string_val[0] - 'a';
	K* value = stateCell[variable];
	if (value == NULL) {
		panic("Trying to read unassigned variable");
	}

	if (printDebug) { printf("Applying 'lookup' rule\n"); }
	*change = 1;
	setHead(value);


	// follows
	handleValue(change);
}

void handleStatements(int* change) {
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	if (top->args->len == 0) {
		if (printDebug) {
			printf("Applying 'statements-empty' rule\n");
		}
		*change = 1;
		trimK();
	} else if (top->args->len == 1) {
		if (printDebug) {
			printf("Applying 'statements-one' rule\n");
		}
		*change = 1;
		K* newTop = top->args->a[0];
		setHead(newTop);
	} else {
		if (printDebug) {
			printf("Applying 'statements-many' rule\n");
		}
		*change = 1;
		appendK(top->args->a[0]);
		K* newPreHead = updateTrimArgs(top, 1, top->args->len);
		setPreHead(newPreHead);
	}
}

void handleVar(int* change) {
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	if ((top->args->len) == 0) {
		if (printDebug) {
			printf("Applying 'var-empty' rule\n");
		}
		*change = 1;
		trimK();
	} else {
		if (printDebug) {
			printf("Applying 'var-something' rule\n");
		}
		*change = 1;
		updateStore(Inner(Inner(top)), k_zero());
		K* newTop = updateTrimArgs(top, 1, top->args->len);
		setHead(newTop);
	}
}

void handleAssign(int* change) {
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	K* left = top->args->a[0];
	K* right = top->args->a[1];
	if (!isValue(right)) {
		if (printDebug) { 
			printf("Applying ':=-heat' rule\n");
		}
		*change = 1;
		appendK(right);
		K* newTop = UpdateArg(top, 1, Hole());
		setPreHead(newTop);
	} else {
		if (printDebug) { 
			printf("Applying 'assign-rule' rule\n");
		}
		*change = 1;
		updateStore(Inner(left), right);
		trimK();
	}
}

void handleWhile(int* change) {
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	if (printDebug) { 
		printf("Applying 'while' rule\n");
	}
	*change = 1;
	K* guard = top->args->a[0];
	K* body = top->args->a[1];
	K* then = NewK(SymbolLabel(symbol_statements), newArgs(2, body, top));
	K* theIf = NewK(SymbolLabel(symbol_if), newArgs(3, guard, then, k_skip()));
	setHead(theIf);

	// follows
	handleIf(change);
}

void handleIf(int* change) {
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	K* guard = top->args->a[0];
	if (!isValue(guard)) {
		if (printDebug) {
			printf("Applying 'if-heat' rule\n");
		}
		*change = 1;
		appendK(guard);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(newTop);
	} else {
		if (checkTypeSafety) {
			if (Inner(guard)->label->type != e_symbol) {
				panic("Expected key to be symbol label");
			}
		}
		if (Inner(guard)->label->symbol_val == symbol_true) {
			if (printDebug) {
				printf("Applying 'if-true' rule\n");
			}
			*change = 1;
			setHead(top->args->a[1]);
		} else if (Inner(guard)->label->symbol_val == symbol_false) {
			if (printDebug) {
				printf("Applying 'if-false' rule\n");
			}
			*change = 1;
			setHead(top->args->a[2]);
		}
	}
}

void handleNot(int* change) {
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	K* body = Inner(top);
	if (!isValue(body)) {
		if (printDebug) {
			printf("Applying 'not-heat' rule\n");
		}
		*change = 1;
		appendK(body);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(newTop);
	} else {
		if (checkTypeSafety) {
			if (Inner(Inner(top))->label->type != e_symbol) {
				panic("Expected key to be symbol label");
			}
		}
		if (Inner(Inner(top))->label->symbol_val == symbol_false) {
			if (printDebug) { 
				printf("Applying 'not-false' rule\n");
			}
			*change = 1;
			setHead(k_true());

			// follows
			handleValue(change);
		} else if (Inner(Inner(top))->label->symbol_val == symbol_true) {
			if (printDebug) {
				printf("Applying 'not-true' rule\n");
			}
			*change = 1;
			setHead(k_false());

			// follows
			handleValue(change);
		}
	}
}


void handleLTE(int* change) {
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	K* left = top->args->a[0];
	K* right = top->args->a[1];
	if (!isValue(left)) {
		if (printDebug) { printf("Applying '<=-heat-left' rule\n"); }
		*change = 1;
		appendK(left);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(newTop);
	} else if (!isValue(right)) {
		if (printDebug) { printf("Applying '<=-heat-right' rule\n"); }
		*change = 1;
		appendK(right);
		K* newTop = UpdateArg(top, 1, Hole());	
		setPreHead(newTop);
	} else {
		if (printDebug) { printf("Applying '<=' rule\n"); }
		*change = 1;
		int64_t leftv = Inner(left)->label->i64_val;
		int64_t rightv = Inner(right)->label->i64_val;
		if (leftv <= rightv) {
			setHead(k_true());
		} else {
			setHead(k_false());
		}

		// follows
		handleValue(change);
	}
}

void handlePlus(int* change) {
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	K* left = top->args->a[0];
	K* right = top->args->a[1];
	if (!isValue(left)) {
		if (printDebug) { printf("Applying '+-heat-left' rule\n"); }
		*change = 1;
		appendK(left);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(newTop);
	} else if (!isValue(right)) {
		if (printDebug) { printf("Applying '+-heat-right' rule\n"); }
		*change = 1;
		appendK(right);
		K* newTop = UpdateArg(top, 1, Hole());
		setPreHead(newTop);
	} else {
		if (printDebug) { printf("Applying '+' rule\n"); }
		*change = 1;
		int64_t leftv = Inner(left)->label->i64_val;
		int64_t rightv = Inner(right)->label->i64_val;
		K* newTop = NewK(SymbolLabel(symbol_int), newArgs(1, NewK(Int64Label(leftv + rightv), NULL)));
		setHead(newTop);

		// follows
		handleValue(change);
	}
}

void handleNeg(int* change) {
	int topSpot = next - 1;
	K* top = kCell[topSpot];

	K* body = Inner(top);
	if (!isValue(body)) {
		if (printDebug) { printf("Applying 'neg-heat' rule\n"); }
		*change = 1;
		appendK(body);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(newTop);
	} else {
		if (printDebug) { printf("Applying 'neg' rule\n"); }
		*change = 1;
		int64_t value = Inner(body)->label->i64_val;
		int64_t newValue = -value;
		K* newTop = NewK(SymbolLabel(symbol_int), newArgs(1, NewK(Int64Label(newValue), NULL)));
		setHead(newTop);

		// follows
		handleValue(change);
	}
}

void handleSkip(int* change) {
	if (printDebug) { printf("Applying 'skip' rule\n"); }
	*change = 1;
	trimK();
}

typedef struct countentry {
	K* entry;
	int count;
} countentry;

void counts_aux(K* k, countentry counts[]) {
	int o = ((uintptr_t)k) % 1000000;
	// printf("o = %d\n", o);
	if (counts[o].entry == 0) {
		counts[o].entry = k;
		counts[o].count = 1;
	} else if (counts[o].entry == k) {
		counts[o].count++;
		return;
	} else {
		panic("Collision!");
	}
	for (int i = 0; i < k->args->len; i++) {
		K* arg = k->args->a[i];
		counts_aux(arg, counts);
	}
}

countentry* counts(K* k) {
	countentry* counts = calloc(1000000, sizeof(countentry));
	counts_aux(k, counts);
	return counts;
}

void check(K *c[MAX_K], K *state[MAX_STATE]) {
	ListK* allValues = malloc(sizeof(ListK));
	allValues->cap = next + 26;
	allValues->len = next;
	allValues->a = malloc(sizeof(K*) * (next + 26));
	for (int i = 0; i < next; i++) {
		allValues->a[i] = c[i];
	}
	// memcpy(allValues, c, sizeof(K*) * next);
	for (int i = 0; i < MAX_STATE; i++) {
		K* val = state[i];
		if (val == NULL) {
			continue;
		}
		allValues->a[allValues->len++] = val;
	}

	K* specialk = NewK(SymbolLabel(symbol_fake), allValues);
	for (int i = 0; i < specialk->args->len; i++) {
 		K* arg = specialk->args->a[i];
 		Dec(arg);
 	}
	specialk->refs = 1;
	countentry* cm = counts(specialk);

	int bad = 0;
	for (int i = 0; i < 1000000; i++) {
		if (cm[i].entry != 0) {
			K* k = cm[i].entry;
			if (k->refs != cm[i].count) {
				bad = 1;
				printf("Count for %s should be %d!\n", KToString(k), cm[i].count);
			}
		}
	}
	if (bad) { panic("Bad check()!"); }
	free(cm);
}

uint64_t rewrites;

void repl() {
	int change = 1;
	while (change) {
		rewrites++;
		change = 0;
		if (printDebug) {
			printf("%s", stateString());
			printf("\n-----------------\n");
		}
		if (next == 0) {
			break;
		}
		if (next > 10) {
			printf("%s", stateString());
			printf("\n-----------------\n");
			panic("Safety check!");
		}
		if (shouldCheck) {
			check(kCell, stateCell);
		}
		int topSpot = next - 1;
		K* top = kCell[topSpot];
		if (checkTypeSafety) {
			if (top->label->type != e_symbol) {
				panic("Expected a symbol label");
			}
		}
		int topLabel = top->label->symbol_val;
		// printf("label: %s\n", topLabel);

		if (isValue(top)) {
			handleValue(&change);
		} else {
			switch (topLabel) {
				case symbol_id:
					handleVariable(&change);
					break;
				case symbol_statements:
					handleStatements(&change);
					break;
				case symbol_var:
					handleVar(&change);
					break;
				case symbol_assign:
					handleAssign(&change);
					break;
				case symbol_while:
					handleWhile(&change);
					break;
				case symbol_if:
					handleIf(&change);
					break;
				case symbol_not:
					handleNot(&change);
					break;
				case symbol_lte:
					handleLTE(&change);
					break;
				case symbol_plus:
					handlePlus(&change);
					break;
				case symbol_neg:
					handleNeg(&change);
					break;
				case symbol_skip:
					handleSkip(&change);
					break;
				case symbol_div:
					panic("don't handle Div");
					break;
				case symbol_program:
					panic("don't handle Program");
					break;
				default: 
					panic("unrecognized label");
			}
		}


		// switch {
		// 	case topLabel.Equals(symbol_paren): change = handleParen()
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
	
	printf("\nMallocedK: %d\n", mallocedK);
	printf("deadlen: %d\n\n", deadlen);

	for (int i = 0; i < MAX_GARBAGE_ARG_LEN; i++) {
		for (int j = 0; j < deadListsLen[i]; j++) {
			ListK* args = deadLists[i][j];
			if (args->len > 0) {
				malloced[args->cap]--;
				free(args->a);
			}
			mallocedArgs--;
			free(args);
		}
	}
	for (int i = 0; i < MAX_GARBAGE_ARG_LEN; i++) {
		deadListsLen[i] = 0;
	}
	for (int i = 0; i < 8; i++) {
		printf("args %d: %d\n", i, malloced[i]);
	}
	for (int i = 0; i < MAX_GARBAGE_ARG_LEN; i++) {
		printf("deadargs %d: %d\n", i, deadListsLen[i]);
	}
	printf("Mallocedargs: %d\n\n", mallocedArgs);

	printf("MallocedLabels: %d\n", mallocedLabels);
	printf("deadLabelLen: %d\n", deadLabelLen);
	// printf("intcount: %d\n", intcount);

	printf("\nrewrites: %" PRId64 "\n", rewrites);
	return 0;
}
