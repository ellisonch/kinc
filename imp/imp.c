#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>

#include "k.h"
#include "cells.h"
#include "utils.h"
#include "settings.h"

// #define UPTO (5000000)
#define UPTO (100000)

// TODO: get rid of these
extern int mallocedLabels;
extern int deadLabelLen;
extern KLabel* deadLabels[MAX_GARBAGE_KEPT];
extern int malloced[];
extern int mallocedArgs;
extern ListK* deadLists[MAX_GARBAGE_ARG_LEN+1][MAX_GARBAGE_KEPT];
extern int deadListsLen[MAX_GARBAGE_ARG_LEN+1];
extern int mallocedK;
extern int deadlen;


uint64_t rewrites;

StateCell* stateCell;

// K *kCell[MAX_K];
// int next = 0;

ComputationCell* kCell;

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

void handleIf(int* change);





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

K* Hole() {
	return NewK(SymbolLabel(symbol_hole), NULL);
}



static void handleValue(int* change) {
	if (k_length(kCell) == 1) {
		return;
	}

	K* top = k_get_item(kCell, 0);
	K* next = k_get_item(kCell, 1);

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
			trimK(kCell);
			setHead(kCell, newTop);
			break;
		}
	}
}


// TODO: unsafe
void handleVariable(int* change) {
	K* top = k_get_item(kCell, 0);

	if (checkTypeSafety) {
		if (Inner(top)->label->type != e_string) {
			panic("Expected key to be string label");
		}
	}
	K* value = state_get_item(stateCell, Inner(top));
	if (value == NULL) {
		panic("Trying to read unassigned variable");
	}

	if (printDebug) { printf("Applying 'lookup' rule\n"); }
	*change = 1;
	setHead(kCell, value);


	// follows
	handleValue(change);
}

void handleStatements(int* change) {
	K* top = k_get_item(kCell, 0);

	if (top->args->len == 0) {
		if (printDebug) {
			printf("Applying 'statements-empty' rule\n");
		}
		*change = 1;
		trimK(kCell);
	} else if (top->args->len == 1) {
		if (printDebug) {
			printf("Applying 'statements-one' rule\n");
		}
		*change = 1;
		K* newTop = top->args->a[0];
		setHead(kCell, newTop);
	} else {
		if (printDebug) {
			printf("Applying 'statements-many' rule\n");
		}
		*change = 1;
		appendK(kCell, top->args->a[0]);
		K* newPreHead = updateTrimArgs(top, 1, top->args->len);
		setPreHead(kCell, newPreHead);
	}
}

void handleVar(int* change) {
	K* top = k_get_item(kCell, 0);

	if ((top->args->len) == 0) {
		if (printDebug) {
			printf("Applying 'var-empty' rule\n");
		}
		*change = 1;
		trimK(kCell);
	} else {
		if (printDebug) {
			printf("Applying 'var-something' rule\n");
		}
		*change = 1;
		updateStore(stateCell, Inner(Inner(top)), k_zero());
		K* newTop = updateTrimArgs(top, 1, top->args->len);
		setHead(kCell, newTop);
	}
}

void handleAssign(int* change) {
	K* top = k_get_item(kCell, 0);

	K* left = top->args->a[0];
	K* right = top->args->a[1];
	if (!isValue(right)) {
		if (printDebug) { 
			printf("Applying ':=-heat' rule\n");
		}
		*change = 1;
		appendK(kCell, right);
		K* newTop = UpdateArg(top, 1, Hole());
		setPreHead(kCell, newTop);
	} else {
		if (printDebug) { 
			printf("Applying 'assign-rule' rule\n");
		}
		*change = 1;
		updateStore(stateCell, Inner(left), right);
		trimK(kCell);
	}
}

void handleWhile(int* change) {
	K* top = k_get_item(kCell, 0);

	if (printDebug) { 
		printf("Applying 'while' rule\n");
	}
	*change = 1;
	K* guard = top->args->a[0];
	K* body = top->args->a[1];
	K* then = NewK(SymbolLabel(symbol_statements), newArgs(2, body, top));
	K* theIf = NewK(SymbolLabel(symbol_if), newArgs(3, guard, then, k_skip()));
	setHead(kCell, theIf);

	// follows
	handleIf(change);
}

void handleIf(int* change) {
	K* top = k_get_item(kCell, 0);

	K* guard = top->args->a[0];
	if (!isValue(guard)) {
		if (printDebug) {
			printf("Applying 'if-heat' rule\n");
		}
		*change = 1;
		appendK(kCell, guard);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(kCell, newTop);
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
			setHead(kCell, top->args->a[1]);
		} else if (Inner(guard)->label->symbol_val == symbol_false) {
			if (printDebug) {
				printf("Applying 'if-false' rule\n");
			}
			*change = 1;
			setHead(kCell, top->args->a[2]);
		}
	}
}

void handleNot(int* change) {
	K* top = k_get_item(kCell, 0);

	K* body = Inner(top);
	if (!isValue(body)) {
		if (printDebug) {
			printf("Applying 'not-heat' rule\n");
		}
		*change = 1;
		appendK(kCell, body);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(kCell, newTop);
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
			setHead(kCell, k_true());

			// follows
			handleValue(change);
		} else if (Inner(Inner(top))->label->symbol_val == symbol_true) {
			if (printDebug) {
				printf("Applying 'not-true' rule\n");
			}
			*change = 1;
			setHead(kCell, k_false());

			// follows
			handleValue(change);
		}
	}
}


void handleLTE(int* change) {
	K* top = k_get_item(kCell, 0);

	K* left = top->args->a[0];
	K* right = top->args->a[1];
	if (!isValue(left)) {
		if (printDebug) { printf("Applying '<=-heat-left' rule\n"); }
		*change = 1;
		appendK(kCell, left);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(kCell, newTop);
	} else if (!isValue(right)) {
		if (printDebug) { printf("Applying '<=-heat-right' rule\n"); }
		*change = 1;
		appendK(kCell, right);
		K* newTop = UpdateArg(top, 1, Hole());	
		setPreHead(kCell, newTop);
	} else {
		if (printDebug) { printf("Applying '<=' rule\n"); }
		*change = 1;
		int64_t leftv = Inner(left)->label->i64_val;
		int64_t rightv = Inner(right)->label->i64_val;
		if (leftv <= rightv) {
			setHead(kCell, k_true());
		} else {
			setHead(kCell, k_false());
		}

		// follows
		handleValue(change);
	}
}

void handlePlus(int* change) {
	K* top = k_get_item(kCell, 0);

	K* left = top->args->a[0];
	K* right = top->args->a[1];
	if (!isValue(left)) {
		if (printDebug) { printf("Applying '+-heat-left' rule\n"); }
		*change = 1;
		appendK(kCell, left);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(kCell, newTop);
	} else if (!isValue(right)) {
		if (printDebug) { printf("Applying '+-heat-right' rule\n"); }
		*change = 1;
		appendK(kCell, right);
		K* newTop = UpdateArg(top, 1, Hole());
		setPreHead(kCell, newTop);
	} else {
		if (printDebug) { printf("Applying '+' rule\n"); }
		*change = 1;
		int64_t leftv = Inner(left)->label->i64_val;
		int64_t rightv = Inner(right)->label->i64_val;
		K* newTop = NewK(SymbolLabel(symbol_int), newArgs(1, NewK(Int64Label(leftv + rightv), NULL)));
		setHead(kCell, newTop);

		// follows
		handleValue(change);
	}
}

void handleNeg(int* change) {
	K* top = k_get_item(kCell, 0);

	K* body = Inner(top);
	if (!isValue(body)) {
		if (printDebug) { printf("Applying 'neg-heat' rule\n"); }
		*change = 1;
		appendK(kCell, body);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(kCell, newTop);
	} else {
		if (printDebug) { printf("Applying 'neg' rule\n"); }
		*change = 1;
		int64_t value = Inner(body)->label->i64_val;
		int64_t newValue = -value;
		K* newTop = NewK(SymbolLabel(symbol_int), newArgs(1, NewK(Int64Label(newValue), NULL)));
		setHead(kCell, newTop);

		// follows
		handleValue(change);
	}
}

void handleSkip(int* change) {
	if (printDebug) { printf("Applying 'skip' rule\n"); }
	*change = 1;
	trimK(kCell);
}

void repl() {
	int change = 1;
	while (change) {
		rewrites++;
		change = 0;
		if (printDebug) {
			printf("%s", stateString(kCell, stateCell));
			printf("\n-----------------\n");
		}
		if (k_length(kCell) == 0) {
			break;
		}
		if (k_length(kCell) > 10) {
			printf("%s", stateString(kCell, stateCell));
			printf("\n-----------------\n");
			panic("Safety check!");
		}
		if (shouldCheck) {
			check(kCell, stateCell);
		}
		K* top = k_get_item(kCell, 0);
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
	stateCell = newStateCell();
	kCell = newComputationCell();

	// printf("%s\n", stateString());
	// stateCell['r' - 'a'] = k_one();
	// printf("%s\n", stateString());
	K* prog = prog1();
	appendK(kCell, prog);
	// printf("%s\n", KToString(prog));

	repl();
	if (state_get_item_from_name(stateCell, 's') != NULL) {
		int64_t result = Inner(state_get_item_from_name(stateCell, 's'))->label->i64_val;
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
