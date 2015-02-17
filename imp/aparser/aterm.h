#ifndef ATERM_H
#define ATERM_H

#include <stdint.h>

typedef enum {
	AT_ERROR,
	AT_INT64,
	AT_REAL,
	AT_STRING,
	AT_APPL,
	AT_LIST,
} at_type;

struct aterm;
typedef struct aterm aterm;

struct at_list;
typedef struct at_list at_list;
struct at_list {
	aterm *item;
	at_list *next;
};

struct list *make_list(void *item, struct list *next);

typedef struct {
	char* name;
	at_list* args;
} at_appl;

struct aterm {
	at_type type;
	union {
		int64_t int64;
		double real;
		char* string;
		at_appl appl;
		at_list* list;
	};
};

at_list* at_list_append(at_list* oldp, aterm at);
char* aterm_to_string(aterm at);

#endif
