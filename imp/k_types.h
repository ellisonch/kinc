#ifndef K_TYPES_H
#define K_TYPES_H

#include <stdint.h>

#include "settings.h"

typedef enum {
	e_symbol,
	e_string,
	e_i64,
} KLabelType;

typedef struct {
	KLabelType type;
	union {
		int64_t i64_val;
		int symbol_val;
		const char* string_val;
	};
} KLabel;

typedef struct K {
	KLabel* label;
	struct ListK {
		int cap;
		int len;
		struct K* a[MAX_GARBAGE_ARG_LEN];
	}* args;
	int refs;
} K;

typedef struct ListK ListK;

#endif
