#ifndef K_TYPES_H
#define K_TYPES_H

#include <stdint.h>

#include "settings.h"

typedef enum {
	E_NOT_LIST,
	E_LIST,
} ListOrNot;

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
		char* string_val;
	};
} KLabel;

typedef struct K {
	KLabel* label;
	struct ListK {
		int cap;

		// 0 1 2 3 
		//   a b
		// then pos_first = 1
		// and pos_end = 3
		// pos_first == pos_end => empty
		
		int pos_first;
		int pos_end;
		struct K** a;
	} args;
	int refs;
	int permanent; // doesn't get garbage collected
} K;

typedef struct ListK ListK;

#endif
