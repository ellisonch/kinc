#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "aterm.h"

int main(void) {
	// setvbuf(stdout, NULL, _IONBF, 0);
	aterm* at = at_parse(stdin);
	printf("Getting ready to to_string\n");
	char* sterm = aterm_to_string(*at);
	printf("Back from to_string\n");
	printf("%s\n", sterm);
}
