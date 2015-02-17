#include <assert.h>
#include <inttypes.h>

#include "imp.h"
#include "k.h"

const char* p1 = "programs/prog1.aterm";
const char* p2 = "programs/prog2.aterm";
const char* p3 = "programs/prog3.aterm";

int run_tests() {
	uint64_t r;

	r = run(p1, 1000);
	assert(r == 500500);

	r = run(p2, 1000);
	assert(r == 59541);

	r = run(p3, 1000);	
	assert(r == 168);
	
	// printf("%" PRId64 "\n", r);

	//uint64_t run(const char* path, int64_t upto)
	return 0;
}
