#ifndef UTILS_H
#define UTILS_H

#define panic(...) (_panic(__func__, __FILE__, __LINE__, __VA_ARGS__))

_Noreturn void _panic(const char* func, const char* file, int line, const char* format, ...);

struct ll_node;
typedef struct ll_node ll_node;
struct ll_node {
	void* item;
	ll_node *next;
};

typedef struct {
	int count;
	ll_node* head;
	ll_node* tail;
} ll_list;

#endif
