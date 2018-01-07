
# Map module `<misc/map.h>`

This module provides maps - that is, hashtables. Keys and values used in maps
can be arbitrary objects. The only thing to note is that maps do not provide
storage either for keys or values, using the pointers that were given to them.
Therefore it's up to the user to ensure that the data they point to is valid
while a map is in use.

## Data types

The data type for maps is `struct map`. The data type for key-value pairs is
`struct map_pair`.

Insertion routines return a value of type `map_err`, which can take one of the 
following values:
- `MAPE_OK`,
- `MAPE_NOMEM`,
- `MAPE_EXIST`.

Finally, there's a couple typedefs for functions used in the module:
- `typedef size_t (*key_size_fn)(void *data)` - such functions should return
the size of the object pointed to by `data`.
- `typedef int (*key_eq_fn)(void *a, void *b)` and
- `typedef int (*key_eq_ex_fn)(void *a, void *b, void *external_arg)` should
return a non-zero value if `a == b` and 0 otherwise.

## Functions - creation

In these functions, the number of requested buckets will be rounded up to the
next prime.

### `map_create`

```
struct map *
map_create(size_t num_buckets, key_size_fn key_size, int allow_autoexpand)
```

Create and return a new map with at least `num_buckets` buckets that'll use
`key_size` to calculate the size of its keys.

Return NULL if an OOM condition has occured.

If `allow_autoexpand` is set to true, the resulting map will attempt to 
automatically expand itself on insertion if load factor becomes too high.

### `map_create_fs`

```
struct map *
map_create_fs(size_t num_buckets, size_t key_size, int allow_autoexpand)
```

Create and return a new map with an least `num_buckets` buckets that'll assume
that all its keys have size `key_size`.

Return NULL if an OOM condition has occured.

If `allow_autoexpand` is set to true, the resulting map will attempt to 
automatically expand itself on insertion if load factor becomes too high.

## Functions - destruction

### `map_destroy`

```
void
map_destroy(struct map *map)
```

Free the memory used by `map`, but don't do anything with either keys or values.

### `map_destroy_ex`

```
void
map_destroy_ex(struct map *map, void (*pair_destroyer)(void *pair))
```

Run `pair_destroyer` on every key-value pair in `map`, which should deallocate
either keys, values or both (but *not the pairs themselves*, the function takes
care of that), then free the memory used by the map.

### `map_destroy_exx`

```
void
map_destroy_exx(struct map *map, void (*pair_destroyer)(void *pair, void *arg), void *arg)
```

Run `pair_destroyer` on every key-value pair in `map` as the first argument and
with `arg` as the second, which should deallocate either keys, values or both
(but *not the pairs themselves*, the function takes care of that), then free 
the memory used by the map.

## Functions - manipulation

### `map_insert`

```
enum map_err
map_insert(struct map *map, void *key, void *value, key_eq_fn eq)
```

Insert a key-value pair given by `key` and `value` into `map`. Use `eq` to
compare keys for equality (pass NULL to avoid checking if `key` already exists
in the map).

Return:
- `MAPE_OK` on success,
- `MAPE_EXIST` (only if `eq != NULL`) if `key` exists in the map,
- `MAPE_NOMEM` if an OOM condition has occured.

If the map was created with `allow_autoexpand` set, the map may be expanded if
needed.

### `map_insert_ex`

```
enum map_err
map_insert_ex(struct map *map, void *key, void *value, key_eq_ex_fn eq, void *arg)
```

Insert a key-value pair given by `key` and `value` into `map`. Use `eq` (with
the third argument being `arg`) to compare keys for equality (pass NULL to
avoid checking if `key` already exists in the map).

Return:
- `MAPE_OK` on success,
- `MAPE_EXIST` (only if `eq != NULL`) if `key` exists in the map,
- `MAPE_NOMEM` if an OOM condition has occured.

If the map was created with `allow_autoexpand` set, the map may be expanded if
needed.

### `map_expand`

```
int 
map_expand(struct map *map, double factor, size_t min)
```

Expand the number of buckets by `factor` times, but no less than by `min`.
Return 1 on success, 0 if there's not enough memory to do so.

## Functions - information retrieval

### `map_num_buckets`

```
size_t
map_num_buckets(struct map *map)
```

Return the number of buckets in `map`.

### `map_load_factor`

```
double
map_load_factor(struct map *map)
```

Return the load factor of `map`.

