#ifndef K_H
#define K_H

#include "k_types.h"
#include "k_labels.h"
#include "k_builtins.h"
#include "uthash.h"

typedef struct {
	K* entry;
	int count;
	UT_hash_handle hh;
} countentry;

// TODO: possibly can get rid of some of these:

const char* KToString(K* k);
void Dec(K* k);
void Inc(K* k);
const char* ListKToString(ListK* args);

K* NewK(KLabel* label, ListK* args);
ListK* newArgs(int count, ...);
K* UpdateArg(K* k, int arg, K* newVal);
K* Inner(K* k);
K* updateTrimArgs(K* k, int left, int right);
countentry** counts(K* k);

void dump_garbage_info();

#endif
