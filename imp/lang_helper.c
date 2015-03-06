#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>

#include "k_pub.h"
#include "lang.h"
#include "adopt.h"

adopt_spec opt_specs[] = {
	// { ADOPT_VALUE, "debug", 'd', NULL, "displays debug information" },
	{ ADOPT_VALUE, "input", 'i', NULL, "input for program" },
	{ ADOPT_SWITCH, "help", 0, NULL, NULL, ADOPT_USAGE_HIDDEN },
	// { ADOPT_SWITCH, "test", 't', NULL, "Turns testing on" },
	// { ADOPT_SWITCH, "bench", 'b', NULL, "Turns benching on" },
	// { ADOPT_SWITCH, "mem", 'm', NULL, "Turns mem test on" },
	// { ADOPT_VALUE, "verbose", 'v', "level", "sets the verbosity level (default 1)" },
	// { ADOPT_VALUE, "channel", 'c', "channel", "sets the channel", ADOPT_USAGE_VALUE_REQUIRED },
	{ ADOPT_LITERAL },
	{ ADOPT_ARG, "file", 'f', NULL, "file path" },
	{ 0 },
};


Configuration* reduce(K* pgm) {
	Configuration* config = new_configuration(pgm);

	repl(config);
	return config;
}



void run(const char* path, int64_t upto) {
	label_helper lh;

	lh.count = num_labels();
	lh.labels = label_names();

	FILE *file = stdin;
	if (path != NULL) {
		file = fopen(path, "r");

		if (file == NULL) {
			printf("Couldn't open %s\n", path);
			exit(1);
		}
	}
	K* prog = aterm_file_to_k(file, lh, new_builtin_int(upto));
	fclose(file);

	Configuration* config = reduce(prog);

	// K* resultK = get_result(config->k);
	// int64_t result = k_get_arg(resultK, 0)->label->i64_val;

	char* ss = get_state_string(config);

	// char* ss = stateString(config->k, config->state);
	printf("%s\n", ss);
	free(ss);



	// check(config->k, config->state);

	// computation_cleanup(config->k);
	// state_cleanup(config->state);

	// free(config->k);
	// free(config->state);
	// free(config);

	return;
}

int main(int argc, char* argv[]) {
	setvbuf(stdout, NULL, _IONBF, 0);
	adopt_parser parser;
	adopt_opt opt;
	// const char *value;
	int upto = 5;
	const char *path = NULL;
	// int test = 0;
	// int bench = 0;
	// int mem = 0;

	k_init();

	// for (int i = 0; i < 3000; i += 10) {
	//  printf("pow(%d) = %d; log(%d) = %d\n", i, next_highest_power(i), i, ceil_log2(i));
	// }
	// return;

	set_labels(num_labels(), label_names());

	adopt_parser_init(&parser, opt_specs, argv + 1, argc - 1);

	while (adopt_parser_next(&opt, &parser)) {
		if (opt.spec) {
			if (printDebug) {
				printf("'%s' = ", opt.spec->name);
				printf("'%s'\n", opt.value);
			}
			if (strcmp(opt.spec->name, "file") == 0) {
				path = opt.value;
				if (printDebug) {
					printf("Will load program file '%s'\n", path);
				}
			}
			if (strcmp(opt.spec->name, "input") == 0) {
				upto = atoi(opt.value);
			}
			// if (strcmp(opt.spec->name, "test") == 0) {
			//     test = 1;
			// }
			// if (strcmp(opt.spec->name, "bench") == 0) {
			//     bench = 1;
			// }
			// if (strcmp(opt.spec->name, "mem") == 0) {
			//     mem = 1;
			// }
		} else {
			fprintf(stderr, "Unknown option: %s\n", opt.value);
			adopt_usage_fprint(stderr, argv[0], opt_specs);
			return 129;
		}
	}
	
	// if (bench) {
	//     run_bench();
	//     dump_garbage_info();
	// } else if (mem) {
	//     run_mem();
	//     dump_garbage_info();
	// } else if (test) {
	//     run_tests();
	//     dump_garbage_info();
	// } else {
	run(path, upto);
	// printf("Result: %" PRId64 "\n", result);
	
	if (printDebug) {
		dump_garbage_info();
	}

		// printf("\nrewrites: %" PRIu64 "\n", rewrites);
	// }

	return 0;
}
