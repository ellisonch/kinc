#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>

#include "lang.h"
#include "test.h"

typedef struct Configuration {
	StateCell* state;
	ComputationCell* k;
} Configuration;

// uint64_t rewrites;

char* givenLabels[] = {
	"Assign",
	"Div",
	"Id",
	"If",
	"LTE",
	"Neg",
	"Not",
	"Plus",
	"Program",
	"Skip",
	"Statements",
	"Var",
	"While",
	"Paren",
	"And",
	"Minus",
};

#define symbol_Assign 0
#define symbol_Div 1
#define symbol_Id 2
#define symbol_If 3
#define symbol_LTE 4
#define symbol_Neg 5
#define symbol_Not 6
#define symbol_Plus 7
#define symbol_Program 8
#define symbol_Skip 9
#define symbol_Statements 10
#define symbol_Var 11
#define symbol_While 12
#define symbol_Paren 13
#define symbol_And 14
#define symbol_Minus 15

void handleIf(Configuration* config, K* top, int* change);

int isValue(K* k) {
	assert(k != NULL);
	assert(k->label != NULL);
	assert(k->label->type == e_symbol);

	if (is_int(k) || is_bool(k)) {
		return 1;
	} else {
		return 0;
	}
}

static void handleValue(Configuration* config, K* top, int* change) {
	if (k_length(config->k) == 1) {
		return;
	}

	// K* top = k_get_item(config->k, 0);
	K* next = k_get_item(config->k, 1);

	for (int i = 0; i < k_num_args(next); i++) {
		K* arg = k_get_arg(next, i);
		if (checkTypeSafety) {
			if (arg->label->type != e_symbol) {
				panic("Expected symbol type in %s\nK is %s\n", KToString(next), kCellToString(config->k));
			}
		}
		if (is_hole(arg)) {
			if (printDebug) {
				printf("Applying 'cooling' rule\n");
			}
			*change = 1;
			K* newTop = k_replace_arg(next, i, arg, top);
			computation_remove_head(config->k);
			if (newTop != next) {
				computation_set_elem(config->k, 0, newTop);
			}
			break;
		}
	}
}


// TODO: unsafe
void handleVariable(Configuration* config, K* top, int* change) {
	// K* top = k_get_item(config->k, 0);

	K* value = state_get_item(config->state, k_get_arg(top, 0));
	if (value == NULL) {
		panic("Trying to read unassigned variable");
	}

	if (printDebug) { printf("Applying 'lookup' rule\n"); }
	*change = 1;
	computation_set_elem(config->k, 0, value);


	// follows
	handleValue(config, value, change);
}

void handleProgram(Configuration* config, K* top, int* change) {
	// K* top = k_get_item(config->k, 0);

	if (k_num_args(top) == 2) {
		if (printDebug) {
			printf("Applying 'program' rule\n");
		}
		*change = 1;
		K* body = k_get_arg(top, 0);
		K* result = k_get_arg(top, 1);		
		computation_add_front(config->k, body);		
		computation_set_elem(config->k, 1, result);
	}
}
// TODO: commonality
void handleParen(Configuration* config, K* top, int* change) {
	// K* top = k_get_item(config->k, 0);

	if (k_num_args(top) == 1) {
		if (printDebug) {
			printf("Applying 'paren' rule\n");
		}
		*change = 1;
		K* newTop = k_get_arg(top, 0);
		computation_set_elem(config->k, 0, newTop);
	}
}

void handleStatements(Configuration* config, K* top, int* change) {
	// K* top = k_get_item(config->k, 0);

	if (k_num_args(top) == 0) {
		if (printDebug) {
			printf("Applying 'statements-empty' rule\n");
		}
		*change = 1;
		computation_remove_head(config->k);
	} else if (k_num_args(top) == 1) {
		if (printDebug) {
			printf("Applying 'statements-one' rule\n");
		}
		*change = 1;
		K* newTop = k_get_arg(top, 0);
		computation_set_elem(config->k, 0, newTop);
	} else {
		if (printDebug) {
			printf("Applying 'statements-many' rule\n");
		}
		*change = 1;
		computation_add_front(config->k, k_get_arg(top, 0));
		K* newPreHead = updateTrimArgs(top, 1, k_num_args(top));
		computation_set_elem(config->k, 1, newPreHead);
	}
}

void handleVar(Configuration* config, K* top, int* change) {
	// K* top = k_get_item(config->k, 0);

	if (k_num_args(top) == 0) {
		if (printDebug) {
			printf("Applying 'var-empty' rule\n");
		}
		*change = 1;
		computation_remove_head(config->k);
	} else {
		if (printDebug) {
			printf("Applying 'var-something' rule\n");
		}
		*change = 1;
		updateStore(config->state, k_get_arg(k_get_arg(top, 0), 0), k_builtin_int_zero());
		K* newTop = updateTrimArgs(top, 1, k_num_args(top));
		computation_set_elem(config->k, 0, newTop);
	}
}

void handleAssign(Configuration* config, K* top, int* change) {
	// K* top = k_get_item(config->k, 0);

	K* left = k_get_arg(top, 0);
	K* right = k_get_arg(top, 1);
	if (!isValue(right)) {
		if (printDebug) { 
			printf("Applying ':=-heat' rule\n");
		}
		*change = 1;
		computation_add_front(config->k, right);
		K* newTop = k_replace_arg(top, 1, right, k_builtin_hole());
		computation_set_elem(config->k, 1, newTop);
	} else {
		if (printDebug) { 
			printf("Applying 'assign-rule' rule\n");
		}
		*change = 1;
		updateStore(config->state, k_get_arg(left, 0), right);
		computation_remove_head(config->k);
	}
}

void handleWhile(Configuration* config, K* top, int* change) {
	// K* top = k_get_item(config->k, 0);

	if (printDebug) {
		printf("Applying 'while' rule\n");
	}
	*change = 1;
	K* guard = k_get_arg(top, 0);
	K* body = k_get_arg(top, 1);
	K* then = k_new(SymbolLabel(symbol_Statements), 2, body, top);
	K* theIf = k_new(SymbolLabel(symbol_If), 3, guard, then, k_new_empty(SymbolLabel(symbol_Skip)));
	computation_set_elem(config->k, 0, theIf);

	// follows
	handleIf(config, theIf, change);
}

void handleIf(Configuration* config, K* top, int* change) {
	// K* top = k_get_item(config->k, 0);

	K* guard = k_get_arg(top, 0);
	if (!isValue(guard)) {
		if (printDebug) {
			printf("Applying 'if-heat' rule\n");
		}
		*change = 1;
		computation_add_front(config->k, guard);
		K* newTop = k_replace_arg(top, 0, guard, k_builtin_hole());
		computation_set_elem(config->k, 1, newTop);
	} else {
		if (checkTypeSafety) {
			if (k_get_arg(guard, 0)->label->type != e_symbol) {
				panic("Expected key to be symbol label.  Guard is %s\n", KToString(guard));
			}
		}
		if (is_true(k_get_arg(guard, 0))) {
			if (printDebug) {
				printf("Applying 'if-true' rule\n");
			}
			*change = 1;
			computation_set_elem(config->k, 0, k_get_arg(top, 1));
		} else if (is_false(k_get_arg(guard, 0))) {
			if (printDebug) {
				printf("Applying 'if-false' rule\n");
			}
			*change = 1;
			computation_set_elem(config->k, 0, k_get_arg(top, 2));
		}
	}
}

void handleNot(Configuration* config, K* top, int* change) {
	// K* top = k_get_item(config->k, 0);

	K* body = k_get_arg(top, 0);
	if (!isValue(body)) {
		if (printDebug) {
			printf("Applying 'not-heat' rule\n");
		}
		*change = 1;
		computation_add_front(config->k, body);
		K* newTop = k_replace_arg(top, 0, body, k_builtin_hole());
		computation_set_elem(config->k, 1, newTop);
	} else {
		if (checkTypeSafety) {
			if (k_get_arg(k_get_arg(top, 0), 0)->label->type != e_symbol) {
				panic("Expected key to be symbol label");
			}
		}
		if (is_false(k_get_arg(k_get_arg(top, 0), 0))) {
			if (printDebug) { 
				printf("Applying 'not-false' rule\n");
			}
			*change = 1;
			computation_set_elem(config->k, 0, k_builtin_true());

			// follows
			handleValue(config, k_builtin_true(), change);
		} else if (is_true(k_get_arg(k_get_arg(top, 0), 0))) {
			if (printDebug) {
				printf("Applying 'not-true' rule\n");
			}
			*change = 1;
			computation_set_elem(config->k, 0, k_builtin_false());

			// follows
			handleValue(config, k_builtin_false(), change);
		}
	}
}


void handleLTE(Configuration* config, K* top, int* change) {
	// K* top = k_get_item(config->k, 0);

	K* left = k_get_arg(top, 0);
	K* right = k_get_arg(top, 1);
	if (!isValue(left)) {
		if (printDebug) { printf("Applying '<=-heat-left' rule\n"); }
		*change = 1;
		computation_add_front(config->k, left);
		K* newTop = k_replace_arg(top, 0, left, k_builtin_hole());
		computation_set_elem(config->k, 1, newTop);
	} else if (!isValue(right)) {
		if (printDebug) { printf("Applying '<=-heat-right' rule\n"); }
		*change = 1;
		computation_add_front(config->k, right);
		K* newTop = k_replace_arg(top, 1, right, k_builtin_hole());	
		computation_set_elem(config->k, 1, newTop);
	} else {
		if (printDebug) { printf("Applying '<=' rule\n"); }
		*change = 1;
		int64_t leftv = k_get_arg(left, 0)->label->i64_val;
		int64_t rightv = k_get_arg(right, 0)->label->i64_val;
		K* new_head;
		if (leftv <= rightv) {
			new_head = k_builtin_true();
		} else {
			new_head = k_builtin_false();
		}
		computation_set_elem(config->k, 0, new_head);

		// follows
		handleValue(config, new_head, change);
	}
}

void handlePlus(Configuration* config, K* top, int* change) {
	// K* top = k_get_item(config->k, 0);

	K* left = k_get_arg(top, 0);
	K* right = k_get_arg(top, 1);
	if (!isValue(left)) {
		if (printDebug) { printf("Applying '+-heat-left' rule\n"); }
		*change = 1;
		computation_add_front(config->k, left);
		K* newTop = k_replace_arg(top, 0, left, k_builtin_hole());
		computation_set_elem(config->k, 1, newTop);
	} else if (!isValue(right)) {
		if (printDebug) { printf("Applying '+-heat-right' rule\n"); }
		*change = 1;
		computation_add_front(config->k, right);
		K* newTop = k_replace_arg(top, 1, right, k_builtin_hole());
		computation_set_elem(config->k, 1, newTop);
	} else {
		if (printDebug) { printf("Applying '+' rule\n"); }
		*change = 1;
		int64_t leftv = k_get_arg(left, 0)->label->i64_val;
		int64_t rightv = k_get_arg(right, 0)->label->i64_val;
		K* newTop = new_builtin_int(leftv + rightv);
		computation_set_elem(config->k, 0, newTop);

		// follows
		handleValue(config, newTop, change);
	}
}

void handleMinus(Configuration* config, K* top, int* change) {
	// K* top = k_get_item(config->k, 0);

	K* left = k_get_arg(top, 0);
	K* right = k_get_arg(top, 1);
	if (!isValue(left)) {
		if (printDebug) { printf("Applying '--heat-left' rule\n"); }
		*change = 1;
		computation_add_front(config->k, left);
		K* newTop = k_replace_arg(top, 0, left, k_builtin_hole());
		computation_set_elem(config->k, 1, newTop);
	} else if (!isValue(right)) {
		if (printDebug) { printf("Applying '--heat-right' rule\n"); }
		*change = 1;
		computation_add_front(config->k, right);
		K* newTop = k_replace_arg(top, 1, right, k_builtin_hole());
		computation_set_elem(config->k, 1, newTop);
	} else {
		if (printDebug) { printf("Applying '-' rule\n"); }
		*change = 1;
		int64_t leftv = k_get_arg(left, 0)->label->i64_val;
		int64_t rightv = k_get_arg(right, 0)->label->i64_val;
		K* newTop = new_builtin_int(leftv - rightv);
		computation_set_elem(config->k, 0, newTop);

		// follows
		handleValue(config, newTop, change);
	}
}

void handleDiv(Configuration* config, K* top, int* change) {
	// K* top = k_get_item(config->k, 0);

	K* left = k_get_arg(top, 0);
	K* right = k_get_arg(top, 1);
	if (!isValue(left)) {
		if (printDebug) { printf("Applying '/-heat-left' rule\n"); }
		*change = 1;
		computation_add_front(config->k, left);
		K* newTop = k_replace_arg(top, 0, left, k_builtin_hole());
		computation_set_elem(config->k, 1, newTop);
	} else if (!isValue(right)) {
		if (printDebug) { printf("Applying '/-heat-right' rule\n"); }
		*change = 1;
		computation_add_front(config->k, right);
		K* newTop = k_replace_arg(top, 1, right, k_builtin_hole());
		computation_set_elem(config->k, 1, newTop);
	} else {
		if (printDebug) { printf("Applying '/' rule\n"); }
		*change = 1;
		int64_t leftv = k_get_arg(left, 0)->label->i64_val;
		int64_t rightv = k_get_arg(right, 0)->label->i64_val;
		K* newTop = new_builtin_int(leftv / rightv);
		computation_set_elem(config->k, 0, newTop);

		// follows
		handleValue(config, newTop, change);
	}
}

void handleAnd(Configuration* config, K* top, int* change) {
	// K* top = k_get_item(config->k, 0);

	K* left = k_get_arg(top, 0);
	K* right = k_get_arg(top, 1);
	if (!isValue(left)) {
		if (printDebug) { printf("Applying '&&-heat-left' rule\n"); }
		*change = 1;
		computation_add_front(config->k, left);
		K* newTop = k_replace_arg(top, 0, left, k_builtin_hole());
		computation_set_elem(config->k, 1, newTop);
	} else if (!isValue(right)) {
		if (printDebug) { printf("Applying '&&-heat-right' rule\n"); }
		*change = 1;
		computation_add_front(config->k, right);
		K* newTop = k_replace_arg(top, 1, right, k_builtin_hole());
		computation_set_elem(config->k, 1, newTop);
	} else {
		if (printDebug) { printf("Applying '&&' rule\n"); }
		*change = 1;

		K* result;
		if (is_true(k_get_arg(left, 0)) && is_true(k_get_arg(right, 0))) {
			result = k_builtin_true();
		} else {
			result = k_builtin_false();
		}
		K* newTop = result;
		computation_set_elem(config->k, 0, newTop);

		// follows
		handleValue(config, newTop, change);
	}
}

void handleNeg(Configuration* config, K* top, int* change) {
	// K* top = k_get_item(config->k, 0);

	K* body = k_get_arg(top, 0);
	if (!isValue(body)) {
		if (printDebug) { printf("Applying 'neg-heat' rule\n"); }
		*change = 1;
		computation_add_front(config->k, body);
		K* newTop = k_replace_arg(top, 0, body, k_builtin_hole());
		computation_set_elem(config->k, 1, newTop);
	} else {
		if (printDebug) { printf("Applying 'neg' rule\n"); }
		*change = 1;
		int64_t value = k_get_arg(body, 0)->label->i64_val;
		int64_t newValue = -value;
		K* newTop = new_builtin_int(newValue);
		computation_set_elem(config->k, 0, newTop);

		// follows
		handleValue(config, newTop, change);
	}
}

void handleSkip(Configuration* config, K* top, int* change) {
	if (printDebug) { printf("Applying 'skip' rule\n"); }
	*change = 1;
	computation_remove_head(config->k);
}

int num_labels() {
	return sizeof(givenLabels) / sizeof(givenLabels[0]);
}
char** label_names() {
	return &givenLabels[0];
}

char* get_state_string(Configuration* config) {
    return stateString(config->k, config->state);
}


void repl(Configuration* config) {
	int change = 1;
	do {
		// rewrites++;
		// if (rewrites % 1000 == 0) {
		// 	double cap = garbage_get_capacity();
		// 	fprintf(stderr, "%f\n", cap);
		// }
		change = 0;
		if (printDebug) {
			char* ss = stateString(config->k, config->state);
			printf("%s", ss);
			printf("\n-----------------\n");
			free(ss);
		}
		if (k_length(config->k) == 0) {
			break;
		}
		if (k_length(config->k) >= 40) {
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
			handleValue(config, top, &change);
		} else {
			switch (topLabel) {
				case symbol_Id:
					handleVariable(config, top, &change);
					break;
				case symbol_Statements:
					handleStatements(config, top, &change);
					break;
				case symbol_Var:
					handleVar(config, top, &change);
					break;
				case symbol_Assign:
					handleAssign(config, top, &change);
					break;
				case symbol_While:
					handleWhile(config, top, &change);
					break;
				case symbol_If:
					handleIf(config, top, &change);
					break;
				case symbol_Not:
					handleNot(config, top, &change);
					break;
				case symbol_LTE:
					handleLTE(config, top, &change);
					break;
				case symbol_Plus:
					handlePlus(config, top, &change);
					break;
				case symbol_Minus:
					handleMinus(config, top, &change);
					break;
				case symbol_Neg:
					handleNeg(config, top, &change);
					break;
				case symbol_Skip:
					handleSkip(config, top, &change);
					break;
				case symbol_Div:
					handleDiv(config, top, &change);
					break;
				case symbol_And:
					handleAnd(config, top, &change);
					break;
				case symbol_Program:
					handleProgram(config, top, &change);
					break;
				case symbol_Paren:
					handleParen(config, top, &change);
					break;
				default: 
					panic("unrecognized label");
			}
		}
	} while (change);
}

Configuration* new_configuration(K* pgm) {
	Configuration* config = malloc(sizeof(Configuration));
	config->state = newStateCell();
	config->k = newComputationCell();

	computation_add_front(config->k, pgm);
	return config;
}

void k_language_init() {
}
