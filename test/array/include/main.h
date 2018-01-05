#ifndef MAIN_H
#define MAIN_H

#include "array.h"

void
free_int(void *);

int *
mk_int(int i);

int
int_arr_eq(struct array *array, int *ints);

#endif /* MAIN_H */
