#ifndef MAP_H
#define MAP_H

/** Map module.
 *
 * Provides mapping structure.
 *
 * While it is not required for the keys to be of the same type, it's easier to
 * make a size function for your map's keys if they are the same. Otherwise you
 * have to resort to some nontrivial data juggling to figure out the size of 
 * your object. And size is vital, as it's used in the hashing algorithm. 
 *
 */

#include <stdlib.h>

#include "array.h"

typedef size_t (*key_size_fn)(void *);
/* These two should return a non-zero value if the objects compare equal, 0
 * otherwise. */
typedef int (*key_eq_fn)(void *, void *);
typedef int (*key_eq_ex_fn)(void *data1, void *data2, void *external_arg);

struct map
{
	struct array *buckets;

	/* If this is NULL, then 'fixed_key_size' will be used instead. */
	key_size_fn key_size;
	size_t fixed_key_size;
};

struct map_pair
{
	void *key, *value;
};

enum map_err
{
	MAPE_OK,
	MAPE_NOMEM,
	MAPE_EXIST,
};

/* ---------- creation ---------- */

/* The number of buckets will be rounded up to the nearest prime. */
struct map *
map_create(size_t num_buckets, key_size_fn key_size);

/* Create a map with fixed size of keys. */
struct map *
map_create_fs(size_t num_buckets, size_t key_size);

/* ---------- destruction ---------- */

void
map_destroy(struct map *);

/* 'pair_destroyer' will be called on every pair in the mapping. 
 * The destroyer should *not* deallocate pairs themselves, the function takes
 * care of that. */
void 
map_destroy_ex(struct map *, void (*pair_destroyer)(void *pair));

/* Same, but 'pair_destroyer' takes an extra argument. */
void
map_destroy_exx(struct map *, void (*pair_destroyer)(void *pair, void *arg), void *arg);

/* ---------- manipulation ---------- */

/* Return MAPE_OK on success,
 * MAPE_NOMEM if there's not enough memory to create a new association,
 * MAPE_EXIST if the key already exists in the mapping.
 * Pass NULL as eq to avoid checking for existing keys.
 */
enum map_err
map_insert(struct map *, void *key, void *value, key_eq_fn eq);

/* Same, but the comparison function takes an extra argument. */
enum map_err
map_insert_ex(struct map *, void *key, void *value, key_eq_ex_fn eq, void *arg);

/* Increase the number of buckets in the map by 'factor' times, but no less 
 * than 'min'.
 * Return 1 on success, 0 if there's not enough memory to do so. */
int
map_expand(struct map *map, double factor, size_t min);

/* ---------- information retrieval ---------- */

/* Return 1 and fill 'value' with the associated value if a key is found in a
 * mapping.
 * Return 0 otherwise.
 */
int
map_lookup(struct map *, void *key, key_eq_fn eq, void **value);

/* Same, but equality function takes an extra argument. */
int
map_lookup_ex(struct map *, void *key, key_eq_ex_fn eq, void *eq_arg, void **value);

double
map_load_factor(struct map *);

#endif /* MAP_H */
