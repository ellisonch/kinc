#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main(void) {
	int m = 500000;
	int n = 0;
	int q = 0;
	int r = 0;
	int s = 0;

	while (!(m <= 2)) {
		n = m ;
		m = m + -1 ;
		while (!(n <= 1)) {
			s = s + 1 ; 
			q = n / 2 ; 
			r = q + q + 1 ;
			if (r <= n) { 
				n = n + n + n + 1;
			} else { 
				n = q;
			}
		} 
	}
	printf("%" PRId64 "\n", s);
	return 0;
}