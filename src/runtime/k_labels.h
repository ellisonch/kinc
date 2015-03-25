#ifndef K_LABELS_H
#define K_LABELS_H

#include "k_types.h"

char* LabelToString(const KLabel* label);
void dispose_label(K* k);
void dump_label_garbage_info();
KLabel* SymbolLabel(int s);
KLabel* Int64Label(int64_t i64);
KLabel* StringLabel(const char* s);

#endif
