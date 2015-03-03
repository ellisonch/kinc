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

extern void k_language_init(); // FIXME

void _k_set_arg(K* k, int i, K* v);

int garbage_k_next = 0;
K* garbage_k[MAX_GARBAGE_KEPT];

double garbage_get_capacity() {
	return (double) garbage_k_next / MAX_GARBAGE_KEPT;
}

int count_malloc_listk;
int count_malloc_listk_array;

int count_malloc_k = 0;

K** _mallocArgsA(int count) {
	if (count > MAX_GARBAGE_ARG_LEN) {
		panic("Don't handle args above %d", MAX_GARBAGE_ARG_LEN);
	}
	count_malloc_listk_array++;
	K** a = malloc(sizeof(K*) * MAX_GARBAGE_ARG_LEN);

	if (helpSafety) {
		for (int i = 0; i < MAX_GARBAGE_ARG_LEN; i++) {
			a[i] = NULL;
		}
	}
	return a;
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
	K* k = NULL;

	if (cap > MAX_GARBAGE_ARG_LEN) {
		panic("Don't handle args above %d", MAX_GARBAGE_ARG_LEN);
	}

	if (garbage_k_next > 0) {
		int top = garbage_k_next - 1;
		k = garbage_k[top];
		garbage_k_next--;
	} else {
		k = mallocK();
		k->args.a = _mallocArgsA(cap);
		k->args.cap = MAX_GARBAGE_ARG_LEN;
	}
	// this is the only place that new k gets its first and end set
	k->args.pos_first = 0;
	k->args.pos_end = len;

	assert(k != NULL);
	assert(k_num_args(k) == len);
	assert(k->args.cap >= cap);
	return k;
}

K* _k_fresh(KLabel* label, int len) {
	assert(label != NULL);
	assert(len >= 0);
	
	K* newK = _k_acquire(len, len);
	assert(newK != NULL);
	assert(k_num_args(newK) >= len);

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
		assert(arg != NULL);
		_k_set_arg(k, i, arg);
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
 		_k_set_arg(k, i, arg);
		Inc(arg);
	}
	va_end(ap);

	return k;
}

// TODO: leaks memory and is unsafe
char* ListKToString(const K* k) {
	assert(k != NULL);
	char* ret;
	ret = malloc(3000);
	ret[0] = '\0';

	for (int i = 0; i < k_num_args(k); i++) {
		K* arg = k_get_arg(k, i);
		char* sk = KToString(arg);
		strcat(ret, sk);
		free(sk);
		if (i < k_num_args(k) - 1) {
			strcat(ret, ", ");
		}
	}
	
	return ret;
}

// TODO: leaks memory and is unsafe
char* KToString(const K* k) {
	char* s = malloc(1000);
	if (k == NULL) {
		strcpy(s, "(null)");
		return s;
	}
	char* sargs = ListKToString(k);
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
	
	// free(args);
}

void dispose_k_itself(K* k) {
	assert(k != NULL);

	if (garbage_k_next < MAX_GARBAGE_KEPT) {
		if (printDebug) { printf("Saving k\n"); }
		garbage_k[garbage_k_next++] = k;
	} else {
		if (printDebug) { printf("Freeing k\n"); }
		free_args(&k->args);
		k->args.a = NULL;
		// k->args = NULL;
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
	// assert(k->args != NULL);
	assert(k->args.a != NULL);

	if (k->permanent) {
		return;
	}

	if (printDebug) {
		char* sk = KToString(k);
		printf("Dead term {%s}\n", sk);
		free(sk);
	}

	// since k is dead, we can remove one ref from any of its children
	for (int i = 0; i < k_num_args(k); i++) {
		K* arg = k_get_arg(k, i);
		_k_set_arg(k, i, NULL);
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

K* copy(const K* oldK) {
	K* k = k_new_array(oldK->label, k_num_args(oldK), &oldK->args.a[oldK->args.pos_first]);
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

void dump_garbage_info() {
	printf("-----Garbage Dump-----\n");
	printf("\ncount_malloc_k: %d\n", count_malloc_k);
	printf("garbage_k_next: %d\n\n", garbage_k_next);

	printf("count_malloc_listk_array: %d\n", count_malloc_listk_array);
	printf("count_malloc_listk: %d\n\n", count_malloc_listk);

	dump_label_garbage_info();

	printf("----------------------\n");
}

int get_symbol(label_helper lh, const char* name) {
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

K* k_get_arg(const K* k, int i) {
	assert(k != NULL);
	assert(i >= 0);
	assert(k_num_args(k) > i);
	assert(k->args.pos_first + i < k->args.cap);

	// K* item = k->args.a[i];
	K* item = k->args.a[k->args.pos_first + i];

	assert(item != NULL);
	assert(item->refs > 0);
	return item;
}

int k_num_args(const K* k) {
	assert(k != NULL);
	// assert(k->args != NULL);

	return k->args.pos_end - k->args.pos_first;
}

void k_set_label(K* k, KLabel* l) {
	// KLabel* oldl = k->label;
	k->label = l;
	// FIXME: not recovering memory from old label
	// dispose_label(oldl);
}

void _k_set_arg(K* k, int i, K* v) {
	assert(k != NULL);
	assert(i >= 0);
	// assert(k->args != NULL);
	assert(k_num_args(k) > i);
	assert(k->args.pos_first + i < k->args.cap);

	// k->args.a[i] = v;
	k->args.a[k->args.pos_first + i] = v;
}

// updates an arg from a k to another k
// sometimes a copy needs to be made, and this function does not Dec() the old k, so make sure you do
// need to copy about 30% of time
K* k_replace_arg(K* k, int arg, K* ov, K* nv) {
	assert(k != NULL);
	assert(ov != NULL);
	assert(nv != NULL);

	if (printDebug) {
		char* sold = KToString(k);
		char* snew = KToString(nv);
		printf("Updating %s's %d argument to %s\n", sold, arg, snew);
		free(sold);
		free(snew);
	}
	if (k->refs > 1) {
		if (printDebug) {
			printf("   Term is shared, need to copy\n");
		}
		k = copy(k);
	}
	Inc(nv);
	_k_set_arg(k, arg, nv);
	if (printDebug) {
		char* sk = KToString(k);
		printf("   After updating: %s\n", sk);
		free(sk);
	}
	Dec(ov);

	return k;
}

// returns a new k with args k[left] ... k[right-1]
K* updateTrimArgs(K* k, int left, int right) {
	assert(k != NULL);
	assert(left >= 0);
	assert(right <= k_num_args(k));

	if (k->refs > 1) {
		if (printDebug) {
			printf("   Term is shared, need to copy\n");
		}
		k = copy(k);
	}
	for (int i = 0; i < left; i++) {
		Dec(k_get_arg(k, i));
		_k_set_arg(k, i, NULL); // for safety
	}
	for (int i = right; i < k_num_args(k); i++) {
		Dec(k_get_arg(k, i));
		_k_set_arg(k, i, NULL); // for safety
	}

	int new_first = k->args.pos_first + left;
	int new_end = k->args.pos_first + right;

	k->args.pos_first = new_first;
	k->args.pos_end = new_end;

	return k;

	// // TODO: inefficient
	// int newi = 0;
	// for (int i = left; i < right; i++) {
	// 	_k_set_arg(k, newi, k_get_arg(k, i)); 
	// 	newi++;
	// }
	// k->args.len = right - left;
	// // k.args = k.args[left:right];
	// return k;
}

void k_make_permanent(K* k) {
	k->permanent = 1;
}


void k_init() {
	k_init_builtins();
	k_language_init();

	// preallocate garbage; doesn't seem to help
	// for (int i = 0; i < 100; i++) {
	// 	garbage_k[i] = _k_acquire(0, 0);
	// 	garbage_k_next++;
	// }
}
