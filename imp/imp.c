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

typedef struct {
	StateCell* state;
	ComputationCell* k;
} Configuration;


uint64_t rewrites;


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

void handleIf(Configuration* config, int* change);

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



static void handleValue(Configuration* config, int* change) {
	if (k_length(config->k) == 1) {
		return;
	}

	K* top = k_get_item(config->k, 0);
	K* next = k_get_item(config->k, 1);

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
			trimK(config->k);
			setHead(config->k, newTop);
			break;
		}
	}
}


// TODO: unsafe
void handleVariable(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	if (checkTypeSafety) {
		if (Inner(top)->label->type != e_string) {
			panic("Expected key to be string label");
		}
	}
	K* value = state_get_item(config->state, Inner(top));
	if (value == NULL) {
		panic("Trying to read unassigned variable");
	}

	if (printDebug) { printf("Applying 'lookup' rule\n"); }
	*change = 1;
	setHead(config->k, value);


	// follows
	handleValue(config, change);
}

void handleStatements(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	if (top->args->len == 0) {
		if (printDebug) {
			printf("Applying 'statements-empty' rule\n");
		}
		*change = 1;
		trimK(config->k);
	} else if (top->args->len == 1) {
		if (printDebug) {
			printf("Applying 'statements-one' rule\n");
		}
		*change = 1;
		K* newTop = top->args->a[0];
		setHead(config->k, newTop);
	} else {
		if (printDebug) {
			printf("Applying 'statements-many' rule\n");
		}
		*change = 1;
		appendK(config->k, top->args->a[0]);
		K* newPreHead = updateTrimArgs(top, 1, top->args->len);
		setPreHead(config->k, newPreHead);
	}
}

void handleVar(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	if ((top->args->len) == 0) {
		if (printDebug) {
			printf("Applying 'var-empty' rule\n");
		}
		*change = 1;
		trimK(config->k);
	} else {
		if (printDebug) {
			printf("Applying 'var-something' rule\n");
		}
		*change = 1;
		updateStore(config->state, Inner(Inner(top)), k_zero());
		K* newTop = updateTrimArgs(top, 1, top->args->len);
		setHead(config->k, newTop);
	}
}

void handleAssign(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	K* left = top->args->a[0];
	K* right = top->args->a[1];
	if (!isValue(right)) {
		if (printDebug) { 
			printf("Applying ':=-heat' rule\n");
		}
		*change = 1;
		appendK(config->k, right);
		K* newTop = UpdateArg(top, 1, Hole());
		setPreHead(config->k, newTop);
	} else {
		if (printDebug) { 
			printf("Applying 'assign-rule' rule\n");
		}
		*change = 1;
		updateStore(config->state, Inner(left), right);
		trimK(config->k);
	}
}

void handleWhile(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	if (printDebug) { 
		printf("Applying 'while' rule\n");
	}
	*change = 1;
	K* guard = top->args->a[0];
	K* body = top->args->a[1];
	K* then = NewK(SymbolLabel(symbol_statements), newArgs(2, body, top));
	K* theIf = NewK(SymbolLabel(symbol_if), newArgs(3, guard, then, k_skip()));
	setHead(config->k, theIf);

	// follows
	handleIf(config, change);
}

void handleIf(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	K* guard = top->args->a[0];
	if (!isValue(guard)) {
		if (printDebug) {
			printf("Applying 'if-heat' rule\n");
		}
		*change = 1;
		appendK(config->k, guard);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(config->k, newTop);
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
			setHead(config->k, top->args->a[1]);
		} else if (Inner(guard)->label->symbol_val == symbol_false) {
			if (printDebug) {
				printf("Applying 'if-false' rule\n");
			}
			*change = 1;
			setHead(config->k, top->args->a[2]);
		}
	}
}

void handleNot(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	K* body = Inner(top);
	if (!isValue(body)) {
		if (printDebug) {
			printf("Applying 'not-heat' rule\n");
		}
		*change = 1;
		appendK(config->k, body);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(config->k, newTop);
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
			setHead(config->k, k_true());

			// follows
			handleValue(config, change);
		} else if (Inner(Inner(top))->label->symbol_val == symbol_true) {
			if (printDebug) {
				printf("Applying 'not-true' rule\n");
			}
			*change = 1;
			setHead(config->k, k_false());

			// follows
			handleValue(config, change);
		}
	}
}


void handleLTE(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	K* left = top->args->a[0];
	K* right = top->args->a[1];
	if (!isValue(left)) {
		if (printDebug) { printf("Applying '<=-heat-left' rule\n"); }
		*change = 1;
		appendK(config->k, left);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(config->k, newTop);
	} else if (!isValue(right)) {
		if (printDebug) { printf("Applying '<=-heat-right' rule\n"); }
		*change = 1;
		appendK(config->k, right);
		K* newTop = UpdateArg(top, 1, Hole());	
		setPreHead(config->k, newTop);
	} else {
		if (printDebug) { printf("Applying '<=' rule\n"); }
		*change = 1;
		int64_t leftv = Inner(left)->label->i64_val;
		int64_t rightv = Inner(right)->label->i64_val;
		if (leftv <= rightv) {
			setHead(config->k, k_true());
		} else {
			setHead(config->k, k_false());
		}

		// follows
		handleValue(config, change);
	}
}

void handlePlus(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	K* left = top->args->a[0];
	K* right = top->args->a[1];
	if (!isValue(left)) {
		if (printDebug) { printf("Applying '+-heat-left' rule\n"); }
		*change = 1;
		appendK(config->k, left);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(config->k, newTop);
	} else if (!isValue(right)) {
		if (printDebug) { printf("Applying '+-heat-right' rule\n"); }
		*change = 1;
		appendK(config->k, right);
		K* newTop = UpdateArg(top, 1, Hole());
		setPreHead(config->k, newTop);
	} else {
		if (printDebug) { printf("Applying '+' rule\n"); }
		*change = 1;
		int64_t leftv = Inner(left)->label->i64_val;
		int64_t rightv = Inner(right)->label->i64_val;
		K* newTop = NewK(SymbolLabel(symbol_int), newArgs(1, NewK(Int64Label(leftv + rightv), NULL)));
		setHead(config->k, newTop);

		// follows
		handleValue(config, change);
	}
}

void handleNeg(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	K* body = Inner(top);
	if (!isValue(body)) {
		if (printDebug) { printf("Applying 'neg-heat' rule\n"); }
		*change = 1;
		appendK(config->k, body);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(config->k, newTop);
	} else {
		if (printDebug) { printf("Applying 'neg' rule\n"); }
		*change = 1;
		int64_t value = Inner(body)->label->i64_val;
		int64_t newValue = -value;
		K* newTop = NewK(SymbolLabel(symbol_int), newArgs(1, NewK(Int64Label(newValue), NULL)));
		setHead(config->k, newTop);

		// follows
		handleValue(config, change);
	}
}

void handleSkip(Configuration* config, int* change) {
	if (printDebug) { printf("Applying 'skip' rule\n"); }
	*change = 1;
	trimK(config->k);
}

void repl(Configuration* config) {
	int change = 1;
	while (change) {
		rewrites++;
		change = 0;
		if (printDebug) {
			printf("%s", stateString(config->k, config->state));
			printf("\n-----------------\n");
		}
		if (k_length(config->k) == 0) {
			break;
		}
		if (k_length(config->k) > 10) {
			printf("%s", stateString(config->k, config->state));
			printf("\n-----------------\n");
			panic("Safety check!");
		}
		if (shouldCheck) {
			check(config->k, config->state);
		}
		K* top = k_get_item(config->k, 0);
		if (checkTypeSafety) {
			if (top->label->type != e_symbol) {
				panic("Expected a symbol label");
			}
		}
		int topLabel = top->label->symbol_val;
		// printf("label: %s\n", topLabel);

		if (isValue(top)) {
			handleValue(config, &change);
		} else {
			switch (topLabel) {
				case symbol_id:
					handleVariable(config, &change);
					break;
				case symbol_statements:
					handleStatements(config, &change);
					break;
				case symbol_var:
					handleVar(config, &change);
					break;
				case symbol_assign:
					handleAssign(config, &change);
					break;
				case symbol_while:
					handleWhile(config, &change);
					break;
				case symbol_if:
					handleIf(config, &change);
					break;
				case symbol_not:
					handleNot(config, &change);
					break;
				case symbol_lte:
					handleLTE(config, &change);
					break;
				case symbol_plus:
					handlePlus(config, &change);
					break;
				case symbol_neg:
					handleNeg(config, &change);
					break;
				case symbol_skip:
					handleSkip(config, &change);
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

Configuration* reduce(K* k) {
	Configuration* config = malloc(sizeof(Configuration));
	config->state = newStateCell();
	config->k = newComputationCell();

	appendK(config->k, k);

	repl(config);
	return config;
}


int main(void) {
	K* prog = prog1();

	Configuration* config = reduce(prog);

	// printf("%s\n", stateString());
	// stateCell['r' - 'a'] = k_one();
	// printf("%s\n", stateString());
	
	if (state_get_item_from_name(config->state, 's') != NULL) {
		int64_t result = Inner(state_get_item_from_name(config->state, 's'))->label->i64_val;
		printf("Result: %" PRId64 "\n", result);
	} else {
		printf("'s' was not set!\n");
	}
	
	dump_garbage_info();

	printf("\nrewrites: %" PRId64 "\n", rewrites);

	return 0;
}
