#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main(void) {
	volatile uint64_t n = 50000000;
	volatile uint64_t s = 0;

	while (!(n <= 0)) {
		s = s + n ;
		n = n + -1 ;
	}
	printf("%" PRId64 "\n", s);
}