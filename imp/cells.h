#ifndef CELLS_H
#define CELLS_H

#include "k.h"

typedef struct {
	// int capacity;
	// int next;
	// K *elements[];
	K* holder;
} ComputationCell;

typedef struct {
	int capacity;
	K *elements[];
} StateCell;

StateCell* newStateCell();

ComputationCell* newComputationCell();
K* k_get_item(const ComputationCell* cell, int i);
int k_length(const ComputationCell* cell);

K* state_get_item(const StateCell* stateCell, const K* i);
K* state_get_item_from_name(const StateCell* stateCell, int i);
void state_cleanup(StateCell *stateCell);

void updateStore(StateCell* stateCell, K* keyK, K* value);

void check(const ComputationCell *c, const StateCell* state);
void computation_remove_head(ComputationCell *kCell);
void computation_set_elem(ComputationCell *kCell, int pos, K* k);
void computation_add_front(ComputationCell *kCell, K* k);
void computation_cleanup(ComputationCell *kCell);

char* stateString(const ComputationCell *kCell, const StateCell* stateCell);
char* kCellToString(const ComputationCell *kCell);

K* get_result(const ComputationCell *kCell);

#endif
