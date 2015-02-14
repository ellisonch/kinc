// TODO: need to handle max cell sizes

#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>

#include "k.h"
#include "settings.h"
#include "utils.h"
#include "cells.h"

// TODO: terrible, not right, horrible
// extern int next;

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
	return item;
}

// FIXME: leaks memory, sucks
char* kCellToString(ComputationCell *kCell) {
	char* s = malloc(10000);
	strcpy(s, "k(\n");
	for (int i = kCell->next - 1; i >= 0; i--) {
		strcat(s, "  ~> ");
		strcat(s, KToString(kCell->elements[i]));
		strcat(s, "\n");
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
			strcat(s, var);
			strcat(s, KToString(stateCell->elements[i]));
			strcat(s, "\n");
		}
	}
	strcat(s, ")\n");
	strcat(s, kCellToString(kCell));
	return s;
}

void trimK(ComputationCell *kCell) {
	int top = kCell->next - 1;
	Dec(kCell->elements[top]);
	kCell->next--;
}

void setHead(ComputationCell *kCell, K* k) {
	int top = kCell->next - 1;
	Inc(k);
	Dec(kCell->elements[top]);
	kCell->elements[top] = k;
}

void setPreHead(ComputationCell *kCell, K* k) {
	int pre = kCell->next - 2;
	Inc(k);
	Dec(kCell->elements[pre]);
	kCell->elements[pre] = k;
}


void appendK(ComputationCell *kCell, K* k) {
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
	ListK* allValues = malloc(sizeof(ListK));
	allValues->cap = k_length(c) + 26;
	allValues->len = k_length(c);
	allValues->a = malloc(sizeof(K*) * (k_length(c) + 26));
	for (int i = 0; i < c->next; i++) {
		allValues->a[i] = c->elements[i];
	}
	// memcpy(allValues, c, sizeof(K*) * next);
	for (int i = 0; i < MAX_STATE; i++) {
		K* val = state->elements[i];
		if (val == NULL) {
			continue;
		}
		allValues->a[allValues->len++] = val;
	}

	K* specialk = NewK(SymbolLabel(18), allValues); // FIX ME!!!!
	for (int i = 0; i < specialk->args->len; i++) {
 		K* arg = specialk->args->a[i];
 		Dec(arg);
 	}
	specialk->refs = 1;
	countentry* cm = counts(specialk);

	int bad = 0;
	for (int i = 0; i < 1000000; i++) {
		if (cm[i].entry != 0) {
			K* k = cm[i].entry;
			if (k->refs != cm[i].count) {
				bad = 1;
				printf("Count for %s should be %d!\n", KToString(k), cm[i].count);
			}
		}
	}
	if (bad) { panic("Bad check()!"); }
	free(cm);
}


void updateStore(StateCell* stateCell, K* keyK, K* value) {
	if (checkTypeSafety) {
		if (keyK->label->type != e_string) {
			panic("Expected key to be string label");
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

K* state_get_item(StateCell* stateCell, K* i) {
	int variable = i->label->string_val[0] - 'a';
	return stateCell->elements[variable];
}