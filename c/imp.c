#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>

#include "k.h"

#define UPTO (2)
#define PRINT_REF_COUNTS (0)

#define panic(...) (_panic(__func__, __FILE__, __LINE__, __VA_ARGS__))

#define MAX_STATE 26
#define MAX_K 10

#define printDebug 1
#define shouldCheck 1

// typedef struct K K;
// typedef struct ListK ListK;

typedef enum {
	e_string,
	e_i64,
} KLabelType;

// typedef struct K *ListK[];
typedef struct {
	KLabelType type;
	union {
		int64_t i64_val;
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



// KLabel Int64Label(int64 i64) {
// 	return (KLabel){type = e_i64, {i64_val = i64}};
// 	// return KLabel{kind: e_i64, data: i64}
// }
KLabel* StringLabel(const char* s) {
	KLabel* newL = (KLabel*)malloc(sizeof(KLabel));
	newL->type = e_string;
	newL->string_val = s;
	return newL;
	// return (KLabel){.type = e_string, {.string_val = s}};
}
KLabel* Int64Label(int64_t i64) {
	KLabel* newL = (KLabel*)malloc(sizeof(KLabel));
	newL->type = e_i64;
	newL->i64_val = i64;
	return newL;
	// return (KLabel){.type = e_string, {.string_val = s}};
}

_Noreturn void _panic(const char* func, const char* file, int line, const char* format, ...) {
    va_list va;
    va_start(va, format);
    fprintf(stderr, "PANIC! %s() (%s:%d): ", func, file, line);
    vfprintf(stderr, format, va);
    fprintf(stderr, "\n");
    exit(1);
}

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
K* NewK(KLabel* label, ListK* args) {
	if (args != NULL) {
		for (int i = 0; i < args->len; i++) {
	 		K* arg = args->a[i];
	 		if (arg == NULL) {
	 			panic("Didn't expect nil arg in NewK()");
	 		}
	 	}
	}
	K* newK = (K*)malloc(sizeof(K));
	// TODO handle dead stuff
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
	if (PRINT_REF_COUNTS) {
		snprintf(s, 300, "%s[%d](%s)", LabelToString(k->label), k->refs, ListKToString(k->args));
	} else {
		snprintf(s, 300, "%s(%s)", LabelToString(k->label), ListKToString(k->args));
	}
	return s;
}

K* k_true() { return NewK(StringLabel("Bool"), newArgs(1, NewK(StringLabel("True"), NULL))); }
K* k_false() { return NewK(StringLabel("Bool"), newArgs(1, NewK(StringLabel("False"), NULL))); }
K* k_zero() { return NewK(StringLabel("Int"), newArgs(1, NewK(Int64Label(0), NULL))); }
K* k_one() { return NewK(StringLabel("Int"), newArgs(1, NewK(Int64Label(1), NULL))); }
K* k_skip() { return NewK(StringLabel("Skip"), NULL); }

// Program(
// 	Statements(
// 		Var(Id("n"),Id("s"),Id("r"))
// 		,Assign(Id("n"), Int(1000000))
// 		,Assign(Id("s"), Int(0))
// 		,While(Not(Paren(LTE(Id("n"), Int(0)))), Statements(Assign(Id("s"), Plus(Id("s"), Id("n"))),Assign(Id("n"), Plus(Id("n"), Neg(Int(1))))))
// 	)
// )

K* prog1() {
	K* n = NewK(StringLabel("Id"), newArgs(1, NewK(StringLabel("n"), NULL)));
	K* s = NewK(StringLabel("Id"), newArgs(1, NewK(StringLabel("s"), NULL)));
	K* hundred = NewK(StringLabel("Int"), newArgs(1, NewK(Int64Label(UPTO), NULL)));

	ListK* args;

	K* l1 = NewK(StringLabel("Var"), newArgs(2, n, s));
	K* l2 = NewK(StringLabel("Assign"), newArgs(2, n, hundred));
	K* l3 = NewK(StringLabel("Assign"), newArgs(2, s, k_zero()));

	K* sPn = NewK(StringLabel("Plus"), newArgs(2, s, n));
	K* l5 = NewK(StringLabel("Assign"), newArgs(2, s, sPn));
	K* negOne = NewK(StringLabel("Neg"), newArgs(1, k_one()));
	K* nPno = NewK(StringLabel("Plus"), newArgs(2, n, negOne));
	K* l6 = NewK(StringLabel("Assign"), newArgs(2, n, nPno));
	K* body = NewK(StringLabel("Statements"), newArgs(2, l5, l6));

	K* nLTzero = NewK(StringLabel("LTE"), newArgs(2, n, k_zero()));
	K* guard = NewK(StringLabel("Not"), newArgs(1, nLTzero));
	K* l4 = NewK(StringLabel("While"), newArgs(2, guard, body));

	// K* pgm = NewK(StringLabel("Semi"), newArgs(2, l3, l4));
	// pgm = NewK(StringLabel("Semi"), newArgs(2, l2, pgm));
	// pgm = NewK(StringLabel("Semi"), newArgs(2, l1, pgm));
	K* pgm = NewK(StringLabel("Statements"), newArgs(4, l1, l2, l3, l4));
	return pgm;
	// return l1;
}

int isValue(K* k) {
	if (k->label->type != e_string) {
		panic("Expected calling isValue on string labels");
	}
	if (strcmp(k->label->string_val, "Int") == 0 || strcmp(k->label->string_val, "Bool") == 0) {
		return 1;
	} else {
		return 0;
	}
}

// var stateCell map[string]*K = make(map[string]*K)
K *stateCell[MAX_STATE];

K *kCell[MAX_K];
int next = 0;

// FIXME: leaks memory, sucks
char* kCellToString() {
	char* s = malloc(10000);
	strcpy(s, "k(\n");
	for (int i = 0; i < next; i++) {
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


// func trimK() {
// 	top := len(kCell)-1
// 	kCell[top].Dec()
// 	kCell = kCell[:top]
// }

// func setHead(k *K) {
// 	top := len(kCell)-1
// 	k.Inc()
// 	kCell[top].Dec()
// 	kCell[top] = k
// }
// func setPreHead(k *K) {
// 	pre := len(kCell)-2
// 	k.Inc()
// 	kCell[pre].Dec()
// 	kCell[pre] = k
// }

void inc(K* k) {
	k->refs++;
}

void appendK(K* k) {
	if (next >= MAX_K) {
		panic("Trying to add too many elements to the K Cell!");
	}
	inc(k);
	kCell[next] = k;
	next++;
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
		if (strcmp(arg->label->string_val, "_hole") == 0){
			if (printDebug) {
				printf("Applying 'cooling' rule\n");
			}
			change = 1;
			panic("didn't finish value");
		}
	}
	
	// for i, arg := range next.P_Args() {
	// 	if arg.P_Label() == Label_hole {
	// 		if printDebug { fmt.Printf("Applying 'cooling' rule\n") }
	// 		change = true
	// 		newTop := UpdateArg(next, i, top)
	// 		trimK()
	// 		setHead(newTop)
	// 		break
	// 	}
	// }
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
		if (top->label->type != e_string) {
			panic("Expected a string label");
		}
		const char* topLabel = top->label->string_val;
		printf("label: %s\n", topLabel);

		if (isValue(top)) {
			change = handleValue();
		} else if (strcmp(topLabel, "Id") == 0) {
			panic("don't handle program");
		} else if (strcmp(topLabel, "Statements") == 0) {
			panic("don't handle Statements");
		} else if (strcmp(topLabel, "Var") == 0) {
			panic("don't handle Var");
		} else if (strcmp(topLabel, "Assign") == 0) {
			panic("don't handle Assign");
		} else if (strcmp(topLabel, "While") == 0) {
			panic("don't handle While");
		} else if (strcmp(topLabel, "If") == 0) {
			panic("don't handle If");
		} else if (strcmp(topLabel, "Not") == 0) {
			panic("don't handle Not");
		} else if (strcmp(topLabel, "LTE") == 0) {
			panic("don't handle LTE");
		} else if (strcmp(topLabel, "Plus") == 0) {
			panic("don't handle Plus");
		} else if (strcmp(topLabel, "Div") == 0) {
			panic("don't handle Div");
		} else if (strcmp(topLabel, "Neg") == 0) {
			panic("don't handle Neg");
		} else if (strcmp(topLabel, "Program") == 0) {
			panic("don't handle Program");
		} else if (strcmp(topLabel, "Skip") == 0) {
			panic("don't handle Skip");
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
		int64_t result = stateCell['s' - 'a']->label->i64_val;
		printf("Result: %" PRId64 "\n", result);
	} else {
		printf("'s' was not set!\n");
	}

	return 0;
}
