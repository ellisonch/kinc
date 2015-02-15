#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>

#include "k.h"
#include "k_labels.h"
#include "settings.h"
#include "utils.h"
#include "uthash.h"


#define MAX_GARBAGE_KEPT 10000

// when printing k terms, print the ref counts as well
#define printRefCounts 1


K* deadK[MAX_GARBAGE_KEPT];
int deadlen = 0;

ListK* deadLists[MAX_GARBAGE_ARG_LEN+1][MAX_GARBAGE_KEPT];
int deadListsLen[MAX_GARBAGE_ARG_LEN+1];

int mallocedArgs;
int malloced[10];
int mallocedK = 0;

ListK* getDeadList(int reqLength) {
	// return NULL;
	if (reqLength > MAX_GARBAGE_ARG_LEN) {
		return NULL;
	}
	ListK** deadList = deadLists[reqLength];
	if (deadListsLen[reqLength] == 0) {
		if (printDebug) { printf("No dead stuff of length %d\n", reqLength); }
		return NULL;
	}

	ListK* ret = deadList[deadListsLen[reqLength] - 1];
	deadListsLen[reqLength]--;

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
	mallocedArgs++;
	return malloc(sizeof(ListK));
}

K** mallocArgsA(int count) {
	malloced[count]++;
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

// TODO unsafe
K* Inner(K* k) {
	return k->args->a[0];
}



void Inc(K* k) {
	k->refs++;
}

K* mallocK() {
	mallocedK++;
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

K* NewK(KLabel* label, ListK* args) {
	if (args == NULL) {
		args = emptyArgs();
	}
	for (int i = 0; i < args->len; i++) {
 		K* arg = args->a[i];
 		if (arg == NULL) {
 			panic("Didn't expect nil arg in NewK()");
 		}
 		Inc(arg);
 	}
	
	K* newK = NULL;
	if (deadlen > 0) {
		newK = deadK[deadlen - 1];
		deadlen--;
	} else {
		newK = mallocK();
	}
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
	char* s = malloc(300);
	if (printRefCounts) {
		snprintf(s, 300, "%s[%d](%s)", LabelToString(k->label), k->refs, ListKToString(k->args));
	} else {
		snprintf(s, 300, "%s(%s)", LabelToString(k->label), ListKToString(k->args));
	}
	return s;
}

void dispose_k(K* k) {
	if (printDebug) { printf("Dead term {%s}\n", KToString(k)); }

	if (checkTermSize) {
		if (k->args->len > 10) {
			panic("Sanity check failed!");
		}
	}
	for (int i = 0; i < k->args->len; i++) {
		K* arg = k->args->a[i];
		Dec(arg);
	}

	int lenkargs = k->args->cap;

	if (lenkargs >= MAX_GARBAGE_ARG_LEN) {
		if (printDebug) {
			printf("MAX_GARBAGE_ARG_LEN is not enough for dead term with len %d\n", lenkargs);
		}		
	}

	if (lenkargs < MAX_GARBAGE_ARG_LEN && deadListsLen[lenkargs] < MAX_GARBAGE_KEPT) {
		deadLists[lenkargs][deadListsLen[lenkargs]] = k->args;
		deadListsLen[lenkargs]++;
		if (printDebug) { printf("Saving args\n"); }
	} else {
		mallocedArgs--;
		if (k->args->cap > 0) {
			malloced[k->args->cap]--;
			free(k->args->a);
		}
		if (printDebug) { printf("Freeing args\n"); }		
		free(k->args);
	}

	if (k->label->type != e_symbol) {
		dispose_label(k->label);
	}
	if (deadlen < MAX_GARBAGE_KEPT) {
		if (printDebug) { printf("Saving k\n"); }
		deadK[deadlen++] = k;
	} else {
		if (printDebug) { printf("Freeing k\n"); }
		mallocedK--;
		free(k);
	}
}

void Dec(K* k) {
	// panic("don't handle dec");
	k->refs--;
	int newRefs = k->refs;
	if (checkRefCounting) {
		if (newRefs < 0) {
			panic("Term %s has fewer than 0 refs :(", KToString(k));
		}
	}
	if (newRefs == 0) {
		// printf("Dead term found: %s", KToString(k));
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
	// K* k = NewK(copyLabel(oldK->label), newArgs);
	K* k = NewK(oldK->label, newArgs);
	if (printDebug) {
		printf("   New Old: %s\n", KToString(oldK));
		printf("   New Copy: %s\n", KToString(k));
	}
	return k;
}

K* UpdateArg(K* k, int arg, K* newVal) {
	if (printDebug) {
		printf("Updating %s's %d argument to %s\n", KToString(k), arg, KToString(newVal));
	}
	if (k->refs > 1) {
		if (printDebug) {
			printf("   Term is shared, need to copy\n");
		}
		// K* oldk = k;
		k = copy(k);
	}
	Inc(newVal);
	Dec(k->args->a[arg]);
	k->args->a[arg] = newVal;
	if (printDebug) {
		printf("   After updating: %s\n", KToString(k));
	}
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
	printf("\nMallocedK: %d\n", mallocedK);
	printf("deadlen: %d\n\n", deadlen);

	for (int i = 0; i < MAX_GARBAGE_ARG_LEN; i++) {
		for (int j = 0; j < deadListsLen[i]; j++) {
			ListK* args = deadLists[i][j];
			if (args->len > 0) {
				malloced[args->cap]--;
				free(args->a);
			}
			mallocedArgs--;
			free(args);
		}
	}
	for (int i = 0; i < MAX_GARBAGE_ARG_LEN; i++) {
		deadListsLen[i] = 0;
	}
	for (int i = 0; i < 8; i++) {
		printf("args %d: %d\n", i, malloced[i]);
	}
	for (int i = 0; i < MAX_GARBAGE_ARG_LEN; i++) {
		printf("deadargs %d: %d\n", i, deadListsLen[i]);
	}
	printf("Mallocedargs: %d\n\n", mallocedArgs);

	dump_label_garbage_info();

	printf("----------------------\n");
	// printf("intcount: %d\n", intcount);

}
