
# List module `<misc/list.h>`

This module provides doubly-linked lists. Most operations that are expected for
lists are implemented - insertion, removal, appending/prepending, sorting, 
converting to an array, copying.

## Data types

The data type for lists is `struct list`. The data type for a list element is
`struct list_elem`.

The data type for a list slice is `struct lslice`. A direction of certain shift
operations on slices has type `enum lslice_dir`, which has two values:
- `LSLICE_LEFT` and
- `LSLICE_RIGHT`.

There are also two typedefs for functions used in searching routines:
```
typedef int (*list_pred)(void *data);
typedef int (*list_pred)(void *data, void *external_arg);
```

## Traversing a list

Typically looks like this:
```
struct list *list = /* Produce a list somehow. */;
struct list_elem *cur = list_first(list);
while (cur != NULL) {
	void *data = list_data(cur);
	/* Do something. */
	cur = list_next(cur);
}
```

If you wish to traverse in the other direction, replace `list_first` with
`list_last` and `list_next` with `list_prev`.

## Functions - creation

### `list_create`

```
struct list *
list_create(void)
```

Create and return a new empty list. Return NULL on an OOM condition.

## Functions - destruction

### `list_destroy`

```
void
list_destroy(struct list *list)
```

Free `list`, but not the data it holds.

### `list_destroy_ex`

```
void
list_destroy_ex(struct list *list, void (*data_destroyer)(void *))
```

Run `data_destroyer` on every `list`'s element's data, then free the list.

### `list_destroy_exx`

```
void
list_destroy_exx(struct list *list, void (*data_destroyer)(void *data, void *arg), void *arg)
```

Run `data_destroyer` on every `list`'s elements's data as the first argument
and with `arg` as the second, then free the list.

## Functions - insertion

### `list_push`

```
int
list_push(struct list *list, void *data)
```

Prepend `data` to `list`. Return 1 on success, 0 on an OOM condition.

### `list_push_back`

```
int
list_push_back(struct list *list, void *data)
```

Append `data` to `list`. Return 1 on success, 0 on an OOM condition.

### `list_insert_after`

```
int
list_insert_after(struct list *list, struct list_elem *after, void *data)
```

Insert `data` into `list` after the element `after`. Return 1 on success, 0 if
an OOM condition has occured.

**Note:** weird things may happen if `after` is not in `list`.

### `list_insert_before`

```
int
list_insert_before(struct list *list, struct list_elem *before, void *data)
```

Insert `data` into `list` before the element `before`. Return 1 on success, 0
if an OOM condition has occured.

**Note:** weird things may happen if `before` is not in `list`.

### `list_append`

```
int
list_append(struct list *to, struct list *append)
```

Make a copy of `append` and append it to `to`. Return 1 on success, 0 if an OOM
condition has occured.

### `list_append_d`

```
void
list_append_d(struct list *to, struct list *append, int do_free)
```

Append `append` to `to` destructively: reuse the elements of `append` instead
of copying them. No memory allocations are performed.

If `do_free` is true, free the appended list.

### `list_prepend`

```
int
list_prepend(struct list *to, struct list *prepend)
```

Make a copy of `prepend` and prepend it to `to`. Return 1 on success, 0 if an 
OOM condition has occured.

### `list_prepend_d`

```
void
list_prepend_d(struct list *to, struct list *prepend, int do_free)
```

Prepend `prepend` to `to` destructively: reuse the elements of `prepend` 
instead of copying them. No memory allocations are performed.

If `do_free` is true, free the prepended list.

### `list_extract`

```
void
list_extract(struct list *into, struct list *from, struct list_elem *which_elem)
```

Take `which_elem` from `from` and prepend it to `into`. No memory allocations
or deallocations are performed.

### `list_extract_back`

```
void
list_extract_back(struct list *into, struct list *from, struct list_elem *which_elem)
```

Take `which_elem` from `from` and append it to `into`. No memory allocations or
deallocations are performed.

## Functions - removal

### `list_pop`

```
void *
list_pop(struct list *list)
```

Remove the head of the list and return its data.

### `list_pop_back`

```
void *
list_pop_back(struct list *list)
```

Remove the tail of the list and return its data.

### `list_clear`

```
void
list_clear(struct list *list)
```

Remove all elements from `list` without touching their data or the list itself.

### `list_clear_ex`

```
void
list_clear_ex(struct list *list, void (*destroyer)(void *data))
```

Run `destroyer` on every element's data, then free all elements. The list
itself remains intact.

### `list_clear_exx`

```
void
list_clear_exx(struct list *list, void (*destroyer)(void *data, void *arg), void *arg)
```

Run `destroyer` on every element's data as the first argument and with `arg` as
the second, then free all elements. The list itself remains intact.

### `list_remove`

```
void
list_remove(struct list *list, struct list_elem *elem)
```

Detach `elem` from `list`. `elem` is *not* freed. The rest of the list loses
access to `elem`, but `elem` still has links to its previous and next elements,
making it suitable for use in list traversing.

## Functions - information retrieval

Despite the common prefix `list`, not all of these operate on lists, some 
operate on list elements.

### `list_first`

```
struct list_elem *
list_first(struct list *list)
```

Return the first element of `list`.

### `list_last`

```
struct list_elem *
list_last(struct list *list)
```

Return the last element of `list`.

### `list_length`

```
size_t
list_length(struct list *list)
```

Return the length of `list`.

### `list_empty`

```
int
list_empty(struct list *list)
```

Return 1 if the list is empty, 0 otherwise.

### `list_nth`

```
struct list_elem *
list_nth(struct list *list, size_t n)
```

Return `n`th element of `list`, or NULL if the list is shorter than `n`. 
Zero-indexed.

### `list_nth_from_back`

```
struct list_elem *
list_nth_from_back(struct list *list, size_t n)
```

Return `n`th element of `list`, counting from the tail, or NULL if the list is
shorter than `n`. Zero-indexed.

### `list_data`

```
void *
list_data(struct list_elem *elem)
```

Return the data `elem` holds.

### `list_next`

```
struct list_elem *
list_next(struct list_elem *elem)
```

Return the next element in the list `elem` belongs to. May be NULL if `elem` is
the last.

### `list_prev`

```
struct list_elem *
list_prev(struct list_elem *elem)
```

Return the previous element in the list `elem` belongs to. May be NULL if `elem`
is the first.

### `list_find`

```
struct list_elem *
list_find(struct list *list, list_pred cond)
```

Return the first element in `list`, for which `cond(list_data(elem))` returns
true, or NULL if there's no such element.

### `list_find_ex`

```
struct list_elem *
list_find_ex(struct list *list, list_pred_ex cond, void *arg)
```

Return the first element in `list`, for which `cond(list_data(elem), arg)`
return true, or NULL if there's no such element.

### `list_find_eq`

```
struct list_elem *
list_find_eq(struct list *list, void *data, int (*eq)(void *, void *))
```

Return the first element in `list` which data compares equal to `data` using
`eq` as comparison function (which should return 0 for unequal objects and 
non-zero for equal), or NULL if there's no such element.

### `list_find_eq_ex`

```
struct list_elem *
list_find_eq_ex(struct list *list, void *data, int (*eq)(void *, void *, void *arg), void *arg)
```

Return the first element in `list` which data compares equal to `data` using
eq as comparison function (the third argument will be `arg`), or NULL if 
there's no such element.

## Functions - other

### `list_sort`

```
struct list *
list_sort(struct list *list, int (*cmp)(void *left, void *right), int desc)
```

Return a new sorted list using `cmp` as a comparison function.

Return NULL on an OOM condition.

`cmp` should return a negative value if `left < right`, 0 if `left == right`
and a positive value if `left > right`.

Sort in ascending order if `desc == 0`, in descending otherwise.

### `list_sort_ex`

```
struct list *
list_sort_ex(struct list *list, int (*cmp)(void *left, void *right, void *external),
	int desc, void *external_arg)
```

Return a new sorted list using `cmp` as a comparison function (the third 
argument will be `external_arg`).

Return NULL on an OOM condition.

`cmp` should return a negative value if `left < right`, 0 if `left == right`
and a positive value if `left > right`.

Sort in ascending order if `desc == 0`, in descending otherwise.

### `list_copy`

```
struct list *
list_copy(struct list *list)
```

Create and return a shallow copy of a list. 

Return NULL on an OOM condition.

### `list_to_array`

```
void **
list_to_array(struct list *list, size_t *size)
```

Create and return an array of pointers to `list`'s data. 
If `size` is not NULL, fill it with the size of the resulting array.
