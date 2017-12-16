#ifndef TEST_H
#define TEST_H

#include "btree.h"

int 
cmp_ints(void *, void *);

int 
cmp_ints_ex(void *, void *, void *);

int *
mkint(int);

int
int_eq(void *, int);

int
inttree_eq(struct btree *, void *);

#endif /* TEST_H */
