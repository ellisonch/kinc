// TODO: need to handle max cell sizes

#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <assert.h>

#include "k.h"
#include "settings.h"
#include "utils.h"
#include "cells.h"
#include "uthash.h"

// TODO: terrible, not right, horrible
extern int mallocedArgs;

int next = 0;

ComputationCell* newComputationCell() {
	ComputationCell* cell = malloc(sizeof(ComputationCell) + MAX_K * sizeof(K*)); // TODO: 1 too few?
	cell->capacity = MAX_K;
	cell->next = 0;
	return cell;
}

StateCell* newStateCell() {
	StateCell* cell = malloc(sizeof(StateCell) + MAX_STATE * sizeof(K*));	
	for (int i = 0; i < 26; i++) {
		cell->elements[i] = NULL;
	}
	cell->capacity = MAX_STATE;
	return cell;
}

int k_length(ComputationCell* cell) {
	return cell->next;
}

K* k_get_item(ComputationCell* cell, int i) {
	int spot = cell->next - 1 - i;
	K* item = cell->elements[spot];

	assert(item != NULL);
	assert(item->refs >= 1);
	return item;
}

// FIXME: leaks memory, sucks
char* kCellToString(ComputationCell *kCell) {
	char* s = malloc(10000);
	strcpy(s, "k(\n");
	for (int i = kCell->next - 1; i >= 0; i--) {
		char* sk = KToString(kCell->elements[i]);
		strcat(s, "  ~> ");
		strcat(s, sk);
		strcat(s, "\n");
		free(sk);
	}
	strcat(s, ")\n");
	return s;
}

// FIXME: leaks memory, sucks
char* stateString(ComputationCell *kCell, StateCell* stateCell) {
	char* s = malloc(20000);
	strcpy(s, "state(\n"); 
	for (int i = 0; i < 26; i++) {
		if (stateCell->elements[i] != NULL) {
			char var[] = "  x -> ";
 			var[2] = i + 'a';
 			char* sk = KToString(stateCell->elements[i]);
			strcat(s, var);
			strcat(s, sk);
			strcat(s, "\n");
			free(sk);
		}
	}
	char* sk = kCellToString(kCell);
	strcat(s, ")\n");
	strcat(s, sk);
	free(sk);
	return s;
}

void computation_remove_head(ComputationCell *kCell) {
	assert(kCell->next >= 1);

	int top = kCell->next - 1;
	Dec(kCell->elements[top]);
	kCell->next--;
}

void computation_set_elem(ComputationCell *kCell, int pos, K* k) {
	assert(kCell->next >= pos + 1);

	int elem = kCell->next - 1 - pos;
	Inc(k);
	Dec(kCell->elements[elem]);
	kCell->elements[elem] = k;
}

void computation_add_front(ComputationCell *kCell, K* k) {
	if (checkStackSize) {
		if (kCell->next >= MAX_K) {
			panic("Trying to add too many elements to the K Cell!");
		}
	}
	Inc(k);
	kCell->elements[kCell->next] = k;
	kCell->next++;
}

void check(ComputationCell *c, StateCell* state) {
	// ListK* allValues = mallocArgs(); // FIXME: this doesn't feel right here
	// allValues->cap = k_length(c) + 26;
	int len = k_length(c);
	K** a = malloc(sizeof(K*) * (len + 26)); // FIXME unsafe
	for (int i = 0; i < c->next; i++) {
		a[i] = c->elements[i];
	}
	// memcpy(allValues, c, sizeof(K*) * next);
	for (int i = 0; i < MAX_STATE; i++) {
		K* val = state->elements[i];
		if (val == NULL) {
			continue;
		}
		a[len++] = val;
	}

	ListK* allValues = newArgs_array(len, a);

	// create a new fake term to hold all the other terms
	K* specialk = k_new(SymbolLabel(1023), allValues); // TODO: FIXME FIX ME!!!!
	// the args get inc()ed by being passed to newk, so dec() em
	for (int i = 0; i < specialk->args->len; i++) {
 		K* arg = specialk->args->a[i];
 		Dec(arg);
 	}
	specialk->refs = 1;
	countentry** cm = counts(specialk);

	int bad = 0;
	for (countentry *s = *cm; s != NULL; s = s->hh.next) {
		if (s->entry == 0) {
			panic("There should be no 0 entries");
		}
		K* k = s->entry;
		if (k->refs != s->count) {
			bad = 1;
			printf("Count for %s should be %d, but saw %d!\n", KToString(k), s->count, k->refs);
		}
	}

	countentry_delete_all(cm);
	free(cm);
	if (bad) { 
		printf("%s\n", KToString(specialk));
		panic("Bad check()!");
	}
	for (int i = 0; i < specialk->args->len; i++) {
 		K* arg = specialk->args->a[i];
 		Inc(arg);
 	}

	Dec(specialk);
}

// TODO: unsafe
void updateStore(StateCell* stateCell, K* keyK, K* value) {
	keyK = Inner(keyK); // get rid of String() wrapper
	if (checkTypeSafety) {
		if (keyK->label->type != e_string) {
			panic("Expected key to be string label, but really %s", KToString(keyK));
		}
	}
	int key = keyK->label->string_val[0] - 'a';
	K* oldK = stateCell->elements[key];
	stateCell->elements[key] = value;
	Inc(value);
	if (oldK != NULL) {
		Dec(oldK);
	}
}

K* state_get_item_from_name(StateCell* stateCell, int i) {
	return stateCell->elements[i - 'a'];
}

// TODO unsafe
K* state_get_item(StateCell* stateCell, K* i) {
	K* keyK = Inner(i);
	if (checkTypeSafety) {
		if (keyK->label->type != e_string) {
			panic("Expected key to be string label, but really %s", KToString(keyK));
		}
	}
	int variable = keyK->label->string_val[0] - 'a';
	return stateCell->elements[variable];
}

K* get_result(ComputationCell *kCell) {
	if (kCell->next != 1) {
		panic("Expected a single value on the K Cell, but instead have %d.  %s", kCell->next, kCellToString(kCell));
	}
	return k_get_item(kCell, 0);
}

void computation_cleanup(ComputationCell *kCell) {
	while (kCell->next > 0) {
		computation_remove_head(kCell);
	}
}

// TODO: assumes 26 vars
void state_cleanup(StateCell *stateCell) {
	for (int i = 0; i < 26; i++) {
		K* oldK = stateCell->elements[i];
		stateCell->elements[i] = NULL;
		if (oldK != NULL) {
			Dec(oldK);
		}
	}
}
