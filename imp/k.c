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
// int garbage_listk_next = 0;
// ListK* garbage_listk[MAX_GARBAGE_KEPT];


int count_malloc_listk;
int count_malloc_listk_array;

int count_malloc_k = 0;

ListK* _mallocArgs() {
	count_malloc_listk++;
	return malloc(sizeof(ListK));
}

K** _mallocArgsA(int count) {
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

ListK* _listk_acquire(int len, int cap) {
	assert(len >= 0);
	assert(cap <= MAX_GARBAGE_ARG_LEN);

	ListK* args = _mallocArgs();
	args->a = _mallocArgsA(cap);
	args->cap = MAX_GARBAGE_ARG_LEN;
	args->len = len;

	assert(args != NULL);
	assert(args->len == len);
	assert(args->cap >= len);
	assert(args->cap <= MAX_GARBAGE_ARG_LEN);
	assert(args->a != NULL);
	return args;
}

K* Inner(K* k) {
	assert(k != NULL);
	assert(k->args != NULL);
	assert(k->args->len > 0);

	return k->args->a[0];
}

void Inc(K* k) {
	assert(k != NULL);

	k->refs++;
}

K* mallocK() {
	count_malloc_k++;
	return malloc(sizeof(K));
}

K* _k_acquire(int len, int cap) {
	K* newK = NULL;
	if (garbage_k_next > 0) {
		newK = garbage_k[garbage_k_next - 1];
		garbage_k_next--;
		assert(newK->args != NULL);
		newK->args->len = len;
	} else {
		newK = mallocK();
		ListK* args = _listk_acquire(len, cap);
		newK->args = args;
		assert(newK->args != NULL);
	}

	assert(newK != NULL);
	assert(newK->args != NULL);
	assert(newK->args->len == len);
	assert(newK->args->cap >= cap);
	return newK;
}

K* _k_fresh(KLabel* label, int len) {
	assert(label != NULL);
	assert(len >= 0);
	
	K* newK = _k_acquire(len, len);
	assert(newK != NULL);
	assert(newK->args != NULL);
	assert(newK->args->len >= len);

	newK->label = label;
	newK->refs = 0;
	newK->permanent = 0;
	return newK;
}

K* k_new_empty(KLabel* label) {
	return k_new_array(label, 0, NULL);
}

K* k_new_array(KLabel* label, int count, K** a) {
	K* k = _k_fresh(label, count);

	for (int i = 0; i < count; i++) {
		K* arg = a[i];
		if (arg == NULL) {
 			panic("Didn't expect NULL arg in k_new().  len: %d", count);
 		}
		k->args->a[i] = arg;
		Inc(arg);
	}

	return k;
}

K* k_new(KLabel* label, int count, ...) {
	K* k = _k_fresh(label, count);

	va_list ap;
	va_start(ap, count);
	for (int i = 0; i < count; i++) {
		K* arg = va_arg(ap, K*);
		if (arg == NULL) {
 			panic("Didn't expect NULL arg in k_new().  len: %d", count);
 		}
		k->args->a[i] = arg;
		Inc(arg);
	}
	va_end(ap);

	return k;
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

void free_args(ListK* args) {
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

void dispose_k_itself(K* k) {
	assert(k != NULL);

	if (garbage_k_next < MAX_GARBAGE_KEPT) {
		if (printDebug) { printf("Saving k\n"); }
		garbage_k[garbage_k_next++] = k;
	} else {
		if (printDebug) { printf("Freeing k\n"); }
		free_args(k->args);
		k->args = NULL;
		count_malloc_k--;
		free(k);
	}
}

// K* garbage_k_pending[MAX_GARBAGE_PENDING];
// int garbage_k_pending_next = 0;

void dispose_k_aux(K* k) {
	// dispose_args(k);
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
}

void Dec(K* k) {
	assert(k != NULL);
	assert(k->refs >= 1);

	k->refs--;
	int newRefs = k->refs;
	if (newRefs == 0) {
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

countentry** counts(int len, K** a) {
	countentry** counts = malloc(sizeof(*counts));
	*counts = NULL;

	for (int i = 0; i < len; i++) {
		K* k = a[i];
		counts_aux(k, counts);
	}
	
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

	printf("count_malloc_listk_array: %d\n", count_malloc_listk_array);
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
	at_free(at);
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
			return new_builtin_string(string_make_copy(at.string));
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
	// for (int i = 0; i < 100; i++) {
	// 	garbage_k[i] = _k_acquire(0, 0);
	// 	garbage_k_next++;
	// }
}
