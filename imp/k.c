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

int garbage_k_next = 0;
K* garbage_k[MAX_GARBAGE_KEPT];

// the next available garbage location
// int garbage_listk_nexts[MAX_GARBAGE_ARG_LEN+1];
// ListK* garbage_listk[MAX_GARBAGE_ARG_LEN+1][MAX_GARBAGE_KEPT];
int garbage_listk_next = 0;
ListK* garbage_listk[MAX_GARBAGE_KEPT];


int count_malloc_listk;
int count_malloc_listk_array;

int count_malloc_k = 0;

// int list_get_right_size_offset(int n) {
// 	if (n <= 16) {
// 		return 0;
// 	}
// 	return ceil_log2(n);
// }

// int list_get_right_size(int n) {
// 	if (n == 0) {
// 		return 1;
// 	}
// 	// if (n <= 16) {
// 	// 	return 16;
// 	// }
// 	return next_highest_power(n);
// }

ListK* getDeadList(int reqLength) {
	if (reqLength > MAX_GARBAGE_ARG_LEN) {
		return NULL;
	}
	if (garbage_listk_next == 0) {
		if (printDebug) { printf("No dead stuff of length %d\n", reqLength); }
		return NULL;
	}

	ListK* ret = garbage_listk[garbage_listk_next - 1];
	assert(ret != NULL);
	assert(ret->cap >= reqLength);

	garbage_listk_next--;
	// ret->len = reqLength;

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
	if (count > MAX_GARBAGE_ARG_LEN) {
		panic("Don't handle args above %d", MAX_GARBAGE_ARG_LEN);
	}
	count_malloc_listk_array++;
	// return malloc(sizeof(K*) * count);
	K** a = malloc(sizeof(K*) * MAX_GARBAGE_ARG_LEN);

	if (helpSafety) {
		for (int i = 0; i < MAX_GARBAGE_ARG_LEN; i++) {
			a[i] = NULL;
		}
	}
	return a;
}

ListK* listk_create(int len, int cap) {
	assert(len >= 0);
	assert(cap <= MAX_GARBAGE_ARG_LEN);

	ListK* args = mallocArgs();
	args->a = mallocArgsA(cap);
	args->cap = MAX_GARBAGE_ARG_LEN;
	args->len = len;
	return args;
}

ListK* listk_acquire(int len, int cap) {
	// printf("%d, %d\n", len, cap);
	ListK* args = getDeadList(cap);
	if (args == NULL) {
		args = listk_create(len, cap);
	} else {
		args->len = len;
	}

	assert(args != NULL);
	assert(args->len == len);
	assert(args->cap >= len);
	assert(args->cap <= MAX_GARBAGE_ARG_LEN);
	assert(args->a != NULL);
	return args;
}

ListK* newArgs_array(int count, K** a) {
	ListK* args = listk_acquire(count, count);

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
	ListK* args = listk_acquire(0, 0);
	return args;
}

K* k_acquire(int len, int cap) {
	K* newK = NULL;
	if (garbage_k_next > 0) {
		newK = garbage_k[garbage_k_next - 1];
		garbage_k_next--;
	} else {
		newK = mallocK();
	}
	// newK->args = listk_acquire(len, cap);

	assert(newK != NULL);
	return newK;
}


K* _k_new(KLabel* label, ListK* args) {
	assert(label != NULL);
	assert(args != NULL);
	assert(args->a != NULL);

	for (int i = 0; i < args->len; i++) {
 		K* arg = args->a[i];
 		if (arg == NULL) {
 			panic("Didn't expect NULL arg in k_new().  len: %d, cap: %d", args->len, args->cap);
 		}
 		Inc(arg);
 	}
	
	K* newK = k_acquire(args->len, args->len);
	assert(newK != NULL);
	newK->label = label;
	newK->args = args;
	newK->refs = 0;
	newK->permanent = 0;
	return newK;
}

K* k_new_empty(KLabel* label) {
	return _k_new(label, emptyArgs());
}

K* k_new_array(KLabel* label, int count, K** a) {
	ListK* args = listk_acquire(count, count);

	for (int i = 0; i < count; i++) {
		args->a[i] = a[i];
	}

	return _k_new(label, args);
}

K* k_new(KLabel* label, int count, ...) {
	ListK* args = listk_acquire(count, count);

	va_list ap;
	va_start(ap, count);
	for (int i = 0; i < count; i++) {
		args->a[i] = va_arg(ap, K*);
	}
	va_end(ap);

	return _k_new(label, args);
}

// TODO: leaks memory and is unsafe
char* ListKToString(ListK* args) {
	char* ret;
	if (args == NULL) {
		return string_make_copy("");
	}
	ret = malloc(3000);
	ret[0] = '\0';

	// size_t len = strlen(ret);
	for (int i = 0; i < args->len; i++) {
		K* arg = args->a[i];
		char* sk = KToString(arg);
		strcat(ret, sk);
		free(sk);
		if (i < args->len - 1) {
			strcat(ret, ", ");
		}
	}
	
	return ret;
}

// TODO: leaks memory and is unsafe
char* KToString(K* k) {
	char* s = malloc(1000);
	if (k == NULL) {
		strcpy(s, "(null)");
		return s;
	}
	char* sargs = ListKToString(k->args);
	char* slabel = LabelToString(k->label);
	if (printRefCounts) {
		snprintf(s, 1000, "%s[%d](%s)", slabel, k->refs, sargs);
	} else {
		snprintf(s, 1000, "%s(%s)", slabel, sargs);
	}
	free(sargs);
	free(slabel);
	return s;
}

void terminate_args(ListK* args) {
	assert(args != NULL);
	assert(args->a != NULL);
	assert(count_malloc_listk >= 1);

	if (printDebug) { printf("Freeing args\n"); }		

	count_malloc_listk--;
	count_malloc_listk_array--;
	free(args->a);
	args->a = NULL;
	
	free(args);
}

int COMPLAINED_MAX_GARBAGE_ARG_LEN = 0;
// we're done with a ListK, and we need to see if we can reuse it
void dispose_args(K* k) {	
	assert(k != NULL);
	assert(k->args != NULL);

	ListK* args = k->args;
	k->args = NULL;

	// if we already have too many args of this many arguments, then kill it
	assert(garbage_listk_next <= MAX_GARBAGE_KEPT);
	if (garbage_listk_next == MAX_GARBAGE_KEPT) {		
		terminate_args(args);
		return;
	}

	// save it
	garbage_listk[garbage_listk_next] = args;	
	garbage_listk_next++;

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
	assert(k->args != NULL);
	assert(k->args->a != NULL);

	if (k->permanent) {
		return;
	}

	if (printDebug) {
		char* sk = KToString(k);
		printf("Dead term {%s}\n", sk);
		free(sk);
	}

	if (checkTermSize) {
		if (k->args->len > 50) {
			panic("Sanity check failed!");
		}
	}

	// since k is dead, we can remove one ref from any of its children
	for (int i = 0; i < k->args->len; i++) {
		K* arg = k->args->a[i];		
		k->args->a[i] = NULL;
		assert(arg != NULL);
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

K* copy(K* oldK) {
	K* k = k_new_array(oldK->label, oldK->args->len, oldK->args->a);
	if (printDebug) {
		char* sold = KToString(oldK);
		char* snew = KToString(k);
		printf("   New Old: %s\n", sold);
		printf("   New Copy: %s\n", snew);
		free(sold);
		free(snew);
	}
	return k;
}


// updates an arg from a k to another k
// sometimes a copy needs to be made, and this function does not Dec() the old k, so make sure you do
K* k_set_arg(K* orig_k, int arg, K* newVal) {
	K* k = orig_k;
	if (printDebug) {
		char* sold = KToString(k);
		char* snew = KToString(newVal);
		printf("Updating %s's %d argument to %s\n", sold, arg, snew);
		free(sold);
		free(snew);
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
		char* sk = KToString(k);
		printf("   After updating: %s\n", sk);
		free(sk);
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

	// for (int i = 0; i < MAX_GARBAGE_ARG_LEN; i++) {
	// 	for (int j = 0; j < garbage_listk_nexts[i]; j++) {
	// 		ListK* args = garbage_listk[i][j];
	// 		if (args->len > 0) {
	// 			if (args->cap < MAX_MALLOC_LISTK_ARRAY_SIZE) {
	// 				count_malloc_listk_array[args->cap]--;
	// 			}
	// 			// free(args->a);
	// 		}
	// 		count_malloc_listk--;
	// 		// free(args);
	// 	}
	// }
	// deadListsLen[0] = 0;
	// deadListsLen[1] = 0;
	// deadListsLen[2] = 0;
	// deadListsLen[3] = 0; // for some reason this is causing a segfault with clang
	// deadListsLen[4] = 0;

	// for (int i = 0; i < MAX_GARBAGE_ARG_LEN-1; i++) {
	// 	deadListsLen[i] = 0;
	// 	// printf("%d\n", deadListsLen[i]);
	// }
	printf("count_malloc_listk_array: %d\n", count_malloc_listk_array);
	// for (int i = 0; i < MAX_MALLOC_LISTK_ARRAY_SIZE; i++) {
	// 	printf("count_malloc_listk_array %d: %d\n", i, count_malloc_listk_array[i]);
	// }
	// for (int i = 0; i < MAX_GARBAGE_ARG_LEN; i++) {
	// 	printf("garbage_listk_nexts %d: %d\n", i, garbage_listk_nexts[i]);
	// }
	printf("garbage_listk_next: %d\n", garbage_listk_next);
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


int aterm_list_to_args(at_list* l, label_helper lh, K* hole, K*** ret) {
	int count = 0;
	at_list* start = l;

	while (l != NULL) {
		l = l->next;
		count++;
	}

	if (count == 0) {
		*ret = NULL;
		return 0;
	}

	K** args = malloc(count * sizeof(K*));
	l = start;
	for (int i = 0; i < count; i++) {
		args[count - 1 - i] = aterm_to_k(*l->item, lh, hole);
		l = l->next;
	}

	// *ret = newArgs_array(count, args);
	*ret = args;
	return count;
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
			K** a;
			int count = aterm_list_to_args(at.appl.args, lh, hole, &a);
			K* k = k_new_array(SymbolLabel(symbol), count, a);
			free(a);
			return k;
		}
		default: {
			panic("Missing case!");
		}
	}
}


void k_init() {
	k_init_builtins();

	// preallocate garbage; doesn't seem to help
	// for (int i = 0; i < MAX_GARBAGE_KEPT; i++) {
	// 	garbage_k[i] = mallocK();
	// 	garbage_k_next++;

	// 	garbage_listk[i] = listk_create(0, 0);
	// 	garbage_listk_next++;
	// }
	// assert(garbage_k_next == MAX_GARBAGE_KEPT);
	// assert(garbage_listk_next == MAX_GARBAGE_KEPT);
}
