#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "k.h"
#include "settings.h"
#include "utils.h"

// TODO: fix this
extern char* symbol_names[];

int garbage_label_next = 0;
KLabel* garbage_label[MAX_GARBAGE_KEPT];

int count_malloc_label = 0;
KLabel* symbolLabels[50];


KLabel* mallocKLabel() {
	count_malloc_label++;
	return (KLabel*)malloc(sizeof(KLabel));
}

KLabel* _new_label() {
	KLabel* newL;
	if (garbage_label_next > 0) {
		newL = garbage_label[garbage_label_next - 1];
		garbage_label_next--;
	} else {
		newL = mallocKLabel();
	}
	return newL;
}

void dispose_label(K* k) {
	assert(k != NULL);

	KLabel* label = k->label;

	// TODO: why am I doing this?
	if (label->type == e_symbol) {
		return;
	}

	if (garbage_label_next < MAX_GARBAGE_KEPT) {
		garbage_label[garbage_label_next++] = label;
		if (printDebug) { printf("Saving label\n"); }
	} else {
		if (printDebug) { printf("Freeing label\n"); }
		count_malloc_label--;
		free(label);
	}
}

void dump_label_garbage_info() {
	printf("count_malloc_label: %d\n", count_malloc_label);
	printf("garbage_label_next: %d\n", garbage_label_next);
}

KLabel* StringLabel(const char* s) {
	if (printDebug) { printf("Str garbage_label_next: %d\n", garbage_label_next); }
	if (printDebug) { printf("Creating string label %s\n", s); }
	KLabel* newL = _new_label();
	newL->type = e_string;
	newL->string_val = s;
	return newL;
}

KLabel* SymbolLabel(int s) {
	if (symbolLabels[s] != NULL) {
		return symbolLabels[s];
	}
	if (printDebug) { printf("Sym garbage_label_next: %d\n", garbage_label_next); }
	if (printDebug) { printf("Creating symbol label %s\n", symbol_names[s]); }
	KLabel* newL = _new_label();
	newL->type = e_symbol;
	newL->symbol_val = s;
	symbolLabels[s] = newL;

	return newL;
}

KLabel* Int64Label(int64_t i64) {
	// intcount++;
	if (printDebug) { printf("Int garbage_label_next: %d\n", garbage_label_next); }
	if (printDebug) { printf("Creating int label %" PRId64 "\n", i64); }
	KLabel* newL = _new_label();
	newL->type = e_i64;
	newL->i64_val = i64;
	return newL;
}


// TODO: leaks memory and is unsafe
const char* LabelToString(KLabel* label) {
	if (label->type == e_string) {
		return label->string_val;
	} else if (label->type == e_i64) {
		char* s = malloc(50);
		snprintf(s, 50, "%" PRId64, label->i64_val);
		return s;
	} else if (label->type == e_symbol) {
		char* val = symbol_names[label->symbol_val];
		if (val == NULL) {
			panic("Couldn't find symbol name for %d\n", label->symbol_val);
		}
		return val;
	} else {
		panic("Some unknown label type %d found", label->type);
	}
	// return NULL;
}



KLabel* copyLabel(KLabel* l) {
	if (printDebug) { printf("Cpy garbage_label_next: %d\n", garbage_label_next); }
	if (printDebug) { printf("Creating copy label %s\n", LabelToString(l)); }
	KLabel* newL;
	if (garbage_label_next > 0) {
		newL = garbage_label[garbage_label_next - 1];
		garbage_label_next--;
	} else {
		newL = mallocKLabel();
	}
	memcpy(newL, l, sizeof(KLabel));
	return newL;
}
