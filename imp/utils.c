#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include "utils.h"

_Noreturn void _panic(const char* func, const char* file, int line, const char* format, ...) {
	va_list va;
	va_start(va, format);
	fprintf(stderr, "PANIC! %s() (%s:%d): ", func, file, line);
	vfprintf(stderr, format, va);
	fprintf(stderr, "\n");
	exit(1);
}

char* string_make_copy(const char* s) {
	assert(s != NULL);
	char* ret = malloc(strlen(s) + 1);
	strcpy(ret, s);
	return ret;
}

// http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
int next_highest_power(int v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;

	return v;
}

// http://stackoverflow.com/questions/3272424/compute-fast-log-base-2-ceiling
int ceil_log2(unsigned long long x) {
	static const unsigned long long t[6] = {
		0xFFFFFFFF00000000ull,
		0x00000000FFFF0000ull,
		0x000000000000FF00ull,
		0x00000000000000F0ull,
		0x000000000000000Cull,
		0x0000000000000002ull
	};

	int y = (((x & (x - 1)) == 0) ? 0 : 1);
	int j = 32;
	int i;

	for (i = 0; i < 6; i++) {
	int k = (((x & t[i]) == 0) ? 0 : j);
		y += k;
		x >>= k;
		j >>= 1;
	}

	return y;
}

// http://c-for-dummies.com/blog/?p=69
// "Feel free to use the delay() function in your code when a short pause is required."
void delay(int milliseconds) {
	long pause;
	clock_t now,then;

	pause = milliseconds*(CLOCKS_PER_SEC/1000);
	now = then = clock();
	while((now-then) < pause) {
		now = clock();
	}
}

// ll_list* ll_new() {
// 	ll_list* list = malloc(sizeof(*list));
// 	list->count = 0;
// 	list->head = NULL;
// 	list->tail = NULL;

// 	return list;
// }

// ll_node* ll_node_new(void* item, ll_node* next) {
// 	ll_node* node = malloc(sizeof(*node));
// 	node->item = item;
// 	node->next = next;

// 	return node;
// }

// void ll_add_back(ll_list* list, void* item) {
// 	assert(list != NULL);

// 	ll_node* node = ll_node_new(item, NULL);

// 	if (list->count == 0) {
// 		list->head = node;
// 		list->tail = node;
// 		return;
// 	}
// 	list->tail->next = node;
// 	list->count++;
// }

// void* ll_remove_front(ll_list* list) {
// 	assert(list != NULL);

// 	if (list->count == 0) {
// 		panic("Tried to remove from empty list");
// 	}
// 	ll_node* node = list->head;

// }
