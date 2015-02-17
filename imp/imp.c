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
#include "adopt.h"

typedef struct {
	StateCell* state;
	ComputationCell* k;
} Configuration;

uint64_t rewrites;

adopt_spec opt_specs[] = {
    // { ADOPT_VALUE, "debug", 'd', NULL, "displays debug information" },
    { ADOPT_VALUE, "input", 'i', NULL, "input for program" },
    { ADOPT_SWITCH, "help", 0, NULL, NULL, ADOPT_USAGE_HIDDEN },
    // { ADOPT_VALUE, "verbose", 'v', "level", "sets the verbosity level (default 1)" },
    // { ADOPT_VALUE, "channel", 'c', "channel", "sets the channel", ADOPT_USAGE_VALUE_REQUIRED },
    { ADOPT_LITERAL },
    { ADOPT_ARG, "file", 'f', NULL, "file path" },
    { 0 },
};

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

void handleIf(Configuration* config, int* change);

K* k_skip() { return NewK(SymbolLabel(symbol_Skip), NULL); }
// K* k_new_id(char* s) {
// 	return NewK(SymbolLabel(symbol_id), newArgs(1, NewK(StringLabel(s), NULL)));
// }

K* prog1(uint64_t upto) {
	// K* n = k_new_id("n");
	// K* s = k_new_id("s");
	// K* hundred = new_builtin_int(upto);
	// K* zero = new_builtin_int(0);

	// K* l1 = NewK(SymbolLabel(symbol_var), newArgs(2, n, s));
	// K* l2 = NewK(SymbolLabel(symbol_assign), newArgs(2, n, hundred));
	// K* l3 = NewK(SymbolLabel(symbol_assign), newArgs(2, s, zero));

	// K* sPn = NewK(SymbolLabel(symbol_plus), newArgs(2, s, n));
	// K* l5 = NewK(SymbolLabel(symbol_assign), newArgs(2, s, sPn));
	// K* negOne = NewK(SymbolLabel(symbol_neg), newArgs(1, new_builtin_int(1)));
	// K* nPno = NewK(SymbolLabel(symbol_plus), newArgs(2, n, negOne));
	// K* l6 = NewK(SymbolLabel(symbol_assign), newArgs(2, n, nPno));
	// K* body = NewK(SymbolLabel(symbol_statements), newArgs(2, l5, l6));

	// K* nLTzero = NewK(SymbolLabel(symbol_lte), newArgs(2, n, zero));
	// K* guard = NewK(SymbolLabel(symbol_not), newArgs(1, nLTzero));
	// K* l4 = NewK(SymbolLabel(symbol_while), newArgs(2, guard, body));

	// K* pgm = NewK(SymbolLabel(symbol_statements), newArgs(4, l1, l2, l3, l4));

	K* hole_inp = new_builtin_int(upto);

	K* pgm = NewK(SymbolLabel(symbol_Program), newArgs(2, NewK(SymbolLabel(symbol_Statements), newArgs(4, NewK(SymbolLabel(symbol_Var), newArgs(3, NewK(SymbolLabel(symbol_Id), newArgs(1, new_builtin_string("n"))),NewK(SymbolLabel(symbol_Id), newArgs(1, new_builtin_string("s"))),NewK(SymbolLabel(symbol_Id), newArgs(1, new_builtin_string("r"))))),NewK(SymbolLabel(symbol_Assign), newArgs(2, NewK(SymbolLabel(symbol_Id), newArgs(1, new_builtin_string("n"))),hole_inp)),NewK(SymbolLabel(symbol_Assign), newArgs(2, NewK(SymbolLabel(symbol_Id), newArgs(1, new_builtin_string("s"))),new_builtin_int(0))),NewK(SymbolLabel(symbol_While), newArgs(2, NewK(SymbolLabel(symbol_Not), newArgs(1, NewK(SymbolLabel(symbol_Paren), newArgs(1, NewK(SymbolLabel(symbol_LTE), newArgs(2, NewK(SymbolLabel(symbol_Id), newArgs(1, new_builtin_string("n"))),new_builtin_int(0))))))),NewK(SymbolLabel(symbol_Statements), newArgs(2, NewK(SymbolLabel(symbol_Assign), newArgs(2, NewK(SymbolLabel(symbol_Id), newArgs(1, new_builtin_string("s"))),NewK(SymbolLabel(symbol_Plus), newArgs(2, NewK(SymbolLabel(symbol_Id), newArgs(1, new_builtin_string("s"))),NewK(SymbolLabel(symbol_Id), newArgs(1, new_builtin_string("n"))))))),NewK(SymbolLabel(symbol_Assign), newArgs(2, NewK(SymbolLabel(symbol_Id), newArgs(1, new_builtin_string("n"))),NewK(SymbolLabel(symbol_Plus), newArgs(2, NewK(SymbolLabel(symbol_Id), newArgs(1, new_builtin_string("n"))),NewK(SymbolLabel(symbol_Neg), newArgs(1, new_builtin_int(1))))))))))))),NewK(SymbolLabel(symbol_Id), newArgs(1, new_builtin_string("s")))));

	return pgm;
}


int isValue(K* k) {
	if (checkTypeSafety) {
		if (k->label->type != e_symbol) {
			panic("Expected calling isValue on symbol labels");
		}
	}

	if (is_int(k) || is_bool(k)) {
		return 1;
	} else {
		return 0;
	}
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
		if (is_hole(arg)){
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

void handleProgram(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	if (top->args->len == 2) {
		if (printDebug) {
			printf("Applying 'program' rule\n");
		}
		*change = 1;
		K* body = top->args->a[0];
		K* result = top->args->a[1];		
		appendK(config->k, body);		
		setPreHead(config->k, result);
	}
}
// TODO: commonality
void handleParen(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	if (top->args->len == 1) {
		if (printDebug) {
			printf("Applying 'paren' rule\n");
		}
		*change = 1;
		K* newTop = top->args->a[0];
		setHead(config->k, newTop);
	}
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
		updateStore(config->state, Inner(Inner(top)), new_builtin_int(0));
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
	K* then = NewK(SymbolLabel(symbol_Statements), newArgs(2, body, top));
	K* theIf = NewK(SymbolLabel(symbol_If), newArgs(3, guard, then, k_skip()));
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
		if (is_true(Inner(guard))) {
			if (printDebug) {
				printf("Applying 'if-true' rule\n");
			}
			*change = 1;
			setHead(config->k, top->args->a[1]);
		} else if (is_false(Inner(guard))) {
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
		if (is_false(Inner(Inner(top)))) {
			if (printDebug) { 
				printf("Applying 'not-false' rule\n");
			}
			*change = 1;
			setHead(config->k, k_true());

			// follows
			handleValue(config, change);
		} else if (is_true(Inner(Inner(top)))) {
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
		K* newTop = new_builtin_int(leftv + rightv);
		setHead(config->k, newTop);

		// follows
		handleValue(config, change);
	}
}


void handleDiv(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	K* left = top->args->a[0];
	K* right = top->args->a[1];
	if (!isValue(left)) {
		if (printDebug) { printf("Applying '/-heat-left' rule\n"); }
		*change = 1;
		appendK(config->k, left);
		K* newTop = UpdateArg(top, 0, Hole());
		setPreHead(config->k, newTop);
	} else if (!isValue(right)) {
		if (printDebug) { printf("Applying '/-heat-right' rule\n"); }
		*change = 1;
		appendK(config->k, right);
		K* newTop = UpdateArg(top, 1, Hole());
		setPreHead(config->k, newTop);
	} else {
		if (printDebug) { printf("Applying '/' rule\n"); }
		*change = 1;
		int64_t leftv = Inner(left)->label->i64_val;
		int64_t rightv = Inner(right)->label->i64_val;
		K* newTop = new_builtin_int(leftv / rightv);
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
		K* newTop = new_builtin_int(newValue);
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
	do {
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
				case symbol_Id:
					handleVariable(config, &change);
					break;
				case symbol_Statements:
					handleStatements(config, &change);
					break;
				case symbol_Var:
					handleVar(config, &change);
					break;
				case symbol_Assign:
					handleAssign(config, &change);
					break;
				case symbol_While:
					handleWhile(config, &change);
					break;
				case symbol_If:
					handleIf(config, &change);
					break;
				case symbol_Not:
					handleNot(config, &change);
					break;
				case symbol_LTE:
					handleLTE(config, &change);
					break;
				case symbol_Plus:
					handlePlus(config, &change);
					break;
				case symbol_Neg:
					handleNeg(config, &change);
					break;
				case symbol_Skip:
					handleSkip(config, &change);
					break;
				case symbol_Div:
					handleDiv(config, &change);
					break;
				case symbol_Program:
					handleProgram(config, &change);
					break;
				case symbol_Paren:
					handleParen(config, &change);
					break;
				default: 
					panic("unrecognized label");
			}
		}
	} while (change);
}

Configuration* reduce(K* k) {
	Configuration* config = malloc(sizeof(Configuration));
	config->state = newStateCell();
	config->k = newComputationCell();

	appendK(config->k, k);

	repl(config);
	return config;
}

int main(int argc, char* argv[]) {

	adopt_parser parser;
	adopt_opt opt;
	// const char *value;
	int upto = 100000;
	const char *path = NULL;

	label_helper lh;
	lh.count = sizeof(givenLabels) / sizeof(givenLabels[0]);
	lh.labels = &givenLabels[0];


	set_labels(sizeof(givenLabels) / sizeof(givenLabels[0]), givenLabels);


	adopt_parser_init(&parser, opt_specs, argv + 1, argc - 1);

	// uint64_t upto = 100000;
	// if (argv > 1) {
	// 	upto = strtoll(argc[1], NULL, 10); // FIXME: fishy
	// 	if (upto <= 0) {
	// 		panic("Argument passed on command line needs to be bigger than 1");
	// 	}
	// }

	while (adopt_parser_next(&opt, &parser)) {
		if (opt.spec) {
			printf("'%s' = ", opt.spec->name);
			printf("'%s'\n", opt.value);
			if (strcmp(opt.spec->name, "file") == 0) {
				path = opt.value;
				printf("Will load program file '%s'\n", path);
			}
			if (strcmp(opt.spec->name, "input") == 0) {
				upto = atoi(opt.value);
			}
		} else {
			fprintf(stderr, "Unknown option: %s\n", opt.value);
			adopt_usage_fprint(stderr, argv[0], opt_specs);
			return 129;
		}
	}
	FILE *file = stdin;
	if (path != NULL) {
		file = fopen(path, "r");

		if (file == NULL) {
			printf("Couldn't open %s\n", path);
			return 1;
		}
	}
	K* prog = aterm_file_to_k(file, lh, new_builtin_int(upto));
	fclose(file);

	// K* prog2 = prog1(upto);

	// printf("\n%s\n", KToString(prog));
	// printf("\n%s\n", KToString(prog2));

	Configuration* config = reduce(prog);
	K* resultK = get_result(config->k);
	int64_t result = Inner(resultK)->label->i64_val;
	printf("Result: %" PRId64 "\n", result);
	
	dump_garbage_info();

	printf("\nrewrites: %" PRIu64 "\n", rewrites);

	return 0;
}
