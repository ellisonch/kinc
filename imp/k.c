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
#include "aparser/aterm.h"


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
	// deadListsLen[0] = 0;
	// deadListsLen[1] = 0;
	// deadListsLen[2] = 0;
	// deadListsLen[3] = 0; // for some reason this is causing a segfault with clang
	// deadListsLen[4] = 0;

	// for (int i = 0; i < MAX_GARBAGE_ARG_LEN-1; i++) {
	// 	deadListsLen[i] = 0;
	// 	// printf("%d\n", deadListsLen[i]);
	// }
	for (int i = 0; i < 8; i++) {
		printf("args %d: %d\n", i, malloced[i]);
	}
	for (int i = 0; i < MAX_GARBAGE_ARG_LEN; i++) {
		printf("deadargs %d: %d\n", i, deadListsLen[i]);
	}
	printf("Mallocedargs: %d\n\n", mallocedArgs);

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
	printf("%s\n", aterm_to_string(*at));
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
			return NewK(SymbolLabel(symbol), args);

			// NewK(SymbolLabel(symbol_Statements), newArgs(4, NewK(SymbolLabel(symbol_Var)

			// symbol_name := "symbol_" + t.Appl.Name
			// args := ATermListToStringList(t.Appl.Args)
			// var sargs string
			// if len(args) == 0 {
			// 	sargs = "NULL"
			// } else {
			// 	sargs = fmt.Sprintf("newArgs(%d, %s)", len(args), strings.Join(args, ","))
			// }
			// return fmt.Sprintf("NewK(SymbolLabel(%s), %s)", symbol_name, sargs)

		}
		default: {
			panic("Missing case!");
		}
	}
}


// func ATermListToStringList(l []aterm.ATerm) []string {
// 	arguments := make([]string, len(l))
// 	for i, t := range l {
// 		arguments[i] = ATermToC(&t)
// 	}
// 	// res := strings.Join(arguments, ",")
// 	// fmt.Printf("%s\n", res)
// 	// if strings.HasSuffix(res, " ") {
// 	// 	panic(fmt.Sprintf("Something up with %v", l))
// 	// }
// 	return arguments
// }
// func ATermToC(t *aterm.ATerm) string {
// 	switch t.Type {
// 		case aterm.Error:
// 			panic("Didn't expect error term in ATermToC()")
// 		case aterm.String:
// 			return fmt.Sprintf("new_builtin_string(\"%s\")", t.Str)
// 		case aterm.Int:
// 			// return NewK(Int64Label(t.Int), nil)
// 			return fmt.Sprintf("new_builtin_int(%d)", t.Int)
// 		case aterm.Appl:
// 			if (t.Appl.Name == "#Int" || t.Appl.Name == "#String") {
// 				return ATermToC(&t.Appl.Args[0])
// 			}
// 			if (t.Appl.Name == "#Hole") {
// 				// return ATermToC(&t.Appl.Args[0])
// 				nodolla := strings.TrimLeft(t.Appl.Args[0].Str, "$")
// 				return "hole_" + nodolla
// 				// strings.TrimLeft(t.Appl.Args[0], cutset)
// 			}
// 			// return NewK(StringLabel(t.Appl.Name), ATermListToListK(t.Appl.Args))
// 			// symbol, ok := symbolMap[t.Appl.Name]
// 			// if !ok {
// 			// 	panic(fmt.Sprintf("Couldn't find symbol %s", t.Appl.Name))
// 			// }
// 			symbol_name := "symbol_" + t.Appl.Name
// 			args := ATermListToStringList(t.Appl.Args)
// 			var sargs string
// 			if len(args) == 0 {
// 				sargs = "NULL"
// 			} else {
// 				sargs = fmt.Sprintf("newArgs(%d, %s)", len(args), strings.Join(args, ","))
// 			}
// 			return fmt.Sprintf("NewK(SymbolLabel(%s), %s)", symbol_name, sargs)
// 		// case aterm.List: 
// 		// 	return NewK(StringLabel("KList"), ATermListToListK(t.List))
// 	}
// 	panic(fmt.Sprintf("Not handling default for %v", t))
// }
