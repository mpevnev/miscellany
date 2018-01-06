
#include <stdlib.h>
#include <string.h>

#include "array.h"

/* ---------- creation and initialization ---------- */

struct array *
arr_create(size_t capacity, size_t stride)
{
	struct array *res = malloc(sizeof(struct array));
	if (res == NULL) return NULL;
	if (!arr_init(res, capacity, stride)) {
		free(res);
		return NULL;
	}
	return res;
}

struct array *
arr_from_data(size_t size, size_t stride, void *data)
{
	struct array *res = malloc(sizeof(struct array));
	if (res == NULL) return NULL;
	if (!arr_init_from_data(res, size, stride, data)) {
		free(res);
		return NULL;
	}
	return res;
}

struct array *
arr_from_array(struct array *array)
{
	return arr_from_data(array->size, array->stride, array->data);
}

int
arr_init(struct array *array, size_t capacity, size_t stride)
{
	void *data = malloc(capacity * stride);
	if (capacity != 0 && data == NULL)
		return 0;

	array->size = 0;
	array->capacity = capacity;
	array->stride = stride;
	array->data = data;
	array->is_view = 0;
	return 1;
}

int
arr_init_from_data(struct array *array, size_t size, size_t stride, void *data)
{
	void *copy = malloc(sizeof(size * stride));
	if (size != 0 && copy == NULL)
		return 0;
	memcpy(copy, data, size * stride);
	array->data = copy;
	array->size = array->capacity = size;
	array->stride = stride;
	array->is_view = 0;
	return 1;
}

int
arr_init_from_array(struct array *init, struct array *from)
{
	return arr_init_from_data(init, from->size, from->stride, from->data);
}

/* ---------- destruction and finalization ---------- */

void
arr_destroy(struct array *array)
{
	if (!array->is_view) 
		free(array->data);
	free(array);
}

void
arr_destroy_ex(struct array *array, void (*destroyer)(void *data))
{
	arr_fin_ex(array, destroyer);
	free(array);
}

void
arr_destroy_exx(struct array *array, void (*destroyer)(void *data, void *arg), void *arg)
{
	arr_fin_exx(array, destroyer, arg);
	free(array);
}

void
arr_fin_ex(struct array *array, void (*destroyer)(void *data))
{
	if (array->is_view) return;

	for (size_t i = 0; i < array->size; i++)
		destroyer(array->data + i * array->stride);
	free(array->data);
}

void
arr_fin_exx(struct array *array, void (*destroyer)(void *data, void *arg), void *arg)
{
	if (array->is_view) return; 

	for (size_t i = 0; i < array->size; i++)
		destroyer(array->data + i * array->stride, arg);
	free(array->data);
}

/* ---------- information retrieval ---------- */

extern size_t
arr_size(struct array *array);

extern size_t 
arr_capacity(struct array *array);

extern size_t 
arr_stride(struct array *array);

extern void *
arr_ix(struct array *array, size_t index);

/* ---------- manipulation ---------- */

extern void
arr_set(struct array *array, size_t index, void *data);

/* It may not be immediately obvious, where 'convert a view to an independent
 * array on an append/prepend' code is. It is in 'preallocate' since for all
 * views capacity is always equal to size, so preallocating code will always
 * be executed. Neat, saves a lot of typing.
 */

int
arr_append(struct array *array, void *data)
{
	if (array->size == array->capacity) {
		if (!arr_preallocate(array, array->size + 1))
			return 0;
	}
	memcpy(array->data + array->size * array->stride, data, array->stride);
	array->size++;
	return 1;
}

int
arr_prepend(struct array *array, void *data)
{
	if (array->size == array->capacity) {
		if (!arr_preallocate(array, array->size + 1))
			return 0;
	}
	memmove(array->data + array->stride, array->data, array->stride * array->size);
	memcpy(array->data, data, array->stride);
	array->size++;
	return 1;
}

int
arr_append_a(struct array *append_to, struct array *append)
{
	size_t new_size = append_to->size + append->size;
	if (new_size > append_to->capacity) {
		if (!arr_preallocate(append_to, new_size))
			return 0;
	}
	memcpy(append_to->data + append_to->size * append_to->stride,
			append->data, append->size * append->stride);
	append_to->size = new_size;
	return 1;
}

int
arr_prepend_a(struct array *prepend_to, struct array *prepend)
{
	size_t new_size = prepend_to->size + prepend->size;
	if (new_size > prepend_to->capacity) {
		if (!arr_preallocate(prepend_to, new_size))
			return 0;
	}
	memmove(prepend_to->data + prepend->size * prepend->stride,
			prepend_to->data, prepend_to->size * prepend_to->stride);
	memcpy(prepend_to->data, prepend->data, prepend->size * prepend->stride);
	prepend_to->size = new_size;
	return 1;
}

int
arr_preallocate(struct array *array, size_t new_capacity)
{
	if (new_capacity <= array->size) return 1;
	if (array->is_view) {
		void *new_data = malloc(new_capacity * array->stride);
		if (new_data == NULL) return 0;
		memcpy(new_data, array->data, array->size * array->stride);
		array->data = new_data;
	} else {
		void *new_data = realloc(array->data, new_capacity * array->stride);
		if (new_data == NULL) return 0;
		array->data = new_data;
	}
	array->capacity = new_capacity;
	return 1;
}

int
arr_shrink_to_fit(struct array *array)
{
	if (array->capacity == array->size) return 1;
	void *new_data = realloc(array->data, array->size * array->stride);
	if (new_data == NULL) return 0;
	array->capacity = array->size;
	array->data = new_data;
	return 1;
}

/* ---------- view manipulation ---------- */

struct array *
aview_create(struct array *array, size_t start, size_t end)
{
	struct array *res = malloc(sizeof(struct array));
	if (res == NULL) return NULL;
	aview_init(res, array, start, end);
	return res;
}

void
aview_init(struct array *view, struct array *array, size_t start, size_t end)
{
	view->data = array->data + start * array->stride;
	view->size = view->capacity = end - start;
	view->stride = array->stride;
	view->is_view = 1;
}

void
aview_shift(struct array *view, enum aview_dir which_end, enum aview_dir where, size_t by)
{
	if (which_end == AVIEW_LEFT && where == AVIEW_LEFT) {
		view->data = view->data - by * view->stride;
		view->size = view->capacity = view->size + by;
	} else if (which_end == AVIEW_LEFT && where == AVIEW_RIGHT) {
		if (by > view->size) {
			view->data = view->data + (view->size - 1) * view->stride;
			view->size = view->capacity = 0;
		} else {
			view->data = view->data + view->stride * by;
			view->size = view->capacity = view->size - by;
		}
	} else if (which_end == AVIEW_RIGHT && where == AVIEW_RIGHT) {
		view->size = view->capacity = view->size + by;
	} else {
		/* Move right end to the left. */
		if (by > view->size) {
			view->size = view->capacity = 0;
		} else {
			view->size = view->capacity = view->size - by;
		}
	}
}
