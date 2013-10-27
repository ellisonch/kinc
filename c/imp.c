#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

#define PANIC(...) (panic(__func__, __FILE__, __LINE__, __VA_ARGS__))

typedef enum {
	e_string,
	e_i64,
} KLabelType;

typedef struct K K;
typedef struct ListK ListK;

// typedef struct K *ListK[];
typedef struct ListK {
	int cap;
	int len;
	K *a[];
} ListK;

typedef struct {
	KLabelType type;
	union {
		const int64_t i64_val;
		const char* string_val;
	} data;
} KLabel;

typedef struct K {
	KLabel kl;
	ListK* args;
	int refs;
} K;


// KLabel Int64Label(int64 i64) {
// 	return (KLabel){type = e_i64, {i64_val = i64}};
// 	// return KLabel{kind: e_i64, data: i64}
// }
KLabel StringLabel(const char* s) {
	return (KLabel){.type = e_string, {.string_val = s}};
}

void panic(const char* func, const char* file, int line, const char* format, ...) {
    va_list va;
    va_start(va, format);
    fprintf(stderr, "PANIC! %s() (%s:%d): ", func, file, line);
    vfprintf(stderr, format, va);
    fprintf(stderr, "\n");
    exit(1);
}

// func NewK(label KLabel, args ListK) *K {
// 	for _, arg := range args {
// 		if arg == nil {
// 			panic("Didn't expect nil arg in NewK()")
// 		}
// 		arg.Inc()
// 	}
// 	var newK *K
// 	if len(deadK) > 0 {
// 		newK = deadK[len(deadK)-1]
// 		newK.label = label
// 		newK.args = args
// 		deadK = deadK[:len(deadK)-1]
// 	} else {
// 		newK = &K{label, args, 0}
// 	}
// 	return newK
// }
K* NewK(KLabel label, ListK* args) {
	if (args != NULL) {
		for (int i = 0; i < args->len; i++) {
	 		K* arg = args->a[i];
	 		if (arg == NULL) {
	 			PANIC("Didn't expect nil arg in NewK()");
	 		}
	 	}
	 }
	return NULL;
}


K* prog1() {
	K* n = NewK(StringLabel("n"), NULL);
	K* s = NewK(StringLabel("s"), NULL);
	// hundred := NewKSpecial(label_upto, nil, true, false)

	// l1 := NewK(label_var, []*K{n, s})
	
	// l2 := NewK(label_assign, []*K{n, hundred})
	
	// l3 := NewK(label_assign, []*K{s, k_zero()})
	// sPn := NewK(label_plus, []*K{s, n})
	// l5 := NewK(label_assign, []*K{s, sPn})
	// negOne := NewK(label_neg, []*K{k_one()})
	// nPno := NewK(label_plus, []*K{n, negOne})
	// l6 := NewK(label_assign, []*K{n, nPno})
	// body := NewK(label_semi, []*K{l5, l6})

	// nLTzero := NewK(label_lte, []*K{n, k_zero()})
	// guard := NewK(label_not, []*K{nLTzero})
	// l4 := NewK(label_while, []*K{guard, body})

	// pgm := NewK(label_semi, []*K{l3, l4})
	// pgm = NewK(label_semi, []*K{l2, pgm})
	// pgm = NewK(label_semi, []*K{l1, pgm})
	// return pgm
	return NULL;
}


int main(void) {
	printf("foo\n");
	K* prog = prog1();
	return 0;
}