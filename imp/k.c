#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "k.h"
#include "k_labels.h"
#include "settings.h"
#include "utils.h"
#include "uthash.h"
#include "aparser/aterm.h"


#define MAX_GARBAGE_KEPT 10000



K* garbage_k[MAX_GARBAGE_KEPT];
int garbage_k_next = 0;

ListK* garbage_listk[MAX_GARBAGE_ARG_LEN+1][MAX_GARBAGE_KEPT];

// the next available garbage location
int garbage_listk_nexts[MAX_GARBAGE_ARG_LEN+1];

int count_malloc_listk;
int count_malloc_listk_array[MAX_MALLOC_LISTK_ARRAY_SIZE];

int count_malloc_k = 0;

ListK* getDeadList(int reqLength) {
	// return NULL;
	if (reqLength > MAX_GARBAGE_ARG_LEN) {
		return NULL;
	}
	ListK** deadList = garbage_listk[reqLength];
	if (garbage_listk_nexts[reqLength] == 0) {
		if (printDebug) { printf("No dead stuff of length %d\n", reqLength); }
		return NULL;
	}

	ListK* ret = deadList[garbage_listk_nexts[reqLength] - 1];
	garbage_listk_nexts[reqLength]--;

	if (checkGC) {
		if (ret == NULL) {
			panic("Didn't expect ret to be nil");
		}
		if (ret->cap < reqLength) {
			panic("Expected list to be of cap %d, but it was %d instead", reqLength, ret->cap);
		}
	}
	ret->len = reqLength;

	if (printDebug) {
		printf("Returning dead list of length %d\n", reqLength);
	}
	return ret;
}



ListK* mallocArgs() {
	count_malloc_listk++;
	return malloc(sizeof(ListK));
}

K** mallocArgsA(int count) {
	if (count < MAX_MALLOC_LISTK_ARRAY_SIZE) {
		count_malloc_listk_array[count]++;
	}
	return malloc(sizeof(K*) * count);
}

ListK* newArgs(int count, ...) {
	ListK* args = getDeadList(count);
	if (args == NULL) {
		args = mallocArgs();
		args->a = mallocArgsA(count);
		args->cap = count;
		args->len = count;
	}
	if (checkGC) {
		if (count != args->len) {
			panic("Expected %d == %d\n", count, args->len);
		}
	}

	va_list ap;
	va_start(ap, count);
	for (int i = 0; i < count; i++) {
		args->a[i] = va_arg(ap, K*);
	}
	va_end(ap);

	return args;
}

ListK* newArgs_array(int count, K** a) {
	ListK* args = getDeadList(count);
	if (args == NULL) {
		args = mallocArgs();
		args->a = mallocArgsA(count);
		args->cap = count;
		args->len = count;
	}
	if (checkGC) {
		if (count != args->len) {
			panic("Expected %d == %d\n", count, args->len);
		}
	}

	for (int i = 0; i < count; i++) {
		args->a[i] = a[i];
	}

	return args;
}

// TODO unsafe
K* Inner(K* k) {
	return k->args->a[0];
}

void Inc(K* k) {
	k->refs++;
}

K* mallocK() {
	count_malloc_k++;
	return malloc(sizeof(K));
}

ListK* emptyArgs() {
	ListK* args = getDeadList(0);
	if (args == NULL) {
		args = mallocArgs();
		args->cap = 0;
		args->len = 0;
	}
	return args;
}

K* k_new_empty(KLabel* label) {
	return k_new(label, emptyArgs());
}

K* k_new(KLabel* label, ListK* args) {
	assert(label != NULL);
	assert(args != NULL);

	for (int i = 0; i < args->len; i++) {
 		K* arg = args->a[i];
 		if (arg == NULL) {
 			panic("Didn't expect nil arg in k_new()");
 		}
 		Inc(arg);
 	}
	
	K* newK = NULL;
	if (garbage_k_next > 0) {
		newK = garbage_k[garbage_k_next - 1];
		garbage_k_next--;
	} else {
		newK = mallocK();
	}
	assert(newK != NULL);
	newK->label = label;
	newK->args = args;
	newK->refs = 0;
	return newK;
}



// TODO: leaks memory and is unsafe
const char* ListKToString(ListK* args) {
	if (args == NULL) {
		return "";
	}
	char* s = malloc(3000);
	s[0] = '\0';

	// size_t len = strlen(s);
	for (int i = 0; i < args->len; i++) {
		K* arg = args->a[i];
		strcat(s, KToString(arg));
		if (i < args->len - 1) {
			strcat(s, ", ");
		}
	}
	
	return s;
}

// TODO: leaks memory and is unsafe
const char* KToString(K* k) {
	char* s = malloc(1000);
	if (k == NULL) {
		strcpy(s, "(null)");
		return s;
	}
	if (printRefCounts) {
		snprintf(s, 1000, "%s[%d](%s)", LabelToString(k->label), k->refs, ListKToString(k->args));
	} else {
		snprintf(s, 1000, "%s(%s)", LabelToString(k->label), ListKToString(k->args));
	}
	return s;
}

void terminate_args(ListK* args) {
	int number_of_args = args->cap;

	assert(args != NULL);
	assert(args->a != NULL);
	assert(number_of_args >= 0);
	// assert(number_of_args < MAX_MALLOC_LISTK_ARRAY_SIZE);
	assert(count_malloc_listk >= 1);

	count_malloc_listk--;
	if (number_of_args > 0) {
		if (number_of_args < MAX_MALLOC_LISTK_ARRAY_SIZE) {
			assert(count_malloc_listk_array[number_of_args] >= 1);
			count_malloc_listk_array[number_of_args]--;
		}
		free(args->a);
	}
	if (printDebug) { printf("Freeing args\n"); }		
	free(args);
}

int COMPLAINED_MAX_GARBAGE_ARG_LEN = 0;
// we're done with a ListK, and we need to see if we can reuse it
void dispose_args(K* k) {	
	assert(k != NULL);
	ListK* args = k->args;

	// // since k is dead, we can remove one ref from any of its children
	// for (int i = 0; i < args->len; i++) {
	// 	K* arg = args->a[i];
	// 	Dec(arg);
	// }

	int number_of_args = args->cap;

	// if we don't keep around args of this many arguments, then kill it
	if (number_of_args >= MAX_GARBAGE_ARG_LEN) {
		if (!COMPLAINED_MAX_GARBAGE_ARG_LEN && checkRightSettings) {
			printf("MAX_GARBAGE_ARG_LEN is not enough for dead term with len %d\n", number_of_args);
			COMPLAINED_MAX_GARBAGE_ARG_LEN = 1;
		}

		terminate_args(args);
		return;
	}

	// if we already have too many args of this many arguments, then kill it
	int next_arg_index = garbage_listk_nexts[number_of_args];
	if (next_arg_index >= MAX_GARBAGE_KEPT) {
		terminate_args(args);
		return;
	}

	// save it
	garbage_listk_nexts[number_of_args]++;
	garbage_listk[number_of_args][next_arg_index] = args;
	if (printDebug) { printf("Saving args\n"); }
}

void dispose_k_itself(K* k) {
	assert(k != NULL);

	if (garbage_k_next < MAX_GARBAGE_KEPT) {
		if (printDebug) { printf("Saving k\n"); }
		garbage_k[garbage_k_next++] = k;
	} else {
		if (printDebug) { printf("Freeing k\n"); }
		count_malloc_k--;
		free(k);
	}
}

// K* garbage_k_pending[MAX_GARBAGE_PENDING];
// int garbage_k_pending_next = 0;

void dispose_k_aux(K* k) {
	dispose_args(k);
	dispose_label(k);
	dispose_k_itself(k);
}

// this is called where there are no references to k, so we can reclaim its memory for use elsewhere
void dispose_k(K* k) {
	assert(k != NULL);
	assert(k->refs == 0);

	if (printDebug) { printf("Dead term {%s}\n", KToString(k)); }

	if (checkTermSize) {
		if (k->args->len > 50) {
			panic("Sanity check failed!");
		}
	}

	// since k is dead, we can remove one ref from any of its children
	for (int i = 0; i < k->args->len; i++) {
		K* arg = k->args->a[i];
		Dec(arg);
	}

	// if (garbage_k_pending_next < MAX_GARBAGE_PENDING) {
	// 	garbage_k_pending[garbage_k_pending_next] = k;
	// 	garbage_k_pending_next++;
	// }

	// if (garbage_k_pending_next == MAX_GARBAGE_PENDING) {
	// 	for (int i = 0; i < MAX_GARBAGE_PENDING; i++) {
	// 		// printf("doing %d of %d\n", i, MAX_GARBAGE_PENDING);
	// 		// printf("%s\n", KToString(garbage_k_pending[i]));
	// 		assert(garbage_k_pending[i] != NULL);
	// 		assert(garbage_k_pending[i]->refs == 0);
	// 		dispose_k_aux(garbage_k_pending[i]);
	// 	}
	// 	garbage_k_pending_next = 0;
	// 	// printf("Completed a cleansing\n");
	// }
	
	dispose_k_aux(k);
	
	// fflush(stdout);
}

void Dec(K* k) {
	assert(k != NULL);
	assert(k->refs >= 1);

	k->refs--;
	int newRefs = k->refs;
	// if (checkRefCounting) {
	// 	if (newRefs < 0) {
	// 		panic("Term %s has fewer than 0 refs :(", KToString(k));
	// 	}
	// }
	if (newRefs == 0) {
		// printf("Dead term found: %s", KToString(k));
		// ListK* args = k->args;

		// // since k is dead, we can remove one ref from any of its children
		// for (int i = 0; i < args->len; i++) {
		// 	K* arg = args->a[i];
		// 	Dec(arg);
		// }

		dispose_k(k);
	}
}

ListK* copyArgs(ListK* oldArgs) {
	ListK* args = getDeadList(oldArgs->cap);
	if (args == NULL) {
		args = mallocArgs();
		args->cap = oldArgs->cap;
		args->len = oldArgs->len;
		args->a = mallocArgsA(oldArgs->cap);
	}

	for (int i = 0; i < oldArgs->len; i++) {
		args->a[i] = oldArgs->a[i];
	}

	return args;
}

K* copy(K* oldK) {
	ListK* newArgs = copyArgs(oldK->args);
	// K* k = k_new(copyLabel(oldK->label), newArgs);
	K* k = k_new(oldK->label, newArgs);
	if (printDebug) {
		printf("   New Old: %s\n", KToString(oldK));
		printf("   New Copy: %s\n", KToString(k));
	}
	return k;
}

// updates an arg from a k to another k
// sometimes a copy needs to be made, and this function does not Dec() the old k, so make sure you do
K* k_set_arg(K* orig_k, int arg, K* newVal) {
	K* k = orig_k;
	if (printDebug) {
		printf("Updating %s's %d argument to %s\n", KToString(k), arg, KToString(newVal));
	}
	if (k->refs > 1) {
		if (printDebug) {
			printf("   Term is shared, need to copy\n");
		}
		k = copy(orig_k);
	}
	Inc(newVal);
	K* orig_arg = k->args->a[arg];
	k->args->a[arg] = newVal;
	if (printDebug) {
		printf("   After updating: %s\n", KToString(k));
	}
	Dec(orig_arg);
	// if (k != orig_k) {
		
	// 	printf("Was %s\n", KToString(orig_k));
	// 	printf("Now %s\n\n", KToString(k));
	// 	// Dec(orig_k);
	// } else {
	// 	// Dec(orig_arg);
	// }

	return k;
}

K* updateTrimArgs(K* k, int left, int right) {
	if (k->refs > 1) {
		if (printDebug) { 
			printf("   Term is shared, need to copy\n");
		}
		k = copy(k);
	}
	for (int i = 0; i < left; i++) {
		Dec(k->args->a[i]);
	}
	for (int i = right; i < k->args->len; i++) {
		Dec(k->args->a[i]);
	}
	// TODO: inefficient
	int newi = 0;
	for (int i = left; i < right; i++) {
		k->args->a[newi] = k->args->a[i];
		newi++;
	}
	k->args->len = right - left;
	// k.args = k.args[left:right];
	return k;
}

void counts_aux(K* k, countentry **counts) {
	countentry *find;
	HASH_FIND_INT(*counts, &k, find);
	if (find == NULL) {
		countentry *new = malloc(sizeof(*new));
	 	new->entry = k;
	 	new->count = 1;
	 	HASH_ADD_INT(*counts, entry, new);

	 	for (int i = 0; i < k->args->len; i++) {
			K* arg = k->args->a[i];
			counts_aux(arg, counts);
		}
	} else {
		find->count++;
		// printf("Adding one to %s's count\n", KToString(k));
	}
}

countentry** counts(K* k) {
	countentry** counts = malloc(sizeof(*counts));
	*counts = NULL;

	counts_aux(k, counts);
	return counts;
}

void countentry_delete_all(countentry** counts) {
	countentry *s;
	countentry *tmp;

	HASH_ITER(hh, *counts, s, tmp) {
		HASH_DEL(*counts, s);  /* delete; users advances to next */
		free(s);            /* optional- if you want to free  */
	}
}

void dump_garbage_info() {
	printf("-----Garbage Dump-----\n");
	printf("\ncount_malloc_k: %d\n", count_malloc_k);
	printf("garbage_k_next: %d\n\n", garbage_k_next);

	for (int i = 0; i < MAX_GARBAGE_ARG_LEN; i++) {
		for (int j = 0; j < garbage_listk_nexts[i]; j++) {
			ListK* args = garbage_listk[i][j];
			if (args->len > 0) {
				if (args->cap < MAX_MALLOC_LISTK_ARRAY_SIZE) {
					count_malloc_listk_array[args->cap]--;
				}
				// free(args->a);
			}
			count_malloc_listk--;
			// free(args);
		}
	}
	// deadListsLen[0] = 0;
	// deadListsLen[1] = 0;
	// deadListsLen[2] = 0;
	// deadListsLen[3] = 0; // for some reason this is causing a segfault with clang
	// deadListsLen[4] = 0;

	// for (int i = 0; i < MAX_GARBAGE_ARG_LEN-1; i++) {
	// 	deadListsLen[i] = 0;
	// 	// printf("%d\n", deadListsLen[i]);
	// }
	for (int i = 0; i < MAX_MALLOC_LISTK_ARRAY_SIZE; i++) {
		printf("count_malloc_listk_array %d: %d\n", i, count_malloc_listk_array[i]);
	}
	for (int i = 0; i < MAX_GARBAGE_ARG_LEN; i++) {
		printf("garbage_listk_nexts %d: %d\n", i, garbage_listk_nexts[i]);
	}
	printf("count_malloc_listk: %d\n\n", count_malloc_listk);

	dump_label_garbage_info();

	printf("----------------------\n");
}

int get_symbol(label_helper lh, char* name) {
	for (int i = 0; i < lh.count; i++) {
		if (strcmp(name, lh.labels[i]) == 0) {
			return i;
		}
	}
	panic("Couldn't find symbol %s", name);
}

K* aterm_to_k(aterm at, label_helper lh, K* hole);

K* aterm_file_to_k(FILE* file, label_helper lh, K* hole) {
	aterm* at = at_parse(file);
	if (at == NULL) {
		panic("something went wrong with parsing!");
	}
	// printf("%s\n", aterm_to_string(*at));
	K* ret = aterm_to_k(*at, lh, hole);
	return ret;
}

ListK* aterm_list_to_args(at_list* l, label_helper lh, K* hole) {
	int count = 0;
	at_list* start = l;

	while (l != NULL) {
		l = l->next;
		count++;
	}

	if (count == 0) {
		return emptyArgs();
	}

	K** args = malloc(count * sizeof(K*));
	l = start;
	for (int i = 0; i < count; i++) {
		args[count - 1 - i] = aterm_to_k(*l->item, lh, hole);
		l = l->next;
	}

	return newArgs_array(count, args);
}

K* aterm_to_k(aterm at, label_helper lh, K* hole) {
	switch (at.type) {
		case AT_INT64: {
			return new_builtin_int(at.int64);
		}
		case AT_STRING: {
			return new_builtin_string(at.string);
		}
		case AT_APPL: {
			if (strcmp(at.appl.name, "#Int") == 0 || strcmp(at.appl.name, "#String") == 0) {
				return aterm_to_k(*at.appl.args->item, lh, hole);
			}
			if (strcmp(at.appl.name, "#Hole") == 0) {
				return hole;
			}

			int symbol = get_symbol(lh, at.appl.name);
			ListK* args = aterm_list_to_args(at.appl.args, lh, hole);
			return k_new(SymbolLabel(symbol), args);
		}
		default: {
			panic("Missing case!");
		}
	}
}
