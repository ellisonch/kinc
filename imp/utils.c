#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

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
