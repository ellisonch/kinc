#ifndef CELLS_H
#define CELLS_H

#include "k.h"

typedef struct {
	int capacity;
	int next;
	K *elements[];
} ComputationCell;

typedef struct {
	int capacity;
	K *elements[];
} StateCell;

StateCell* newStateCell();

ComputationCell* newComputationCell();
K* k_get_item(ComputationCell* cell, int i);
int k_length(ComputationCell* cell);

K* state_get_item(StateCell* stateCell, K* i);
K* state_get_item_from_name(StateCell* stateCell, int i);
void state_cleanup(StateCell *stateCell);

void updateStore(StateCell* stateCell, K* keyK, K* value);

void check(ComputationCell *c, StateCell* state);
void computation_remove_head(ComputationCell *kCell);
void computation_set_elem(ComputationCell *kCell, int pos, K* k);
void computation_add_front(ComputationCell *kCell, K* k);
void computation_cleanup(ComputationCell *kCell);

char* stateString(ComputationCell *kCell, StateCell* stateCell);
char* kCellToString(ComputationCell *kCell);

K* get_result(ComputationCell *kCell);

#endif
