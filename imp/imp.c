#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>

#include "k.h"
#include "cells.h"
#include "utils.h"
#include "settings.h"
#include "adopt.h"
#include "test.h"

typedef struct {
	StateCell* state;
	ComputationCell* k;
} Configuration;

uint64_t rewrites;

adopt_spec opt_specs[] = {
    // { ADOPT_VALUE, "debug", 'd', NULL, "displays debug information" },
    { ADOPT_VALUE, "input", 'i', NULL, "input for program" },
    { ADOPT_SWITCH, "help", 0, NULL, NULL, ADOPT_USAGE_HIDDEN },
    { ADOPT_SWITCH, "test", 't', NULL, "Turns testing on" },
    { ADOPT_SWITCH, "bench", 'b', NULL, "Turns benching on" },
    { ADOPT_SWITCH, "mem", 'm', NULL, "Turns mem test on" },
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

void handleIf(Configuration* config, int* change);

K* k_skip() { return k_new_empty(SymbolLabel(symbol_Skip)); }

int isValue(K* k) {
	assert(k != NULL);
	assert(k->label != NULL);

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

	for (int i = 0; i < k_num_args(next); i++) {
		K* arg = k_get_arg(next, i);
		if (checkTypeSafety) {
			if (arg->label->type != e_symbol) {
				panic("Expected string type in %s\nK is %s\n", KToString(next), kCellToString(config->k));
			}
		}
		if (is_hole(arg)) {
			if (printDebug) {
				printf("Applying 'cooling' rule\n");
			}
			*change = 1;
			K* newTop = k_replace_arg(next, i, top);
			computation_remove_head(config->k);
			computation_set_elem(config->k, 0, newTop);
			break;
		}
	}
}


// TODO: unsafe
void handleVariable(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	K* value = state_get_item(config->state, k_get_arg(top, 0));
	if (value == NULL) {
		panic("Trying to read unassigned variable");
	}

	if (printDebug) { printf("Applying 'lookup' rule\n"); }
	*change = 1;
	computation_set_elem(config->k, 0, value);


	// follows
	handleValue(config, change);
}

void handleProgram(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

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
void handleParen(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	if (k_num_args(top) == 1) {
		if (printDebug) {
			printf("Applying 'paren' rule\n");
		}
		*change = 1;
		K* newTop = k_get_arg(top, 0);
		computation_set_elem(config->k, 0, newTop);
	}
}

void handleStatements(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

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

void handleVar(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

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
		updateStore(config->state, k_get_arg(k_get_arg(top, 0), 0), new_builtin_int(0));
		K* newTop = updateTrimArgs(top, 1, k_num_args(top));
		computation_set_elem(config->k, 0, newTop);
	}
}

void handleAssign(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	K* left = k_get_arg(top, 0);
	K* right = k_get_arg(top, 1);
	if (!isValue(right)) {
		if (printDebug) { 
			printf("Applying ':=-heat' rule\n");
		}
		*change = 1;
		computation_add_front(config->k, right);
		K* newTop = k_replace_arg(top, 1, Hole());
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

void handleWhile(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	if (printDebug) {
		printf("Applying 'while' rule\n");
	}
	*change = 1;
	K* guard = k_get_arg(top, 0);
	K* body = k_get_arg(top, 1);
	K* then = k_new(SymbolLabel(symbol_Statements), 2, body, top);
	K* theIf = k_new(SymbolLabel(symbol_If), 3, guard, then, k_skip());
	computation_set_elem(config->k, 0, theIf);

	// follows
	handleIf(config, change);
}

void handleIf(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	K* guard = k_get_arg(top, 0);
	if (!isValue(guard)) {
		if (printDebug) {
			printf("Applying 'if-heat' rule\n");
		}
		*change = 1;
		computation_add_front(config->k, guard);
		K* newTop = k_replace_arg(top, 0, Hole());
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

void handleNot(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	K* body = k_get_arg(top, 0);
	if (!isValue(body)) {
		if (printDebug) {
			printf("Applying 'not-heat' rule\n");
		}
		*change = 1;
		computation_add_front(config->k, body);
		K* newTop = k_replace_arg(top, 0, Hole());
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
			computation_set_elem(config->k, 0, k_true());

			// follows
			handleValue(config, change);
		} else if (is_true(k_get_arg(k_get_arg(top, 0), 0))) {
			if (printDebug) {
				printf("Applying 'not-true' rule\n");
			}
			*change = 1;
			computation_set_elem(config->k, 0, k_false());

			// follows
			handleValue(config, change);
		}
	}
}


void handleLTE(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	K* left = k_get_arg(top, 0);
	K* right = k_get_arg(top, 1);
	if (!isValue(left)) {
		if (printDebug) { printf("Applying '<=-heat-left' rule\n"); }
		*change = 1;
		computation_add_front(config->k, left);
		K* newTop = k_replace_arg(top, 0, Hole());
		computation_set_elem(config->k, 1, newTop);
	} else if (!isValue(right)) {
		if (printDebug) { printf("Applying '<=-heat-right' rule\n"); }
		*change = 1;
		computation_add_front(config->k, right);
		K* newTop = k_replace_arg(top, 1, Hole());	
		computation_set_elem(config->k, 1, newTop);
	} else {
		if (printDebug) { printf("Applying '<=' rule\n"); }
		*change = 1;
		int64_t leftv = k_get_arg(left, 0)->label->i64_val;
		int64_t rightv = k_get_arg(right, 0)->label->i64_val;
		if (leftv <= rightv) {
			computation_set_elem(config->k, 0, k_true());
		} else {
			computation_set_elem(config->k, 0, k_false());
		}

		// follows
		handleValue(config, change);
	}
}

void handlePlus(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	K* left = k_get_arg(top, 0);
	K* right = k_get_arg(top, 1);
	if (!isValue(left)) {
		if (printDebug) { printf("Applying '+-heat-left' rule\n"); }
		*change = 1;
		computation_add_front(config->k, left);
		K* newTop = k_replace_arg(top, 0, Hole());
		computation_set_elem(config->k, 1, newTop);
	} else if (!isValue(right)) {
		if (printDebug) { printf("Applying '+-heat-right' rule\n"); }
		*change = 1;
		computation_add_front(config->k, right);
		K* newTop = k_replace_arg(top, 1, Hole());
		computation_set_elem(config->k, 1, newTop);
	} else {
		if (printDebug) { printf("Applying '+' rule\n"); }
		*change = 1;
		int64_t leftv = k_get_arg(left, 0)->label->i64_val;
		int64_t rightv = k_get_arg(right, 0)->label->i64_val;
		K* newTop = new_builtin_int(leftv + rightv);
		computation_set_elem(config->k, 0, newTop);

		// follows
		handleValue(config, change);
	}
}

void handleMinus(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	K* left = k_get_arg(top, 0);
	K* right = k_get_arg(top, 1);
	if (!isValue(left)) {
		if (printDebug) { printf("Applying '--heat-left' rule\n"); }
		*change = 1;
		computation_add_front(config->k, left);
		K* newTop = k_replace_arg(top, 0, Hole());
		computation_set_elem(config->k, 1, newTop);
	} else if (!isValue(right)) {
		if (printDebug) { printf("Applying '--heat-right' rule\n"); }
		*change = 1;
		computation_add_front(config->k, right);
		K* newTop = k_replace_arg(top, 1, Hole());
		computation_set_elem(config->k, 1, newTop);
	} else {
		if (printDebug) { printf("Applying '-' rule\n"); }
		*change = 1;
		int64_t leftv = k_get_arg(left, 0)->label->i64_val;
		int64_t rightv = k_get_arg(right, 0)->label->i64_val;
		K* newTop = new_builtin_int(leftv - rightv);
		computation_set_elem(config->k, 0, newTop);

		// follows
		handleValue(config, change);
	}
}

void handleDiv(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	K* left = k_get_arg(top, 0);
	K* right = k_get_arg(top, 1);
	if (!isValue(left)) {
		if (printDebug) { printf("Applying '/-heat-left' rule\n"); }
		*change = 1;
		computation_add_front(config->k, left);
		K* newTop = k_replace_arg(top, 0, Hole());
		computation_set_elem(config->k, 1, newTop);
	} else if (!isValue(right)) {
		if (printDebug) { printf("Applying '/-heat-right' rule\n"); }
		*change = 1;
		computation_add_front(config->k, right);
		K* newTop = k_replace_arg(top, 1, Hole());
		computation_set_elem(config->k, 1, newTop);
	} else {
		if (printDebug) { printf("Applying '/' rule\n"); }
		*change = 1;
		int64_t leftv = k_get_arg(left, 0)->label->i64_val;
		int64_t rightv = k_get_arg(right, 0)->label->i64_val;
		K* newTop = new_builtin_int(leftv / rightv);
		computation_set_elem(config->k, 0, newTop);

		// follows
		handleValue(config, change);
	}
}

void handleAnd(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	K* left = k_get_arg(top, 0);
	K* right = k_get_arg(top, 1);
	if (!isValue(left)) {
		if (printDebug) { printf("Applying '&&-heat-left' rule\n"); }
		*change = 1;
		computation_add_front(config->k, left);
		K* newTop = k_replace_arg(top, 0, Hole());
		computation_set_elem(config->k, 1, newTop);
	} else if (!isValue(right)) {
		if (printDebug) { printf("Applying '&&-heat-right' rule\n"); }
		*change = 1;
		computation_add_front(config->k, right);
		K* newTop = k_replace_arg(top, 1, Hole());
		computation_set_elem(config->k, 1, newTop);
	} else {
		if (printDebug) { printf("Applying '&&' rule\n"); }
		*change = 1;

		K* result;
		if (is_true(k_get_arg(left, 0)) && is_true(k_get_arg(right, 0))) {
			result = k_true();
		} else {
			result = k_false();
		}
		K* newTop = result;
		computation_set_elem(config->k, 0, newTop);

		// follows
		handleValue(config, change);
	}
}

void handleNeg(Configuration* config, int* change) {
	K* top = k_get_item(config->k, 0);

	K* body = k_get_arg(top, 0);
	if (!isValue(body)) {
		if (printDebug) { printf("Applying 'neg-heat' rule\n"); }
		*change = 1;
		computation_add_front(config->k, body);
		K* newTop = k_replace_arg(top, 0, Hole());
		computation_set_elem(config->k, 1, newTop);
	} else {
		if (printDebug) { printf("Applying 'neg' rule\n"); }
		*change = 1;
		int64_t value = k_get_arg(body, 0)->label->i64_val;
		int64_t newValue = -value;
		K* newTop = new_builtin_int(newValue);
		computation_set_elem(config->k, 0, newTop);

		// follows
		handleValue(config, change);
	}
}

void handleSkip(Configuration* config, int* change) {
	if (printDebug) { printf("Applying 'skip' rule\n"); }
	*change = 1;
	computation_remove_head(config->k);
}

void repl(Configuration* config) {
	int change = 1;
	do {
		rewrites++;
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
		if (k_length(config->k) > MAX_K) {
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
				case symbol_Minus:
					handleMinus(config, &change);
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
				case symbol_And:
					handleAnd(config, &change);
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

	computation_add_front(config->k, k);
	
	// char* ss = stateString(config->k, config->state);
	// printf("%s\n", ss);
	// free(ss);

	repl(config);
	return config;
}

uint64_t run(const char* path, int64_t upto) {
	label_helper lh;
	lh.count = sizeof(givenLabels) / sizeof(givenLabels[0]);
	lh.labels = &givenLabels[0];

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

	Configuration* config = reduce(prog);
	K* resultK = get_result(config->k);
	int64_t result = k_get_arg(resultK, 0)->label->i64_val;

	char* ss = stateString(config->k, config->state);
	printf("%s\n", ss);
	free(ss);

	check(config->k, config->state);

	computation_cleanup(config->k);
	state_cleanup(config->state);

	free(config->k);
	free(config->state);
	free(config);

	return result;
}

int main(int argc, char* argv[]) {
	setvbuf(stdout, NULL, _IONBF, 0);
	adopt_parser parser;
	adopt_opt opt;
	// const char *value;
	int upto = 5;
	const char *path = NULL;
	int test = 0;
	int bench = 0;
	int mem = 0;

	k_init();

	// for (int i = 0; i < 3000; i += 10) {
	// 	printf("pow(%d) = %d; log(%d) = %d\n", i, next_highest_power(i), i, ceil_log2(i));
	// }
	// return;

	set_labels(sizeof(givenLabels) / sizeof(givenLabels[0]), givenLabels);

	adopt_parser_init(&parser, opt_specs, argv + 1, argc - 1);

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
			if (strcmp(opt.spec->name, "test") == 0) {
				test = 1;
			}
			if (strcmp(opt.spec->name, "bench") == 0) {
				bench = 1;
			}
			if (strcmp(opt.spec->name, "mem") == 0) {
				mem = 1;
			}
		} else {
			fprintf(stderr, "Unknown option: %s\n", opt.value);
			adopt_usage_fprint(stderr, argv[0], opt_specs);
			return 129;
		}
	}
	
	if (bench) {
		run_bench();
		dump_garbage_info();
	} else if (mem) {
		run_mem();
		dump_garbage_info();
	} else if (test) {
		run_tests();
		dump_garbage_info();
	} else {
		uint64_t result = run(path, upto);
		printf("Result: %" PRId64 "\n", result);
	
		dump_garbage_info();

		printf("\nrewrites: %" PRIu64 "\n", rewrites);
	}

	return 0;
}
