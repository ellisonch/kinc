#ifndef K_BUILTINS_H
#define K_BUILTINS_H

#include "k_types.h"

void set_labels(int n, char* labels[static n]);

int is_int(K* k);
int is_bool(K* k);
int is_hole(K* k);
int is_true(K* k);
int is_false(K* k);

K* k_hole();
K* k_true();
K* k_false();
K* k_int_zero();
K* k_int_one();

K* new_builtin_int(int64_t i);
K* new_builtin_string(char* i);

void k_init_builtins();

#endif
