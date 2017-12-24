
#include <stdlib.h>
#include <string.h>

#include "list.h"

/* ---------- creation ---------- */

struct list *
list_create(void)
{
	struct list *res = malloc(sizeof(struct list));
	if (res == NULL) return NULL;

	res->first = res->last = NULL;
	return res;
}

struct list_elem *
list_elem_create(void *data)
{
	struct list_elem *res = malloc(sizeof(struct list_elem));
	if (res == NULL) return NULL;

	res->data = data;
	res->prev = res->next = NULL;
	return res;
}

/* ---------- destruction ---------- */

void
list_destroy(struct list *list)
{
	struct list_elem *cur = list->first;
	while (cur != NULL) {
		struct list_elem *next = cur->next;
		free(cur);
		cur = next;
	}
	free(list);
}

void
list_destroy_ex(struct list *list, void (*destroyer)(void *data))
{
	struct list_elem *cur = list->first;
	while (cur != NULL) {
		struct list_elem *next = cur->next;
		destroyer(cur->data);
		free(cur);
		cur = next;
	}
	free(list);
}

void
list_destroy_exx(struct list *list, void (*destroyer)(void *data, void *arg), void *arg)
{
	struct list_elem *cur = list->first;
	while (cur != NULL) {
		struct list_elem *next = cur->next;
		destroyer(cur->data, arg);
		free(cur);
		cur = next;
	}
	free(list);
}

/* ---------- insertion ---------- */

int
list_push(struct list *list, void *data)
{
	struct list_elem *new = list_elem_create(data);
	if (new == NULL) return 0;

	if (list->first == NULL) {
		list->first = list->last = new;
	} else {
		struct list_elem *old_head = list->first;
		list->first = new;
		new->next = old_head;
		old_head->prev = new;
	}
	return 1;
}

int
list_push_back(struct list *list, void *data)
{
	struct list_elem *new = list_elem_create(data);
	if (new == NULL) return 0;

	if (list->first == NULL) {
		list->first = list->last = new;
	} else {
		struct list_elem *old_tail = list->last;
		list->last = new;
		new->prev = old_tail;
		old_tail->next = new;
	}
	return 1;
}

int
list_insert_after(struct list *list, struct list_elem *after, void *data)
{
	struct list_elem *new = list_elem_create(data);
	if (new == NULL) return 0;

	after->next = new;
	new->prev = after;
	if (after == list->last) list->last = new;
	return 1;
}

int
list_insert_before(struct list *list, struct list_elem *before, void *data)
{
	struct list_elem *new = list_elem_create(data);
	if (new == NULL) return 0;

	before->prev = new;
	new->next = before;
	if (before == list->first) list->first = new;
	return 1;
}

int
list_append(struct list *to, struct list *append)
{
	struct list *copy = list_copy(append);
	if (copy == NULL) return 0;

	list_append_d(to, copy);
	return 1;
}

void
list_append_d(struct list *to, struct list *append)
{
	if (to->first == NULL) {
		to->first = append->first;
		to->last = append->last;
	} else {
		to->last->next = append->first;
		append->first->prev = to->last;
		to->last = append->last;
	}
	free(append);
}

int
list_prepend(struct list *to, struct list *prepend)
{
	struct list *copy = list_copy(prepend);
	if (copy == NULL) return 0;

	list_prepend_d(to, copy);
	return 1;
}

void
list_prepend_d(struct list *to, struct list *prepend)
{
	if (to->first == NULL) {
		to->first = prepend->first;
		to->last = prepend->last;
	} else {
		to->first->prev = prepend->last;
		prepend->last->next = to->first;
		to->first = prepend->first;
	}
	free(prepend);
}

/* ---------- popping ---------- */

void *
list_pop(struct list *list)
{
	struct list_elem *first = list->first;
	struct list_elem *next = first->next;

	list->first = next;
	if (list->last == first) list->last = NULL;
	if (next != NULL) next->prev = NULL;

	void *res = first->data;
	free(first);
	return res;
}

void *
list_pop_back(struct list *list)
{
	struct list_elem *last = list->last;
	struct list_elem *prev = last->prev;

	list->last = prev;
	if (list->first == last) list->first = NULL;
	if (prev != NULL) prev->next = NULL;

	void *res = last->data;
	free(last);
	return res;
}

void
list_clear(struct list *list)
{
	struct list_elem *cur = list->first;
	while (cur != NULL) {
		struct list_elem *next = cur->next;
		free(cur);
		cur = next;
	}
	list->first = list->last = NULL;
}

void
list_clear_ex(struct list *list, void (*destroyer)(void *data))
{
	struct list_elem *cur = list->first;
	while (cur != NULL) {
		struct list_elem *next = cur->next;
		destroyer(cur->data);
		free(cur);
		cur = next;
	}
	list->first = list->last = NULL;
}

void
list_clear_exx(struct list *list, void (*destroyer)(void *data, void *arg), void *arg)
{
	struct list_elem *cur = list->first;
	while (cur != NULL) {
		struct list_elem *next = cur->next;
		destroyer(cur->data, arg);
		free(cur);
		cur = next;
	}
	list->first = list->last = NULL;
}

/* ---------- information retrieval ---------- */

struct list_elem *
list_first(struct list *list)
{
	return list->first;
}

struct list_elem *
list_last(struct list *list)
{
	return list->last;
}

int
list_empty(struct list *list)
{
	return list->first == NULL;
}

struct list_elem *
list_nth(struct list *list, size_t n)
{
	struct list_elem *cur = list->first;
	size_t i = 0;
	while (cur != NULL) {
		if (i == n) return cur;
		cur = cur->next;
		i++;
	}
	return NULL;
}

struct list_elem *
list_nth_from_back(struct list *list, size_t n)
{
	struct list_elem *cur = list->last;
	size_t i = 0;
	while (cur != NULL) {
		if (i == n) return cur;
		cur = cur->prev;
		i++;
	}
	return NULL;
}

void *
list_data(struct list_elem *elem)
{
	return elem->data;
}

struct list_elem *
list_next(struct list_elem *elem)
{
	return elem->next;
}

struct list_elem *
list_prev(struct list_elem *elem)
{
	return elem->prev;
}

struct list_elem *
list_find(struct list *list, list_pred cond)
{
	struct list_elem *cur = list->first;
	while (cur != NULL) {
		if (cond(cur->data)) return cur;
		cur = cur->next;
	}
	return NULL;
}

struct list_elem *
list_find_ex(struct list *list, list_pred_ex cond, void *arg)
{
	struct list_elem *cur = list->first;
	while (cur != NULL) {
		if (cond(cur->data, arg)) return cur;
		cur = cur->next;
	}
	return NULL;
}

/* ---------- slicing ---------- */

struct lslice *
lslice_create(struct list *list, int cover_all)
{
	if (list->first == NULL) return NULL;
	struct lslice *res = malloc(sizeof(struct lslice));
	if (res == NULL) return NULL;

	res->list = list;
	res->first = list->first;
	res->last = cover_all ? list->last : list->first;
	res->empty = 0;
	return res;
}

int
lslice_init(struct lslice *slice, struct list *list, int cover_all)
{
	if (list->first == NULL) return 0;

	slice->list = list;
	slice->first = list->first;
	slice->last = cover_all ? list->last : list->first;
	slice->empty = 0;
	return 1;
}

struct lslice *
lslice_copy(struct lslice *slice)
{
	struct lslice *res = malloc(sizeof(struct lslice));
	if (res == NULL) return NULL;

	memcpy(res, slice, sizeof(struct lslice));
	return res;
}

void
lslice_init_from(struct lslice *init, struct lslice *from)
{
	memcpy(init, from, sizeof(struct lslice));
}

void
lslice_destroy(struct lslice *slice)
{
	free(slice);
}

struct list_elem *
lslice_start(struct lslice *slice)
{
	return slice->first;
}

struct list_elem *
lslice_end(struct lslice *slice)
{
	return slice->last;
}

int
lslice_empty(struct lslice *slice)
{
	return slice->empty;
}

struct list *
lslice_to_list(struct lslice *slice)
{
	struct list *res = list_create();
	if (res == NULL) return NULL;
	if (slice->empty) return res;

	struct list_elem *cur = slice->first;
	while (cur != slice->last) {
		if (!list_push_back(res, cur->data)) {
			list_destroy(res);
			return NULL;
		}
		cur = cur->next;
	}
	if (!list_push_back(res, cur->data)) {
		list_destroy(res);
		return NULL;
	}

	return res;
}

struct list *
lslice_extract(struct lslice *slice)
{
	struct list *res = list_create();
	if (res == NULL) return NULL;
	if (slice->empty) return res;

	struct list_elem *start = slice->first;
	struct list_elem *end = slice->last;
	res->first = start;
	res->last = end;

	struct list *list = slice->list;
	struct list_elem *prestart = start->prev;
	struct list_elem *postend = end->next;
	if (prestart == NULL && postend == NULL) {
		list->first = list->last = NULL;
	} else if (prestart == NULL && postend != NULL) {
		list->first = postend;
		postend->prev = NULL;
	} else if (prestart != NULL && postend == NULL) {
		list->last = prestart;
		prestart->next = NULL;
	} else {
		prestart->next = postend;
		postend->prev = prestart;
	}

	return res;
}

void
lslice_set_start(struct lslice *slice, struct list_elem *elem)
{
	slice->first = elem;

	struct list_elem *cur = elem->prev;
	slice->empty = 0;
	while (cur != NULL) {
		if (cur == slice->last) {
			slice->empty = 1;
			break;
		}
		cur = cur->prev;
	}
}

void
lslice_set_end(struct lslice *slice, struct list_elem *elem)
{
	slice->last = elem;

	struct list_elem *cur = elem->next;
	slice->empty = 0;
	while (cur != NULL) {
		if (cur == slice->first) {
			slice->empty = 1;
			break;
		}
		cur = cur->next;
	}
}

void
lslice_set_start_to_first(struct lslice *slice)
{
	slice->empty = 0;
	slice->first = slice->list->first;
}

void
lslice_set_end_to_last(struct lslice *slice)
{
	slice->empty = 0;
	slice->last = slice->list->last;
}

void
lslice_set_start_to_end(struct lslice *slice)
{
	slice->empty = 0;
	slice->first = slice->last;
}

void
lslice_set_end_to_start(struct lslice *slice)
{
	slice->empty = 0;
	slice->last = slice->first;
}

#define SHIFT_BODY(slice, which_end, dir, res, cur, while_cond) \
{ \
	size_t res = 0; \
	struct list_elem *cur; \
	int empty = slice->empty; \
	if (which_end == LSLICE_LEFT && dir == LSLICE_LEFT) { \
		/* Shifting the start of a slice to the left can only make an \
		 * empty slice non-empty. */ \
		cur = slice->first; \
		while (while_cond) { \
			if (empty && cur == slice->last)  \
				slice->empty = empty = 0; \
			cur = cur->prev; \
			res++; \
		} \
		slice->first = cur; \
	} else if (which_end == LSLICE_LEFT && dir == LSLICE_RIGHT) { \
		/* Shifting the start of a slice to the right can only make a \
		 * slice empty. */ \
		cur = slice->first; \
		int found_last = 0; \
		while (while_cond) { \
			if (!empty && found_last) \
				slice->empty = empty = 1; \
			if (!empty && !found_last && cur == slice->last) \
				found_last = 1; \
			cur = cur->next; \
			res++; \
		} \
		slice->first = cur; \
	} else if (which_end == LSLICE_RIGHT && dir == LSLICE_LEFT) { \
		/* Shifting the end of a slice to the left can only make a  \
		 * slice empty. */ \
		cur = slice->last; \
		int found_first = 0; \
		while (while_cond) { \
			if (!empty && found_first) \
				slice->empty = empty = 1; \
			if (!empty && !found_first && cur == slice->first) \
				found_first = 1; \
			cur = cur->prev; \
			res++; \
		} \
		slice->last = cur; \
	} else { \
		/* Shifting the end of a slice to the right can only make an \
		 * empty slice non-empty. */ \
		cur = slice->last; \
		while (while_cond) { \
			if (empty && cur == slice->first) \
				slice->empty = empty = 0; \
			cur = cur->next; \
			res++; \
		} \
	} /* if end and dir combination. */ \
	return res; \
} \

size_t 
lslice_shift(struct lslice *slice, enum lslice_dir which_end, size_t by,
		enum lslice_dir dir)
{
	SHIFT_BODY(slice, which_end, dir, res, cur, (cur != NULL && res < by));
}

size_t 
lslice_shift_while(struct lslice *slice, enum lslice_dir which_end, enum lslice_dir dir,
		list_pred cond)
{
	SHIFT_BODY(slice, which_end, dir, res, cur, (cur != NULL && cond(cur->data)));
}

size_t
lslice_shift_while_ex(struct lslice *slice, enum lslice_dir which_end, enum lslice_dir dir,
		list_pred_ex cond, void *arg)
{
	SHIFT_BODY(slice, which_end, dir, res, cur, (cur != NULL && cond(cur->data, arg)));
}

/* ---------- other ---------- */

struct list *
list_sort(struct list *list, int (*cmp)(void *left, void *right), int desc)
{
	return NULL;
}

/* ---------- helper things ---------- */
