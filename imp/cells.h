#ifndef CELLS_H
#define CELLS_H

#include "k.h"

void trimK(K *kCell[]);
void setHead(K *kCell[], K* k);
void setPreHead(K *kCell[], K* k);
void appendK(K *kCell[], K* k);
char* stateString(K *kCell[], K *stateCell[]);

#endif