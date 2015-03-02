#ifndef LANG_H
#define LANG_H

#include <stdint.h>

#include "k_pub.h"

struct Configuration;
typedef struct Configuration Configuration;

void k_language_init();
Configuration* new_configuration(K* pgm);
void repl(Configuration* config);
int num_labels();
char** label_names();
char* get_state_string(Configuration* config);


#endif
