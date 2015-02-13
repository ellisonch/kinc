// TODO: need to handle max cell sizes

#include <stdint.h>
#include <stdlib.h>
#include <strings.h>

#include "k.h"
#include "settings.h"
#include "utils.h"

// TODO: terrible, not right, horrible
extern int next;

// FIXME: leaks memory, sucks
char* kCellToString(K *kCell[]) {
	char* s = malloc(10000);
	strcpy(s, "k(\n");
	for (int i = next - 1; i >= 0; i--) {
		strcat(s, "  ~> ");
		strcat(s, KToString(kCell[i]));
		strcat(s, "\n");
	}
	strcat(s, ")\n");
	return s;
}

// FIXME: leaks memory, sucks
char* stateString(K *kCell[], K *stateCell[]) {
	char* s = malloc(20000);
	strcpy(s, "state(\n"); 
	for (int i = 0; i < 26; i++) {
		if (stateCell[i] != NULL) {
			char var[] = "  x -> ";
 			var[2] = i + 'a';
			strcat(s, var);
			strcat(s, KToString(stateCell[i]));
			strcat(s, "\n");
		}
	}
	strcat(s, ")\n");
	strcat(s, kCellToString(kCell));
	return s;
}

void trimK(K *kCell[]) {
	int top = next - 1;
	Dec(kCell[top]);
	next--;
	// kCell = kCell[:top]
}

void setHead(K *kCell[], K* k) {
	int top = next - 1;
	Inc(k);
	Dec(kCell[top]);
	kCell[top] = k;
}

void setPreHead(K *kCell[], K* k) {
	int pre = next - 2;
	Inc(k);
	Dec(kCell[pre]);
	kCell[pre] = k;
}


void appendK(K *kCell[], K* k) {
	if (checkStackSize) {
		if (next >= MAX_K) {
			panic("Trying to add too many elements to the K Cell!");
		}
	}
	Inc(k);
	kCell[next] = k;
	next++;
}
