#include <assert.h>
#include <string.h>
#include <stdint.h>

#include "utils.h"
#include "k.h"

#define BUILTINS_MAX 1024

char* symbol_names[BUILTINS_MAX] = {
	[BUILTINS_MAX - 7] =
	"String",
	"_hole",
	"Bool",
	"Int",
	"True",
	"False",
	"_fake",
};

#define symbol_string (BUILTINS_MAX - 7)
#define symbol_hole (BUILTINS_MAX - 6)
#define symbol_bool (BUILTINS_MAX - 5)
#define symbol_int (BUILTINS_MAX - 4)
#define symbol_true (BUILTINS_MAX - 3)
#define symbol_false (BUILTINS_MAX - 2)
#define symbol_fake (BUILTINS_MAX - 1)

// FIXME: can do better than this
int num_labels = 7;

// int get_symbol(char* name) {
// 	for (int i = 0; i < num_labels; i++) {
// 		// FIXME: need max symbol length
// 		if (strcmp(name, symbol_names[i]) == 0) {
// 			return i;
// 		}
// 	}
// 	panic("Couldn't find symbol %s", name);
// }

void set_labels(int n, char* labels[static n]) {
	assert(n >= 0);
	assert(n + num_labels < BUILTINS_MAX);

	for (int i = 0; i < n; i++) {
		symbol_names[i] = labels[i];
		num_labels++;
	}
}

K* Hole() {
	return NewK(SymbolLabel(symbol_hole), NULL);
}

K* k_true() { return NewK(SymbolLabel(symbol_bool), newArgs(1, NewK(SymbolLabel(symbol_true), NULL))); }
K* k_false() { return NewK(SymbolLabel(symbol_bool), newArgs(1, NewK(SymbolLabel(symbol_false), NULL))); }

K* new_builtin_int(int64_t i) {
	return NewK(SymbolLabel(symbol_int), newArgs(1, NewK(Int64Label(i), NULL)));
}
K* new_builtin_string(char* i) {
	return NewK(SymbolLabel(symbol_string), newArgs(1, NewK(StringLabel(i), NULL)));
}

int is_int(K* k) {
	int val = k->label->symbol_val;
	return val == symbol_int;
}

int is_bool(K* k) {
	int val = k->label->symbol_val;
	return val == symbol_bool;
}

int is_hole(K* k) {
	int val = k->label->symbol_val;
	return val == symbol_hole;
}

int is_true(K* k) {
	int val = k->label->symbol_val;
	return val == symbol_true;
}

int is_false(K* k) {
	int val = k->label->symbol_val;
	return val == symbol_false;
}



// K* Hole() {
// 	return NewK(SymbolLabel(symbol_hole), NULL);
// }
