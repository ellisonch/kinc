#ifndef K_BUILTINS_H
#define K_BUILTINS_H

#include "k_types.h"

void set_labels(int n, char* labels[static n]);

int is_int(const K* k);
int is_bool(const K* k);
int is_hole(const K* k);
int is_true(const K* k);
int is_false(const K* k);

KLabel* k_builtin_kra_label();
K* k_builtin_hole();
K* k_builtin_true();
K* k_builtin_false();
K* k_builtin_int_zero();
K* k_builtin_int_one();

K* new_builtin_int(int64_t i);
K* new_builtin_string(char* i);

K* k_builtin_int_plus(K* v1, K* v2);
K* k_builtin_int_lte(K* v1, K* v2);
K* k_builtin_bool_not(K* v1);

int k_builtin_bool_symbol();
int k_builtin_true_symbol();
int k_builtin_false_symbol();
int k_builtin_int_symbol();

void k_init_builtins();

#endif
