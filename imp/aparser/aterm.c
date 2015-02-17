#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "aterm.h"

at_list* at_list_append(at_list* oldp, aterm at) {	
	aterm* atp = malloc(sizeof(*atp));
	atp = memcpy(atp, &at, sizeof(aterm));

	at_list* newp = malloc(sizeof(*newp));
	newp->item = atp;
	newp->next = oldp;

	return newp;
}

// fixme not efficient
char* at_list_to_string(at_list* l) {
	if (l == NULL) {
		char* ret = malloc(1);
		ret[0] = '\0';
		return ret;
	}

	char* sterm = aterm_to_string(*l->item);
	char* sargs = at_list_to_string(l->next);
	int needs_comma = 0;
	if (strlen(sargs) == 0) {
		return sterm;
	}
	char* ret = malloc(strlen(sargs) + strlen(sterm) + 2); // for comma and null terminator
	sprintf(ret, "%s,%s", sargs, sterm);
	free(sterm);
	free(sargs);

	return ret;
}

char* aterm_to_string(aterm at) {
	switch (at.type) {
		case AT_ERROR: {
			return "AT_ERROR";
		}
		case AT_INT64: {
			// longtest int64 is -9223 37203 68547 75808
			char* ret = malloc(25);
			sprintf(ret, "%" PRId64, at.int64);
			return ret;
		}
		case AT_REAL: {
			return "AT_REAL";
		}
		case AT_STRING: {
			return "AT_STRING";
		}
		case AT_APPL: {
			at_appl appl = at.appl;
			char* sargs = at_list_to_string(appl.args);

			int name_len = strlen(appl.name);
			int args_len = strlen(sargs);
			int ret_len = name_len + args_len + 3; // for parens and null terminator
			char* ret = malloc(ret_len);

			sprintf(ret, "%s(%s)", appl.name, sargs);
			free(sargs);

			return ret;
		}
		case AT_LIST: {
			return "AT_LIST";
		}
		default: {
			return "Missing Case";
		}
	}
}


// func (at *ATerm) String() string {
// 	switch at.Type {
// 		case Error: return "---Error---"
// 		case Int: return fmt.Sprintf("%d", at.Int)
// 		case Real: return fmt.Sprintf("%f", at.Real)
// 		case Appl: return fmt.Sprintf("%s(%s)", at.Appl.Name, at.Appl.Args.String())
// 		case List: return fmt.Sprintf("[%s]", CommaList(at.List).String())
// 	}
// 	return "---Error---"
// }
// func (l CommaList) String() string {
// 	ss := []string{}
// 	for _, at := range l {
// 		ss = append(ss, at.String())
// 	}
// 	return strings.Join(ss, ",")
// }

