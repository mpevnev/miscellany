#ifndef ARRAY_H
#define ARRAY_H

/** Array module.
 *
 * Provides dynamically growable (and shrinkable) arrays.
 *
 */

struct array
{
	size_t size, capacity;
	size_t stride;
	void *data;
	int is_view;
};

enum aview_dir
{
	AVIEW_LEFT,
	AVIEW_RIGHT,
};

/* ---------- creation and initialization ---------- */

/* Create a new array.
 * Return NULL on an OOM condition. */
extern struct array *
arr_create(size_t capacity, size_t stride);

/* Construct an array from a given data block.
 * Data will be copied.
 * Return NULL on an OOM condition. */
extern struct array *
arr_from_data(size_t size, size_t stride, void *data);

/* This will copy the data, so the resulting array is wholly independent from
 * the source array. 
 * Return NULL on an OOM condition. 
 * */
extern struct array *
arr_from_array(struct array *);

/* Return 1 on success, 0 on failure (typically, if an OOM condition occured). */
extern int
arr_init(struct array *array, size_t capacity, size_t stride);

extern int
arr_init_from_data(struct array *array, size_t size, size_t stride, void *data);

extern int
arr_init_from_array(struct array *init, struct array *from);

/* ---------- destruction and finalization ---------- */

/* Don't call this on an array that was statically allocated. It'll attempt to
 * free it, most likely segfaulting in process. */
extern void
arr_destroy(struct array *);

/* These two free the array and the data it holds. */

extern void
arr_destroy_ex(struct array *, void (*data_destroyer)(void *data));

extern void
arr_destroy_exx(struct array *, void (*data_destroyer)(void *data, void *arg), void *arg);

/* These - only the data. */

extern void
arr_fin_ex(struct array *, void (*data_destroyer)(void *data));

extern void
arr_fin_exx(struct array *, void (*data_destroyer)(void *data, void *arg), void *arg);

/* ---------- information retrieval ---------- */

inline size_t
arr_size(struct array *array)
{
	return array->size;
}

inline size_t 
arr_capacity(struct array *array)
{
	return array->capacity;
}

inline size_t 
arr_stride(struct array *array)
{
	return array->stride;
}

/* Index into an array. Return the pointer to the data. */
inline void *
arr_ix(struct array *array, size_t index)
{
	return array->data + array->stride * index;
}

/* ---------- manipulation ---------- */

/* Note that this uses buffer pointed to by 'data', not the pointer itself. */
extern void
arr_set(struct array *, size_t index, void *data);

/* All of these return 1 on success and 0 on failure. 
 * If used on a view, data will be copied and the view will become an 
 * independent array. */

/* Note that both append and prepend work on data pointed to by the 'data', not
 * with the data pointer itself. */

extern int
arr_append(struct array *, void *data);

extern int
arr_prepend(struct array *, void *data);

extern int
arr_append_a(struct array *append_to, struct array *append);

extern int
arr_prepend_a(struct array *prepend_to, struct array *prepend);

/* Note that preallocating and shrinking arrays invalidates views based on them. */

/* Preallocate some space for array elements. 
 * Do nothing and return true if the requested capacity is less that the size
 * of the array. */
extern int
arr_preallocate(struct array *, size_t new_capacity);

extern int
arr_shrink_to_fit(struct array *);

/* ---------- view manipulation ---------- */

extern struct array *
aview_create(struct array *, size_t start, size_t end);

extern void
aview_init(struct array *view, struct array *array, size_t start, size_t end);

/* This one performs no bounds-checking and may extend the data array beyound
 * that of the original array. 
 *
 * Left end will never be shifted beyond the right end, and vice-versa.
 *
 * Shifting the right end will never affect the left end, and vice-versa.
 *
 * The previous two rules mean that shifting left end to the right or right end
 * to the left may result in less shift than requested, if the other end would
 * be encountered on the way. In such case, 'data' pointer will be moved to the
 * first or the last element of the array, depending on the direction.
 *
 * And please, don't use it with actual arrays, only with views. */
extern void
aview_shift(struct array *view, enum aview_dir which_end, enum aview_dir where,
		size_t by);

#endif /* ARRAY_H */
