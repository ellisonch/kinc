// TODO: need to handle max cell sizes

#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>

#include "k.h"
#include "settings.h"
#include "utils.h"
#include "cells.h"
#include "uthash.h"

int next = 0;

ComputationCell* newComputationCell() {
	if (printDebug) { 
		printf("Creating new computation cell\n");
	}
	ComputationCell* cell = malloc(sizeof(*cell));
	// cell->capacity = MAX_K;
	// cell->next = 0;
	cell->holder = k_new_empty(k_builtin_kra_label());
	Inc(cell->holder);
	return cell;
}

MapCell* newMapCell() {
	// StateCell* cell = malloc(sizeof(StateCell) + MAX_STATE * sizeof(K*));	
	// for (int i = 0; i < 26; i++) {
	// 	cell->elements[i] = NULL;
	// }
	// cell->capacity = MAX_STATE;

	MapCell* cell = malloc(sizeof(*cell));
	cell->hash_table = NULL;

	return cell;
}

int k_length(const ComputationCell* cell) {
	assert(cell != NULL);
	// return cell->next;
	return k_num_args(cell->holder);
}

K* k_get_item(const ComputationCell* cell, int i) {
	K* item = k_get_arg(cell->holder, i);
	// int spot = cell->next - 1 - i;
	// K* item = cell->elements[spot];

	return item;
}

// FIXME: leaks memory, sucks
char* kCellToString(const ComputationCell *kCell) {
	char* s = malloc(10000);
	strcpy(s, "k(\n");
	for (int i = 0; i < k_length(kCell); i++) {
		char* sk = KToString(k_get_item(kCell, i));
		strcat(s, "  ~> ");
		strcat(s, sk);
		strcat(s, "\n");
		free(sk);
	}
	strcat(s, ")\n");
	return s;
}

char* mapToString(const MapCell* stateCell) {
	char* s = malloc(20000);
	int length = 0;
	length += snprintf(s + length, 20000 - length, "state(\n");

	map_hash_entry *needle;
	map_hash_entry *tmp;

	HASH_ITER(hh, stateCell->hash_table, needle, tmp) {
		char* sk = KToString(needle->value);
		length += snprintf(s + length, 20000 - length, "  %s -> %s\n", needle->key, sk);
		free(sk);
	}
	// length += snprintf(s + length, 20000 - length, "%s)", s);
	if (length < 20000) {
		s[length] = ')';
	}
	return s;
}

// FIXME: leaks memory, sucks
char* stateString(const ComputationCell *kCell, const MapCell* stateCell) {
	char* s = malloc(20000);
	int length = 0;
	char* sm = mapToString(stateCell);
	char* sk = kCellToString(kCell);
	length += snprintf(s + length, 20000 - length, "%s\n%s", sm, sk);
	free(sm);
	free(sk);

	return s;
}

K* computation_without_first_n_arg(ComputationCell *kCell, int left) {
	return k_without_first_n_arg(kCell->holder, left);
}

void computation_remove_head(ComputationCell *kCell) {
	assert(kCell != NULL);

	k_remove_arg_head(kCell->holder);
	// assert(kCell != NULL);
	// assert(k_length(kCell) >= 1);

	// int top = kCell->next - 1;
	// Dec(kCell->elements[top]);
	// kCell->next--;
}

void computation_set_elem(ComputationCell *kCell, int pos, K* k) {
	assert(kCell != NULL);
	assert(k_length(kCell) > pos);

	k_set_arg(kCell->holder, pos, k);

	// int elem = kCell->next - 1 - pos;
	// Inc(k);
	// Dec(kCell->elements[elem]);
	// kCell->elements[elem] = k;
}

void computation_insert_elems(ComputationCell *kCell, int pos, int overwriteCount, int actualResultCount, int varargCount, ...) {
	va_list elems;
	va_start(elems, varargCount);

	kCell->holder = k_insert_elems_vararg(kCell->holder, pos, overwriteCount, actualResultCount, varargCount, elems);
	va_end(elems);
}

void computation_add_front(ComputationCell *kCell, K* k) {
	assert(kCell != NULL);
	if (checkStackSize) {
		if (k_length(kCell) > 20) {
			panic("Trying to add too many elements to the K Cell!");
		}
	}

	k_add_front_arg(kCell->holder, k);

	// Inc(k);
	// kCell->elements[kCell->next] = k;
	// kCell->next++;
}

void check(const ComputationCell *c, const MapCell* state) {
	// panic("FIXME: Not handling check() yet!");
	// ListK* allValues = mallocArgs(); // FIXME: this doesn't feel right here
	// allValues->cap = k_length(c) + 26;

	int len = k_length(c);
	const K** a = malloc(sizeof(K*) * (len + HASH_CNT(hh, state->hash_table)));
	for (int i = 0; i < k_length(c); i++) {
		a[i] = k_get_item(c, i);
	}

	map_hash_entry *needle;
	map_hash_entry *tmp;
	HASH_ITER(hh, state->hash_table, needle, tmp) {
		a[len++] = needle->value;
	}

	countentry** cm = counts(len, a);

	int bad = 0;
	for (countentry *s = *cm; s != NULL; s = s->hh.next) {
		if (s->entry == 0) {
			panic("There should be no 0 entries");
		}
		const K* k = s->entry;
		if (k->refs != s->count) {
			bad = 1;
			printf("Count for %s should be %d, but saw %d!\n", KToString(k), s->count, k->refs);
		}
	}

	countentry_delete_all(cm);
	free(cm);
	free(a);
	if (bad) {
		// printf("%s\n", KToString(specialk));
		panic("Bad check()!");
	}
}

void counts_aux(const K* k, countentry **counts) {
	countentry *find;
	HASH_FIND_INT(*counts, &k, find);
	if (find == NULL) {
		countentry *new = malloc(sizeof(*new));
	 	new->entry = k;
	 	new->count = 1;
	 	HASH_ADD_INT(*counts, entry, new);

	 	for (int i = 0; i < k_num_args(k); i++) {
			K* arg = k_get_arg(k, i);
			counts_aux(arg, counts);
		}
	} else {
		find->count++;
		// printf("Adding one to %s's count\n", KToString(k));
	}
}

countentry** counts(int len, const K** a) {
	countentry** counts = malloc(sizeof(*counts));
	*counts = NULL;

	for (int i = 0; i < len; i++) {
		const K* k = a[i];
		counts_aux(k, counts);
	}
	
	return counts;
}

void countentry_delete_all(countentry** counts) {
	countentry *s;
	countentry *tmp;

	HASH_ITER(hh, *counts, s, tmp) {
		HASH_DEL(*counts, s);  /* delete; users advances to next */
		free(s);            /* optional- if you want to free  */
	}
}

// string_make_copy

// TODO: unsafe
void updateStore(MapCell* stateCell, K* keyK, K* value) {
	assert(stateCell != NULL);
	assert(keyK != NULL);
	assert(value != NULL);

	if (printDebug) {
		printf("Updating store.\n");
		printf("Current state is %s\n", mapToString(stateCell));
		printf("adding %s |-> %s\n", KToString(keyK), KToString(value));
	}

	keyK = k_get_arg(keyK, 0); // get rid of String() wrapper
	// if (checkTypeSafety) {
	// 	if (keyK->label->type != e_string) {
	// 		panic("Expected key to be string label, but really %s", KToString(keyK));
	// 	}
	// }
	// int key = keyK->label->string_val[0] - 'a';
	assert(keyK->label->type == e_string);
	char* key = keyK->label->string_val;
	map_hash_entry* needle;
	HASH_FIND_STR(stateCell->hash_table, key, needle);

	// HASH_FIND_INT(*counts, &k, find);
	if (needle == NULL) {
		map_hash_entry *new = malloc(sizeof(*new));
	 	new->key = string_make_copy(key);
	 	new->value = value;
	 	HASH_ADD_KEYPTR(hh, stateCell->hash_table, new->key, strlen(new->key), new);
	 	Inc(value);
	} else {
		K* oldK = needle->value;
		needle->value = value;
		Inc(value);
		Dec(oldK);
	}
}

// K* state_get_item_from_name(const StateCell* stateCell, int i) {
// 	return stateCell->elements[i - 'a'];
// }

// TODO unsafe
K* state_get_item(const MapCell* stateCell, const K* keyK) {
	keyK = k_get_arg(keyK, 0); // get rid of String() wrapper
	if (checkTypeSafety) {
		if (keyK->label->type != e_string) {
			panic("Expected key to be string label, but really %s", KToString(keyK));
		}
	}
	// int key = keyK->label->string_val[0] - 'a';
	char* key = keyK->label->string_val;
	map_hash_entry* needle;
	HASH_FIND_STR(stateCell->hash_table, key, needle);

	if (needle == NULL) {
		panic("Trying to lookup %s, but no such key was found in map", key);
	} else {
		return needle->value;
	}
}

K* get_result(const ComputationCell *kCell) {
	if (k_length(kCell) != 1) {
		panic("Expected a single value on the K Cell, but instead have %d.  %s", k_length(kCell), kCellToString(kCell));
	}
	return k_get_item(kCell, 0);
}

void computation_cleanup(ComputationCell *kCell) {
	while (k_length(kCell) > 0) {
		computation_remove_head(kCell);
	}
}

// TODO: assumes 26 vars
void state_cleanup(MapCell *stateCell) {
	// panic("Not yet handling state_cleanup!()");
	map_hash_entry *needle;
	map_hash_entry *tmp;
	HASH_ITER(hh, stateCell->hash_table, needle, tmp) {
		HASH_DEL(stateCell->hash_table, needle);
		K* oldK = needle->value;
		needle->value = NULL;
		free(needle->key);
		Dec(oldK);
		free(needle);
	}

	// for (int i = 0; i < 26; i++) {
	// 	K* oldK = stateCell->elements[i];
	// 	stateCell->elements[i] = NULL;
	// 	if (oldK != NULL) {
	// 		Dec(oldK);
	// 	}
	// }
}
