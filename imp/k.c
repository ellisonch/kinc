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

// TODO: should have a private header
void _k_set_arg(K* k, int i, K* v);
K* _copy_if_necessary(K* k);

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
	if (count_malloc_k > MAX_OUTSTANDING_K) {
		panic("Refuse to allocate more than %d outstanding Ks", MAX_OUTSTANDING_K);
	}
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
	k->args.pos_first = 20;
	k->args.pos_end = len + 20;

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
	return k_new_from_array(label, 0, NULL);
}

K* k_new_from_k_args(KLabel* label, K* v) {
	K* k = _k_fresh(label, k_num_args(v));

	for (int i = 0; i < k_num_args(k); i++) {
		K* arg = k_get_arg(v, i);
		assert(arg != NULL);
		_k_set_arg(k, i, arg);
		Inc(arg);
	}

	return k;
}

K* k_new_from_array(KLabel* label, int count, K** a) {
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
		assert(arg != NULL);
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
	assert(k->refs == 0);

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


void k_dispose(K* k) {
	dispose_k(k);
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
	assert(oldK != NULL);
	if (printDebug) {
		printf("going to do copy() on %s\n", KToString(oldK));
	}
	K* k = k_new_from_array(oldK->label, k_num_args(oldK), &oldK->args.a[oldK->args.pos_first]);
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
			if (
				strcmp(at.appl.name, "#Int") == 0 
				|| strcmp(at.appl.name, "#String") == 0) 
			{
				return aterm_to_k(*at.appl.args->item, lh, hole);
			}
			if (strcmp(at.appl.name, "#false") == 0) {
				return k_builtin_false();
			}
			if (strcmp(at.appl.name, "#true") == 0) {
				return k_builtin_true();
			}
			if (strcmp(at.appl.name, "#Hole") == 0) {
				return hole;
			}

			int symbol = get_symbol(lh, at.appl.name);
			K** a;
			int count = aterm_list_to_args(at.appl.args, lh, hole, &a);
			K* k = k_new_from_array(SymbolLabel(symbol), count, a);
			free(a);
			return k;
		}
		default: {
			panic("Missing case!");
		}
	}
}

K* _k_get_arg(const K* k, int i) {
	assert(k != NULL);
	assert(i >= 0);
	if (k_num_args(k) <= i) {
		printf("Trying to get %dth argument of %s\n", i, KToString(k));
	}
	assert(k_num_args(k) > i);
	assert(k->args.pos_first + i < k->args.cap);

	// K* item = k->args.a[i];
	K* item = k->args.a[k->args.pos_first + i];

	return item;
}

K* k_get_arg(const K* k, int i) {
	assert(k != NULL);
	assert(i >= 0);
	if (k_num_args(k) <= i) {
		printf("Trying to get %dth argument of %s\n", i, KToString(k));
	}
	assert(k_num_args(k) > i);
	assert(k->args.pos_first + i < k->args.cap);

	K* item = _k_get_arg(k, i);

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
	k = _copy_if_necessary(k);

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

	k = _copy_if_necessary(k);

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

	assert(k_num_args(k) == right - left);
	return k;
}

// returns a new k with args k[left] ... k[pos_end-1]
K* k_without_first_n_arg(K* k, int left) {
	assert(k != NULL);
	assert(left >= 0);

	k = copy(k);

	int old_length = k_num_args(k);

	for (int i = 0; i < left; i++) {
		Dec(k_get_arg(k, i));
		_k_set_arg(k, i, NULL); // for safety
	}

	int new_first = k->args.pos_first + left;

	k->args.pos_first = new_first;

	assert(k_num_args(k) == old_length - left);
	return k;
}

// seems much faster
// void _k_remove_first_arg(K* k) {
// 	assert(k != NULL);
// 	assert(k_num_args(k) > 0);
// 	assert(k->refs == 1); // don't want this, but necessary for now

// 	int old_length = k_num_args(k);

// 	Dec(k_get_arg(k, 0));
// 	_k_set_arg(k, 0, NULL); // for safety

// 	int new_first = k->args.pos_first + 1;
// 	k->args.pos_first = new_first;

// 	assert(k_num_args(k) == old_length - 1);
// }

K* _copy_if_necessary(K* k) {
	if (k->refs > 1) {
		if (printDebug) {
			printf("   Term is shared, need to copy\n");
		}
		k = copy(k);
	}
	return k;
}

K* _k_insert_space_after(K* k, int pos, int count) {
	assert(k != NULL);

	k = _copy_if_necessary(k);

	panic("not inserting space yet");
}


void _k_grow_front_arg(K* k) {
	assert(k != NULL);
	assert(k->refs == 1); // don't want this, but necessary for now

	if (k->args.pos_first > 0) {
		k->args.pos_first--;

		return;
	}
	panic("Not enough room to grow!");
}


K* k_insert_elems(K* k, int pos, int overwriteCount, int actualResultCount, int varargCount, ...) {
	va_list elems;
	va_start(elems, varargCount);

	k = k_insert_elems_vararg(k, pos, overwriteCount, actualResultCount, varargCount, elems);
	va_end(elems);
	return k;
}

// for k[start] to k[start + count-1], moves each to k[start + shift] ... k[count - 1 + shift] respectively
// FIXME: this is horrible
void _shift_args(K* k, int start, int count, int shift) {
	assert(k != NULL);
	assert(start >= 0);
	assert(count >= 0);

	K** temp = malloc(sizeof(K*) * k_num_args(k));

	// printf("Shifting; start=%d, count=%d, shift=%d\n", start, count, shift);
	for (int i = 0; i < count; i++) {
		temp[i] = k_get_arg(k, start + i);
	}
	for (int i = 0; i < count; i++) {
		_k_set_arg(k, start + i + shift, temp[i]);
	}

	// assert(start - shift >= 0);
	// assert(
	// memmove(&(k->args.a[k->args.pos_first + start]), &(k->args.a[k->args.pos_first + start + shift]), count * sizeof(K*));

	free(temp);
}

K* _k_extend(K* k, int count) {
	assert(k != NULL);
	// assert(k->refs == 1);

	k = _copy_if_necessary(k);
	if (printDebug) {
		printf("after copy in _k_extend()\n");
	}

	if (k->args.cap >= k->args.pos_end + count) {
		k->args.pos_end += count;
	} else {
		printf("cap: %d, pos_end: %d, want to add: %d\n", k->args.cap, k->args.pos_end, count);
		panic("Not handling true growth");
	}

	return k;
}
/*
	a, (b, c) => (d, e, f), g, h

	a, (b, c) => (d), g, h

	((a, b, c) => d), e, f

	a, (. => (b, c)), d
*/
// FIXME: not deccing old stuff
K* k_insert_elems_vararg(K* k, int pos, int overwriteCount, int actualResultCount, int varargCount, va_list elems) {
	assert(k != NULL);
	assert(k_num_args(k) >= pos);
	assert(pos >= 0);
	assert(overwriteCount >= 0);
	assert(varargCount >= 0);
	assert(pos + overwriteCount <= k_num_args(k));
	assert(k_num_args(k) >= overwriteCount);

	// FIXME: this is super gross.  copying to make sure elements get deleted if not being used in result
	K* fakeCopy = copy(k);

	int old_count = k_num_args(k);
	// printf("Old Length is %d\n", old_count);

	int number_added = actualResultCount - overwriteCount;
	// printf("Number added: %d\n", number_added);

	// if we need to kill args, go ahead and preemptively copy
	if (number_added < 0) {
		k = copy(k);
	}

	if (printDebug) {
		printf("k_insert_elems_vararg() at old_count=%d, pos=%d, owc=%d, ac=%d, varargCount=%d\n", old_count, pos, overwriteCount, actualResultCount, varargCount);
	}

	// int need_to_add = (pos + actualResultCount) - old_count;
	if (number_added > 0) {
		// printf("adding %d\n", number_added);
		if (printDebug) {
			printf("Going to do extend to grow some args\n");
		}
		k = _k_extend(k, number_added);
		assert(k_num_args(k) == old_count + number_added);
	}

	// printf("old_count=%d\n", old_count);


	// if (overwriteCount > actualResultCount) {
	int lastGoodArg = pos + overwriteCount;
	// printf("last good arg is %d\n", lastGoodArg);
	for (int i = pos; i < lastGoodArg; i++) {
		// printf("getting %d... ", i);
		K* arg = k_get_arg(k, i);
		// printf("Deccing %s\n", KToString(arg));
		Dec(arg);
		_k_set_arg(k, i, NULL);
	}	
	int shift = actualResultCount - overwriteCount;
	// printf("Shifting by %d\n", shift);
	_shift_args(k, lastGoodArg, old_count - lastGoodArg, shift);

	// int need_to_remove = (
	// if (old_count > lastGoodArg ) {
	// 	k->args.pos_end -= lastGoodArg - pos;
	// }

	if (number_added < 0) {
		// printf("removing %d\n", -number_added);
		if (printDebug) {
			printf("Going to do extend to squash some args\n");
		}
		k = _k_extend(k, number_added);
		assert(k_num_args(k) == old_count + number_added);
	}

	// printf("New Length is %d\n", k_num_args(k));

	// 	memmove(dest, &k->args.a[
		// panic("Not handling removing elements yet");
	// }

	// if (overwriteCount > actualResultCount) {, 
	// for (int srci = lastGoodArg; srci < old_count; srci++) {
	// 	int desti = srci + shift;
	// 	// memmove(k->args.pos_first

	// }
	// // 	memmove(dest, &k->args.a[
	// 	// panic("Not handling removing elements yet");
	// // }

	// int need_to_move = old_count - overwriteCount;
	// if (need_to_move > 0) {
	// 	printf("need to move: %d\n", need_to_move);
	// 	panic("not moving yet");
	// }

	// printf("need_to_add: %d\n", need_to_add);

	// int startIndex = pos;
	// int endIndex = pos + count;

	// printf("inserting at old_count=%d, pos=%d, owc=%d, ac=%d, varargCount=%d\n", old_count, pos, overwriteCount, actualResultCount, varargCount);
	// k = _k_insert_space_after(k, pos, varargCount);
	int numWrote = 0;
	int target = pos;
	for (int i = 0; i < varargCount; i++) {
		//printf("vararging... ");
		_Bool isList = va_arg(elems, ListOrNot) == E_LIST;
		K* arg = va_arg(elems, K*);
		// printf("done\n");
		assert(arg != NULL);
		// printf("arg: %s\n", KToString(arg));
		// printf("inserting %s %s at pos=%d, i=%d, owc=%d, ac=%d, varargCount=%d in %s\n", isList? " the args of " : "", KToString(arg), pos, i, overwriteCount, actualResultCount, varargCount, KToString(k));
		// printf("inserting %s %s at pos=%d, i=%d, owc=%d, ac=%d, varargCount=%d\n", isList? " the args of " : "", KToString(arg), pos, i, overwriteCount, actualResultCount, varargCount);

		if (isList) {
			for (int listArgi = 0; listArgi < k_num_args(arg); listArgi++) {
				// printf("Getting arg %d\n", listArgi);
				K* argi = k_get_arg(arg, listArgi);
				// printf("inserting %s at %d\n", KToString(argi), target);
				// K* oldArg = NULL;
				// if (numWrote < overwriteCount) {
				// 	oldArg = k_get_arg(k, target);
				// }
				_k_set_arg(k, target, argi);				
				Inc(argi);
				// if (numWrote < overwriteCount) {
				// 	Dec(oldArg);
				// }
				numWrote++;
				target++;
			}
			// panic("not handling lists yet");
		} else {
			// K* oldArg = NULL;
			// if (numWrote < overwriteCount) {
			// 	oldArg = k_get_arg(k, target);
			// }
			_k_set_arg(k, target, arg);
			Inc(arg);
			// if (numWrote < overwriteCount) {
			// 	Dec(oldArg);
			// }
			numWrote++;
			target++;
		}
		// printf("Inserted.\n");
	}

	// printf("printing k args...\n");
	// for (int i = 0; i < k_num_args(k); i++) {
	// 	printf("getting %d\n", i);
	// 	K* arg = _k_get_arg(k, i);
	// 	if (arg == NULL) {
	// 		printf("crap, null\n");
	// 	} else {
	// 		printf("%s\n", KToString(arg));
	// 	}
	// }
	k_dispose(fakeCopy);
	// printf("done\n");

	return k;
}

void k_remove_arg_head(K* k) {
	assert(k != NULL);
	assert(k_num_args(k) > 0);
	assert(k->refs == 1); // don't want this, but necessary for now

	// a nice safe way of doing this
	K* newk = updateTrimArgs(k, 1, k_num_args(k));
	assert(newk == k); // not really true, but true for a while

	// faster, but specialized
	// _k_remove_first_arg(k);

	// K* newk = k_remove_first_n_arg(k, 1);
	// Inc(newk);
	// assert(newk == k); // not really true, but true for a while
}

void k_set_arg(K* k, int i, K* v) {
	assert(k != NULL);
	assert(v != NULL);
	assert(i >= 0);
	assert(i < k_num_args(k));
	assert(k->refs == 1); // don't want this, but necessary for now

	K* oldv = k_get_arg(k, i);
	_k_set_arg(k, i, v);
	Inc(v);
	Dec(oldv);
}

void k_add_front_arg(K* k, K* v) {
	assert(k != NULL);
	assert(v != NULL);
	assert(k->refs == 1); // don't want this, but necessary for now

	_k_grow_front_arg(k);
	_k_set_arg(k, 0, v);
	Inc(v);
}

void k_make_permanent(K* k) {
	assert(k != NULL);

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
