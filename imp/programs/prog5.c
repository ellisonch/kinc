#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int64_t inp = 50;
int main(void) {
	int64_t x = 1;
	int64_t n = 1;
	int64_t y = 1;
	int64_t z = 1;
	int64_t t;
	while ( n <= inp ) {
		t = y ;
		x = y ;
		y = z ;
		z = t + y ;
		n = n + 1 ;
	}
	printf("%" PRId64 "\n", x);
	return 0;
}
