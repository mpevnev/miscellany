#include <math.h>
#include <stdlib.h>

#include "array.h"
#include "list.h"
#include "map.h"

#define CRIT_LOAD_FACTOR 0.7
#define EXPAND_FACTOR 1.3
#define EXPAND_MIN 10

/* ---------- helper function declarations ---------- */

static size_t 
next_prime(size_t i);

static int
is_prime(size_t i);

static void 
destroy_list_from_array(void *ptr);

static void
destroy_pair_list(void *list);

static int
init_buckets(struct map *, size_t num_buckets);

static size_t
fnv_hash(void *data, size_t size);

static size_t
get_size(struct map *, void *data);

static int
can_find(struct list *list, void *data, key_eq_fn eq);

static int 
can_find_ex(struct list *list, void *data, key_eq_ex_fn eq, void *arg);

static struct map_pair *
create_pair(void *key, void *value);

/* ---------- creation ---------- */

struct map *
map_create(size_t num_buckets, key_size_fn key_size, int allow_autoexpand)
{
	struct map *res = malloc(sizeof(struct map));
	if (res == NULL) return NULL;

	if (!init_buckets(res, num_buckets)) {
		free(res);
		return NULL;
	}
	res->key_size = key_size;
	res->allow_autoexpand = allow_autoexpand;
	return res;
}

struct map *
map_create_fs(size_t num_buckets, size_t key_size, int allow_autoexpand)
{
	struct map *res = malloc(sizeof(struct map));
	if (res == NULL) return NULL;

	if (!init_buckets(res, num_buckets)) {
		free(res);
		return NULL;
	}
	res->key_size = NULL;
	res->fixed_key_size = key_size;
	res->allow_autoexpand = allow_autoexpand;
	return res;
}

/* ---------- destruction ---------- */

void
map_destroy(struct map *map)
{
	arr_destroy_ex(map->buckets, &destroy_pair_list);
	free(map);
}

void
map_destroy_ex(struct map *map, void (*pair_destroyer)(void *pair))
{
	size_t size = arr_size(map->buckets);
	for (size_t i = 0; i < size; i++) {
		struct list **chain = arr_ix(map->buckets, i);
		struct list_elem *cur = list_first(*chain);
		while (cur != NULL) {
			pair_destroyer(list_data(cur));
			cur = cur->next;
		}
		list_destroy_ex(*chain, &free);
	}
	arr_destroy(map->buckets);
	free(map);
}

void
map_destroy_exx(struct map *map, void (*pair_destroyer)(void *pair, void *arg), void *arg)
{
	size_t size = arr_size(map->buckets);
	for (size_t i = 0; i < size; i++) {
		struct list **chain = arr_ix(map->buckets, i);
		struct list_elem *cur = list_first(*chain);
		while (cur != NULL) {
			pair_destroyer(list_data(cur), arg);
			cur = cur->next;
		}
		list_destroy_ex(*chain, &free);
	}
	arr_destroy(map->buckets);
	free(map);
}

/* ---------- manipulation ---------- */

enum map_err
map_insert(struct map *map, void *key, void *value, key_eq_fn eq)
{
	size_t ix = fnv_hash(key, get_size(map, key));
	ix = ix % arr_size(map->buckets);

	struct list **chain = arr_ix(map->buckets, ix);
	if (eq != NULL && can_find(*chain, key, eq)) 
		return MAPE_EXIST;

	if (map->allow_autoexpand && map_load_factor(map) >= CRIT_LOAD_FACTOR) {
		int ok = map_expand(map, EXPAND_FACTOR, EXPAND_MIN);
		if (!ok) return MAPE_NOMEM;
	}

	struct map_pair *pair = create_pair(key, value);
	if (pair == NULL)
		return MAPE_NOMEM;
	if (!list_push(*chain, pair)) {
		free(pair);
		return MAPE_NOMEM;
	}
	return MAPE_OK;
}

enum map_err
map_insert_ex(struct map *map, void *key, void *value, key_eq_ex_fn eq, void *arg)
{
	size_t ix = fnv_hash(key, get_size(map, key));
	ix = ix % arr_size(map->buckets);

	struct list **chain = arr_ix(map->buckets, ix);
	if (eq != NULL && can_find_ex(*chain, key, eq, arg)) 
		return MAPE_EXIST;

	if (map->allow_autoexpand && map_load_factor(map) >= CRIT_LOAD_FACTOR) {
		int ok = map_expand(map, EXPAND_FACTOR, EXPAND_MIN);
		if (!ok) return MAPE_NOMEM;
	}

	struct map_pair *pair = create_pair(key, value);
	if (pair == NULL)
		return MAPE_NOMEM;
	if (!list_push(*chain, pair)) {
		free(pair);
		return MAPE_NOMEM;
	}
	return MAPE_OK;
}

int
map_expand(struct map *map, double factor, size_t min)
{
	size_t old_size = arr_size(map->buckets);
	size_t new_size = old_size * factor;
	if (new_size < old_size + min) new_size = old_size + min;
	new_size = next_prime(new_size);

	struct array *old_buckets = map->buckets;
	int ok = init_buckets(map, new_size);
	if (!ok) return 0;

	for (size_t i = 0; i < old_size; i++) {
		struct list **chain = arr_ix(old_buckets, i);
		struct list_elem *cur = (*chain)->first;
		while (cur != NULL) {
			struct map_pair *pair = list_data(cur);
			if (map_insert(map, pair->key, pair->value, NULL) != MAPE_OK) {
				arr_destroy_ex(map->buckets, &destroy_pair_list);
				map->buckets = old_buckets;
				return 0;
			}
			cur = list_next(cur);
		} /* foreach pair in chain */
	} /* foreach bucket */

	arr_destroy_ex(old_buckets, &destroy_pair_list);
	return 1;
}

int
map_remove(struct map *map, void *key, key_eq_fn eq, int remove_all, void **value)
{
	size_t ix = fnv_hash(key, get_size(map, key)) % arr_size(map->buckets);

	struct list **chain = arr_ix(map->buckets, ix);
	void *res = NULL;
	int found = 0;
	struct list_elem *cur = list_first(*chain);
	while (cur != NULL) {
		struct list_elem *next = list_next(cur);
		struct map_pair *pair = list_data(cur);
		if (eq(pair->key, key)) {
			if (!found) res = pair->value;
			found = 1;
			list_remove(*chain, cur);
			free(cur);
			free(pair);
			if (!remove_all) break;
		}
		cur = next;
	}

	if (found && value != NULL) *value = res;
	return found;
}

int
map_remove_ex(struct map *map, void *key, key_eq_ex_fn eq, int remove_all, void *arg,
		void **value)
{
	size_t ix = fnv_hash(key, get_size(map, key)) % arr_size(map->buckets);

	struct list **chain = arr_ix(map->buckets, ix);
	void *res = NULL;
	int found = 0;
	struct list_elem *cur = list_first(*chain);
	while (cur != NULL) {
		struct list_elem *next = list_next(cur);
		struct map_pair *pair = list_data(cur);
		if (eq(pair->key, key, arg)) {
			if (!found) res = pair->value;
			found = 1;
			list_remove(*chain, cur);
			free(cur);
			free(pair);
			if (!remove_all) break;
		}
		cur = next;
	}

	if (found && value != NULL) *value = res;
	return found;
}

/* ---------- information retrieval ---------- */

int
map_lookup(struct map *map, void *key, key_eq_fn eq, void **value)
{
	size_t ix = fnv_hash(key, get_size(map, key)) % arr_size(map->buckets);

	struct list **chain = arr_ix(map->buckets, ix);
	struct list_elem *cur = list_first(*chain);
	while (cur != NULL) {
		struct map_pair *pair = list_data(cur);
		if (eq(pair->key, key)) {
			*value = pair->value;
			return 1;
		}
		cur = list_next(cur);
	}
	return 0;
}

int
map_lookup_ex(struct map *map, void *key, key_eq_ex_fn eq, void *eq_arg, void **value)
{
	size_t ix = fnv_hash(key, get_size(map, key)) % arr_size(map->buckets);

	struct list **chain = arr_ix(map->buckets, ix);
	struct list_elem *cur = list_first(*chain);
	while (cur != NULL) {
		struct map_pair *pair = list_data(cur);
		if (eq(pair->key, key, eq_arg)) {
			*value = pair->value;
			return 1;
		}
		cur = list_next(cur);
	}
	return 0;
}

extern size_t
map_num_buckets(struct map *map);

double
map_load_factor(struct map *map)
{
	size_t len = arr_size(map->buckets);
	size_t num_used = 0;
	for (size_t i = 0; i < len; i++) {
		struct list **cur = arr_ix(map->buckets, i);
		if (!list_empty(*cur)) num_used++;
	}
	return num_used * 1.0 / len;
}

/* ---------- helper functions ---------- */

int
is_prime(size_t i)
{
	size_t root = sqrt(i);
	for (size_t k = 2; k <= root; k++)
		if (i % k == 0) return 0;
	return 1;
}

size_t
next_prime(size_t i)
{
	if (i % 2 == 0) i++;
	while (1) {
		i += 2;
		if (is_prime(i)) return i;
	}
}

void
destroy_list_from_array(void *ptr)
{
	struct list **list = ptr;
	list_destroy(*list);
}

void
destroy_pair_list(void *ptr)
{
	struct list **list = ptr;
	list_destroy_ex(*list, &free);
}

int
init_buckets(struct map *map, size_t num_buckets)
{
	num_buckets = next_prime(num_buckets);
	map->buckets = arr_create(num_buckets, sizeof(struct list *));
	if (map->buckets == NULL) return 0;

	for (size_t i = 0; i < num_buckets; i++) {
		struct list *chain = list_create();
		if (chain == NULL) {
			arr_destroy_ex(map->buckets, &destroy_list_from_array);
			return 0;
		}
		arr_append(map->buckets, &chain);
	}
	return 1;
}

size_t
fnv_hash(void *data, size_t size)
{
	unsigned char *p = data;
	unsigned int h = 2166136261;

	for (size_t i = 0; i < size; i++) 
		h = (h * 16777619) ^ p[i];
	return h;
}

size_t
get_size(struct map *map, void *data)
{
	if (map->key_size == NULL)
		return map->fixed_key_size;
	else
		return map->key_size(data);
}

int
can_find(struct list *list, void *data, key_eq_fn eq)
{
	return list_find_eq(list, data, eq) != NULL;
}

int
can_find_ex(struct list *list, void *data, key_eq_ex_fn eq, void *arg)
{
	return list_find_eq_ex(list, data, eq, arg) != NULL;
}

struct map_pair *
create_pair(void *key, void *value)
{
	struct map_pair *res = malloc(sizeof(struct map_pair));
	if (res == NULL) return NULL;
	res->key = key;
	res->value = value;
	return res;
}
