
# Array module `<misc/array.h>`

This module provides growable arrays. At the moment the functionality is quite
limited - just basic array operations, appending/prepending elements and other
arrays and views are implemented.

## Data types

The data type for the arrays is `struct array`. Functions to access its members
are provided, so please treat it as opaque.

The header also provides an enum for use with view operations: `enum aview_dir`,
which members are:
- `AVIEW_LEFT`,
- `AVIEW_RIGHT`.

## Functions - creation

These functions create a new array in dynamically allocated memory.

### `arr_create`

```
struct array *
arr_create(size_t capacity, size_t stride)
```

Create and return a new array with the buffer big enough to hold `capacity`
elements of size `stride`.

Return NULL if an OOM condition has occured.

### `arr_from_data`

```
struct array *
arr_from_data(size_t size, size_t stride, void *data)
```

Create and return a new array with `size` elements of size `stride`. The data
pointed to by `data` will be copied, not reused.

Return NULL if an OOM condition has occured.

### `arr_from_array`

```
struct array *
arr_from_array(struct array *array)
```

Create and return a new array of size and capacity equal to the size of
`array`, and then copy the data of `array` into the newly created array.

Return NULL if an OOM condition has occured.

## Functions - initialization

These functions initialize an array that has already been allocated. Memory
saving isn't much, because the data will still live on heap.

### `arr_init`

```
int
arr_init(struct array *array, size_t capacity, size_t stride)
```

Allocate enough memory for `array` to hold `capacity` elements of size
`stride`.

Return 1 on success, 0 if an OOM condition occured.

### `arr_init_from_data`

```
int
arr_init_from_data(struct array *array, size_t size, size_t stride, void *data)
```

Allocate enough memory for `array` to hold `size` elements of size `stride`,
then copy that many elements from `data` into it.

Return 1 on success, 0 if an OOM condition occured.

### `arr_init_from_array`

```
int
arr_init_from_array(struct array *init, struct array *from)
```

Copy the data of `from` to `init`.

Return 1 on success, 0 if an OOM condition occured.

## Functions - destruction

These functions deallocate both the array and the data it holds.

### `arr_destroy`

```
void
arr_destroy(struct array *array)
```

Free the data `array` holds and then the array itself.

### `arr_destroy_ex`

```
void
arr_destroy_ex(struct array *array, void (*data_destroyer)(void *data))
```

Run `data_destroyer` on every element of `array`, then free the data `array`
holds and the array itself.

### `arr_destroy_exx`

```
void
arr_destroy_exx(struct array *array, void (*data_destroyer)(void *data, void *arg), void *arg)
```

Run `data_destroyer` on every element of `array` as the first argument and with
`arg` as the second, then free the data `array` holds and the array itself.

## Functions - finalization

These functions deallocate just the data an array holds, leaving the array
intact.

### `arr_fin`

```
void
arr_fin(struct array *array)
```

Free the data `array` holds.

### `arr_fin_ex`

```
void
arr_fin_ex(struct array *array, void (*data_destroyer)(void *data))
```

Run `data_destroyer` on every element of `array`, then free the data `array`
holds. The `array` itself is not freed.

### `arr_fin_exx`

```
void
arr_fin_exx(struct array *array, void (*data_destroyer)(void *data, void *arg), void *arg)
```

Run `data_destroyer` on every element of `array` as the first argument and with
`arg` as the second, then free the data `array` holds. The `array` itself is 
not freed.

## Functions - information retrieval

### `arr_size`

```
size_t
arr_size(struct array *array)
```

Return the number of elements `array` holds.

### `arr_capacity`

```
size_t
arr_capacity(struct array *array)
```

Return the number of elements `array` can hold.

### `arr_stride`

```
size_t
arr_stride(struct array *array)
```

Return the size of an element in `array`.

### `arr_ix`

```
void *
arr_ix(struct array *array, size_t index)
```

Return a pointer to the data at `index`. 

## Functions - manipulation

### `arr_set`

```
void
arr_set(struct array *array, size_t index, void *data)
```

Copy data pointed to by `data` into position specified by `index`.

### `arr_append`

```
int
arr_append(struct array *array, void *data)
```

Append data pointed to by `data` to `array`. 

Return 1 on success, 0 if an OOM condition has occured.

If used on a view, view's data will be copied and the view will become an 
independent array.

### `arr_prepend`

```
int
arr_prepend(struct array *array, void *data)
```

Prepend data pointed to by `data` to `array`.

Return 1 on success, 0 if an OOM condition has occured.

If used on a view, view's data will be copied and the view will become an 
independent array.

### `arr_append_a`

```
int
arr_append_a(struct array *append_to, struct array *append)
```

Append data contained in `append` to `append_to`.

Return 1 on success, 0 if an OOM condition has occured.

If used on a view, view's data will be copied and the view will become an 
independent array.

### `arr_prepend_a`

```
int
arr_prepend_a(struct array *prepend_to, struct array *prepend)
```

Prepend data contained in `prepend` to `prepend_to`.

Return 1 on success, 0 if an OOM condition has occured.

If used on a view, view's data will be copied and the view will become an 
independent array.

### `arr_preallocate`

```
int
arr_preallocate(struct array *array, size_t new_capacity)
```

Allocate enough space for `array` to hold `new_capacity` elements. Do nothing
if `new_capacity` is less than `array`'s size.

If used on a view, view's data will be copied and the view will become an 
independent array.

Return 1 on success, 0 if an OOM condition has occured.

**Note:** a successful preallocation invalidates views based on `array`.

### `arr_shrink_to_fit`

```
int
arr_shrink_to_fit(struct array *array)
```

Reallocate `array`'s memory so its capacity is exactly its size.

Return 1 on success, 0 if an OOM condition has occured.

**Note:** this invalidates views based on `array`.

## Functions - view manipulation

Views provide a way to select a portion of an array without allocating extra 
memory. A view based on an array is invalidated if the said array is freed, or
if `arr_preallocate` or `arr_shrink_to_fit` has been called on it.

You can use `arr_set` with views, it'll change the value in the original array.

You also can use any appending or prepending function with views, the view in
question will become an independent array and will hold a copy of original data.

### `aview_create`

```
struct array *
aview_create(struct array *source, size_t start, size_t end)
```

Create and return a view of `source` contents between `start` and `end`.

Return NULL if an OOM condition has occured.

### `aview_init`

```
void
aview_init(struct array *view, struct array *source, size_t start, size_t end)
```

Initialize `view` to use `source`'s data between indices `start` and `end`. No
memory allocations are made, so the function always succeeds.

### `aview_shift`

```
void
aview_shift(struct array *view, enum aview_dir which_end, enum aview_dir where, size_t by)
```

Shift the end of `view` specified by `which_end` in direction specified by 
`where` by `by` elements.

There are a few caveats:
- No bounds checking is performed, so the view may be expanded beyond its data
buffer.
- The left end of the view will never be shifted beyond the right end, and
vice-versa.
- Shifting can only affect one end at a time.
- You can use it with arrays, not just with views, but please don't do this.
