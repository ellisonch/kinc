#include <assert.h>
#include <string.h>
#include <stdint.h>

#include "utils.h"
#include "k.h"

char* symbol_names[SYMBOLS_MAX] = {
	[SYMBOLS_MAX - 7] =
	"String",
	"_hole",
	"Bool",
	"Int",
	"True",
	"False",
	"_fake",
};

#define symbol_string (SYMBOLS_MAX - 7)
#define symbol_hole (SYMBOLS_MAX - 6)
#define symbol_bool (SYMBOLS_MAX - 5)
#define symbol_int (SYMBOLS_MAX - 4)
#define symbol_true (SYMBOLS_MAX - 3)
#define symbol_false (SYMBOLS_MAX - 2)
#define symbol_fake (SYMBOLS_MAX - 1)

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
	assert(n + num_labels < SYMBOLS_MAX);

	for (int i = 0; i < n; i++) {
		symbol_names[i] = labels[i];
		num_labels++;
	}
}

K* _hole;
K* _false;
K* _true;

void k_init_builtins() {
	_hole = k_new_empty(SymbolLabel(symbol_hole));
	_hole->permanent = 1;
	_false = k_new(SymbolLabel(symbol_bool), newArgs(1, k_new_empty(SymbolLabel(symbol_false))));
	_false->permanent = 1;
	_true = k_new(SymbolLabel(symbol_bool), newArgs(1, k_new_empty(SymbolLabel(symbol_true)))); 
	_true->permanent = 1;
}

K* Hole() {
	// return k_new_empty(SymbolLabel(symbol_hole));
	return _hole;
}

K* k_true() { 
	// return k_new(SymbolLabel(symbol_bool), newArgs(1, k_new_empty(SymbolLabel(symbol_true)))); 
	return _true;
}
K* k_false() {
	// return k_new(SymbolLabel(symbol_bool), newArgs(1, k_new_empty(SymbolLabel(symbol_false))));
	return _false;
}

K* new_builtin_int(int64_t i) {
	return k_new(SymbolLabel(symbol_int), newArgs(1, k_new_empty(Int64Label(i))));
}
K* new_builtin_string(char* i) {
	return k_new(SymbolLabel(symbol_string), newArgs(1, k_new_empty(StringLabel(i))));
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
