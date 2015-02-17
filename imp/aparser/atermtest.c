#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "aterm.h"
#include "parser.tab.h"

extern FILE* yyin;
extern aterm final_term;

int main(void) {
	setvbuf(stdout, NULL, _IONBF, 0);
	yyin = stdin;
	do {
		yyparse();
	} while(!feof(yyin));
	printf("Getting ready to to_string\n");
	char* sterm = aterm_to_string(final_term);
	printf("Back from to_string\n");
	printf("%s\n", sterm);
}
