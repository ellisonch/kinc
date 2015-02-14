#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>

#include "k.h"
#include "settings.h"
#include "utils.h"

// TODO: fix this
extern char* givenLabels[];

int deadLabelLen = 0;

KLabel* deadLabels[MAX_GARBAGE_KEPT];

int mallocedLabels = 0;
KLabel* symbolLabels[50];


KLabel* mallocKLabel() {
	mallocedLabels++;
	return (KLabel*)malloc(sizeof(KLabel));
}

KLabel* _new_label() {
	KLabel* newL;
	if (deadLabelLen > 0) {
		newL = deadLabels[deadLabelLen - 1];
		deadLabelLen--;
	} else {
		newL = mallocKLabel();
	}
	return newL;
}

void dispose_label(KLabel* label) {
	if (deadLabelLen < MAX_GARBAGE_KEPT) {
		deadLabels[deadLabelLen++] = label;
		if (printDebug) { printf("Saving label\n"); }
	} else {
		if (printDebug) { printf("Freeing label\n"); }
		mallocedLabels--;
		free(label);
	}
}

void dump_label_garbage_info() {
	printf("MallocedLabels: %d\n", mallocedLabels);
	printf("deadLabelLen: %d\n", deadLabelLen);
}

KLabel* StringLabel(const char* s) {
	if (printDebug) { printf("Str DeadLabelLen: %d\n", deadLabelLen); }
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
	if (printDebug) { printf("Sym DeadLabelLen: %d\n", deadLabelLen); }
	if (printDebug) { printf("Creating symbol label %s\n", givenLabels[s]); }
	KLabel* newL = _new_label();
	newL->type = e_symbol;
	newL->symbol_val = s;
	symbolLabels[s] = newL;

	return newL;
}

KLabel* Int64Label(int64_t i64) {
	// intcount++;
	if (printDebug) { printf("Int DeadLabelLen: %d\n", deadLabelLen); }
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
		return givenLabels[label->symbol_val];
	} else {
		panic("Some unknown label type %d found", label->type);
	}
	// return NULL;
}



KLabel* copyLabel(KLabel* l) {
	if (printDebug) { printf("Cpy DeadLabelLen: %d\n", deadLabelLen); }
	if (printDebug) { printf("Creating copy label %s\n", LabelToString(l)); }
	KLabel* newL;
	if (deadLabelLen > 0) {
		newL = deadLabels[deadLabelLen - 1];
		deadLabelLen--;
	} else {
		newL = mallocKLabel();
	}
	memcpy(newL, l, sizeof(KLabel));
	return newL;
}
