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
K* _zero;
K* _one;

void k_init_builtins() {
	_hole = k_new_empty(SymbolLabel(symbol_hole));
	k_make_permanent(_hole);
	_false = k_new(SymbolLabel(symbol_bool), 1, k_new_empty(SymbolLabel(symbol_false)));
	k_make_permanent(_false);
	_true = k_new(SymbolLabel(symbol_bool), 1, k_new_empty(SymbolLabel(symbol_true)));
	k_make_permanent(_true);
	_zero = k_new(SymbolLabel(symbol_int), 1, k_new_empty(Int64Label(0)));
	k_make_permanent(_zero);
	_one = k_new(SymbolLabel(symbol_int), 1, k_new_empty(Int64Label(1)));
	k_make_permanent(_one);
}

K* k_hole() {
	return _hole;
}

K* k_true() { 
	return _true;
}
K* k_false() {
	return _false;
}

K* k_int_zero() {
	return _zero;
}
K* k_int_one() {
	return _one;
}


K* new_builtin_int(int64_t i) {
	// K* arg = k_new_empty(Int64Label(i));
	// return k_new_array(SymbolLabel(symbol_int), 1, &arg);
	return k_new(SymbolLabel(symbol_int), 1, k_new_empty(Int64Label(i)));
}
K* new_builtin_string(char* i) {
	return k_new(SymbolLabel(symbol_string), 1, k_new_empty(StringLabel(i)));
}

int is_int(const K* k) {
	int val = k->label->symbol_val;
	return val == symbol_int;
}

int is_bool(const K* k) {
	int val = k->label->symbol_val;
	return val == symbol_bool;
}

// TODO: could just compare against permanent _hole
int is_hole(const K* k) {
	int val = k->label->symbol_val;
	return val == symbol_hole;
}

int is_true(const K* k) {
	int val = k->label->symbol_val;
	return val == symbol_true;
}

int is_false(const K* k) {
	int val = k->label->symbol_val;
	return val == symbol_false;
}
