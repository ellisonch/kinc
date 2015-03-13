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
	char* key; // should be const, but we have to free it
	K* value;
	UT_hash_handle hh;
} map_hash_entry;


typedef struct {
	map_hash_entry* hash_table;
	// int capacity;
	// K *elements[];
} MapCell;

MapCell* newMapCell();

ComputationCell* newComputationCell();
K* k_get_item(const ComputationCell* cell, int i);
int k_length(const ComputationCell* cell);

K* state_get_item(const MapCell* stateCell, const K* i);
K* state_get_item_from_name(const MapCell* stateCell, int i);
void state_cleanup(MapCell *stateCell);

void updateStore(MapCell* stateCell, K* keyK, K* value);

void check(const ComputationCell *c, const MapCell* state);
K* computation_without_first_n_arg(ComputationCell *kCell, int left);
void computation_remove_head(ComputationCell *kCell);
void computation_set_elem(ComputationCell *kCell, int pos, K* k);
void computation_insert_elems(ComputationCell *kCell, int pos, int overwriteCount, int actualResultCount, int varargCount, ...);
void computation_add_front(ComputationCell *kCell, K* k);
void computation_cleanup(ComputationCell *kCell);

char* stateString(const ComputationCell *kCell, const MapCell* stateCell);
char* kCellToString(const ComputationCell *kCell);

K* get_result(const ComputationCell *kCell);

#endif
