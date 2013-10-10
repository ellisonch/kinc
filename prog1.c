#include <stdio.h>
int main(void) {
	volatile unsigned long long n = 5000000;
	volatile unsigned long long s = 0;

	while (!(n <= 0)) {
		s = s + n ;
		n = n + -1 ;
	}
	printf("%lld\n", s);
}