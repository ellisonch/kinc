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

K* k_remove_first_n_arg(K* k, int left);
void k_remove_arg_head(K* k);
void k_set_arg(K* k, int i, K* v);
void k_add_front_arg(K* k, K* v);

K* k_insert_elems(K* k, int pos, int overwriteCount, int count, ...);
K* k_insert_elems_vararg(K* k, int pos, int overwriteCount, int count, va_list elems);
void k_set_label(K* k, KLabel* label);
K* k_get_arg(const K* k, int i);
int k_num_args(const K* k);
K* k_replace_arg(K* k, int arg, K* ov, K* nv);
K* updateTrimArgs(K* k, int left, int right);

K* k_new_empty(KLabel* label);
K* k_new(KLabel* label, int count, ...);
K* k_new_from_array(KLabel* label, int count, K** a);
K* k_new_from_k_args(KLabel* label, K* k);

void k_make_permanent(K* k);

double garbage_get_capacity();

void Dec(K* k);
void Inc(K* k);

countentry** counts(int len, const K** a);
void countentry_delete_all(countentry** counts);

void dump_garbage_info();

K* aterm_file_to_k(FILE* file, label_helper lh, K* hole);

#endif
