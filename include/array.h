#ifndef ARRAY_H
#define ARRAY_H

/** Array module.
 *
 * Provides dynamically growable (and shrinkable) arrays.
 *
 * Note that if you try to append an array or an element of a different size
 * to another array, it may sort of work, but you will have absolutely no way
 * of indexing such a construct (without major black magic). Please don't do
 * this, it'll just corrupt your normal data.
 *
 * */

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
 * If 'copy_data' is true, make a copy of 'data' instead of reusing it. 
 * Return NULL on an OOM condition. */
extern struct array *
arr_from_data(size_t size, size_t stride, void *data, int copy_data);

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
arr_init_from_data(struct array *array, size_t size, size_t stride, void *data,
		int copy_data);

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

extern size_t
arr_size(struct array *);

extern size_t 
arr_capacity(struct array *);

extern size_t 
arr_stride(struct array *);

/* Index into the array. */
extern void *
arr_ix(struct array *, size_t index);

/* ---------- manipulation ---------- */

/* All of these return 1 on success and 0 on failure. 
 * If used on a view, data will be copied and the view will become an 
 * independent array. */

extern int
arr_append(struct array *, void *data);

extern int
arr_prepend(struct array *, void *data);

extern int
arr_append_a(struct array *append_to, struct array *append);

extern int
arr_prepend_a(struct array *prepend_to, struct array *prepend);

/* Note that preallocating and shrinking arrays invalidates views based on them. */

/* Preallocate some space for array elements. Do nothing if the requested 
 * capacity is less that the size of the array. */
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
