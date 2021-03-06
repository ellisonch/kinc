#include <assert.h>
#include <inttypes.h>
#include <time.h>

#include "imp_bench.h"
#include "k.h"

const char* p1 = "programs/prog1.aterm";
const char* p2 = "programs/prog2.aterm";
const char* p3 = "programs/prog3.aterm";
const char* p4 = "programs/prog4.aterm";

int run_tests() {
	uint64_t r;

	r = run(p1, 1000);
	assert(r == 500500);

	r = run(p2, 1000);
	assert(r == 59541);

	r = run(p3, 1000);
	assert(r == 168);

	r = run(p4, 1100087778366101931LL);
	assert(r = 3);

	// printf("%" PRId64 "\n", r);

	//uint64_t run(const char* path, int64_t upto)
	return 0;
}

int run_bench() {
	clock_t t_start = clock();

	run(p1, 500000);

	run(p2, 7000);

	run(p3, 20000);

	run(p4, 1100087778366101931LL);
	
	clock_t t_end = clock();
	double t_sec = (double)(t_end - t_start) / CLOCKS_PER_SEC;
	printf("Took %f\n", t_sec);

	// printf("%" PRId64 "\n", r);

	//uint64_t run(const char* path, int64_t upto)
	return 0;
}

int run_mem() {
	uint64_t r;
	for (int i = 0; i < 20000; i++) {
		r = run(p1, 1000);
		assert(r == 500500);
	}
	return 0;
}
