#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

// 50000000
int main(int argc, char* argv[]) {
	uint64_t n = atoll(argv[1]);
	uint64_t s = 0;

	while (!(n <= 0)) {
		s = s + n ;
		n = n + -1 ;
	}
	printf("%" PRId64 "\n", s);
	return 0;
}
