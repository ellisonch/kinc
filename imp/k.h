#ifndef K_H
#define K_H

typedef enum {
	e_symbol,
	e_string,
	e_i64,
} KLabelType;

// typedef struct K *ListK[];
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


typedef struct countentry {
	K* entry;
	int count;
} countentry;

// TODO: possibly can get rid of some of these:

const char* KToString(K* k);
void Dec(K* k);
void Inc(K* k);
const char* ListKToString(ListK* args);

KLabel* SymbolLabel(int s);
KLabel* Int64Label(int64_t i64);
KLabel* StringLabel(const char* s);
K* NewK(KLabel* label, ListK* args);
ListK* newArgs(int count, ...);
K* UpdateArg(K* k, int arg, K* newVal);
K* Inner(K* k);
K* updateTrimArgs(K* k, int left, int right);
countentry* counts(K* k);

void dump_garbage_info();

#endif
