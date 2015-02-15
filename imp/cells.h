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

void updateStore(StateCell* stateCell, K* keyK, K* value);

void check(ComputationCell *c, StateCell* state);
void trimK(ComputationCell *kCell);
void setHead(ComputationCell *kCell, K* k);
void setPreHead(ComputationCell *kCell, K* k);
void appendK(ComputationCell *kCell, K* k);
char* stateString(ComputationCell *kCell, StateCell* stateCell);

#endif
