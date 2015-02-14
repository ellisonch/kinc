#ifndef K_TYPES_H
#define K_TYPES_H

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
		struct K** a;
	}* args;
	int refs;
} K;

typedef struct ListK ListK;

#endif
