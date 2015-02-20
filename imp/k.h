#ifndef K_H
#define K_H

#include <stdio.h>

#include "k_types.h"
#include "k_labels.h"
#include "k_builtins.h"
#include "uthash.h"
// #include "inline_helper.h"

typedef struct {
	const K* entry;
	int count;
	UT_hash_handle hh;
} countentry;

// TODO: possibly can get rid of some of these:


typedef struct {
	int count;
	char** labels;
} label_helper;

char* KToString(const K* k);


void k_init();

K* k_get_arg(const K* k, int i);
int k_num_args(const K* k);
K* k_replace_arg(K* k, int arg, K* ov, K* nv);
K* updateTrimArgs(K* k, int left, int right);

K* k_new_empty(KLabel* label);
K* k_new(KLabel* label, int count, ...);
K* k_new_array(KLabel* label, int count, K** a);

void k_make_permanent(K* k);

double garbage_get_capacity();

void Dec(K* k);
void Inc(K* k);

countentry** counts(int len, const K** a);
void countentry_delete_all(countentry** counts);

void dump_garbage_info();

K* aterm_file_to_k(FILE* file, label_helper lh, K* hole);

#endif
