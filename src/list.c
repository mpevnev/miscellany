
#include <stdlib.h>
#include <string.h>

#include "list.h"

/* ---------- helper functions declarations and some helper macros ---------- */

static void 
destroy_nested(void *list);

static int
sort_step(struct list **groups, int (*cmp)(void *, void *), int desc);

static int
sort_step_ex(struct list **groups, int (*cmp)(void *, void *, void *), int desc, void *arg);

static struct list *
merge(struct list *group_a, struct list *group_b, 
		int (*cmp)(void *left, void *right), int desc);

static struct list *
merge_ex(struct list *group_a, struct list *group_b,
		int (*cmp)(void *left, void *right, void *arg), int desc, void *arg);

static struct list *
split_into_groups(struct list *list);

static int
append_trailing(struct list *next_groups, struct list_elem *trailing_group);

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

	list_append_d(to, copy, 1);
	return 1;
}

void
list_append_d(struct list *to, struct list *append, int do_free)
{
	if (to->first == NULL) {
		to->first = append->first;
		to->last = append->last;
	} else {
		to->last->next = append->first;
		append->first->prev = to->last;
		to->last = append->last;
	}
	append->first = append->last = NULL;
	if (do_free) free(append);
}

int
list_prepend(struct list *to, struct list *prepend)
{
	struct list *copy = list_copy(prepend);
	if (copy == NULL) return 0;

	list_prepend_d(to, copy, 1);
	return 1;
}

void
list_prepend_d(struct list *to, struct list *prepend, int do_free)
{
	if (to->first == NULL) {
		to->first = prepend->first;
		to->last = prepend->last;
	} else {
		to->first->prev = prepend->last;
		prepend->last->next = to->first;
		to->first = prepend->first;
	}
	prepend->first = prepend->last = NULL;
	if (do_free) free(prepend);
}

void
list_extract(struct list *into, struct list *from, struct list_elem *elem)
{
	struct list_elem *prev = elem->prev;
	struct list_elem *next = elem->next;
	if (elem == from->first)
		from->first = next;
	if (elem == from->last)
		from->last = prev;
	if (prev != NULL) 
		prev->next = next;
	if (next != NULL)
		next->prev = prev;

	struct list_elem *into_head = into->first;
	if (into_head == NULL) {
		into->first = into->last = elem;
		elem->next = elem->prev = NULL;
	} else {
		into->first = elem;
		elem->next = into_head;
		elem->prev = NULL;
		into_head->prev = elem;
	}
}

void
list_extract_back(struct list *into, struct list *from, struct list_elem *elem)
{
	struct list_elem *prev = elem->prev;
	struct list_elem *next = elem->next;
	if (elem == from->first)
		from->first = next;
	if (elem == from->last)
		from->last = prev;
	if (prev != NULL) 
		prev->next = next;
	if (next != NULL)
		next->prev = prev;

	struct list_elem *into_tail = into->last;
	if (into_tail == NULL) {
		into->first = into->last = elem;
		elem->next = elem->prev = NULL;
	} else {
		into->last = elem;
		elem->prev = into_tail;
		elem->next = NULL;
		into_tail->next = elem;
	}
}

/* ---------- removal ---------- */

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

void
list_remove(struct list *from, struct list_elem *elem)
{
	struct list_elem *prev = elem->prev;
	struct list_elem *next = elem->next;
	if (prev != NULL) prev->next = next;
	if (next != NULL) next->prev = prev;
	if (from->first == elem) from->first = next;
	if (from->last == elem) from->last = prev;
}

/* ---------- information retrieval ---------- */

extern struct list_elem *
list_first(struct list *list);

struct list_elem *
list_last(struct list *list);

size_t
list_length(struct list *list)
{
	struct list_elem *cur = list->first;
	size_t res = 0;
	while (cur != NULL) {
		res++;
		cur = cur->next;
	}
	return res;
}

int
list_empty(struct list *list);

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
list_data(struct list_elem *elem);

struct list_elem *
list_next(struct list_elem *elem);

struct list_elem *
list_prev(struct list_elem *elem);

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

struct list_elem *
list_find_eq(struct list *list, void *data, int (*eq)(void *, void *))
{
	struct list_elem *cur = list->first;
	while (cur != NULL) {
		if (eq(cur->data, data)) return cur;
		cur = cur->next;
	}
	return NULL;
}

struct list_elem *
list_find_eq_ex(struct list *list, void *data, 
		int (*eq)(void *data1, void *data2, void *arg), void *arg)
{
	struct list_elem *cur = list->first;
	while (cur != NULL) {
		if (eq(cur->data, data, arg)) return cur;
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
	struct list *groups = split_into_groups(list);
	if (groups == NULL) return NULL;

	size_t len = list_length(list);
	size_t group_size = 1;
	if (len == 0) return groups;
	while (group_size < len) {
		if (!sort_step(&groups, cmp, desc)) {
			list_destroy_ex(groups, &destroy_nested);
			return NULL;
		}
		group_size <<= 1;
	}

	/* After the loop is done, 'groups' will contain a single sorted list. */
	struct list *res = list_pop(groups);
	list_destroy(groups);
	return res;
}

struct list *
list_sort_ex(struct list *list, int (*cmp)(void *left, void *right, void *arg),
		int desc, void *arg)
{
	struct list *groups = split_into_groups(list);
	if (groups == NULL) return NULL;

	size_t len = list_length(list);
	size_t group_size = 1;
	if (len == 0) return groups;
	while (group_size < len) {
		if (!sort_step_ex(&groups, cmp, desc, arg)) {
			list_destroy_ex(groups, &destroy_nested);
			return NULL;
		}
		group_size <<= 1;
	}

	/* After the loop is done, 'groups' will contain a single sorted list. */
	struct list *res = list_pop(groups);
	list_destroy(groups);
	return res;
}

struct list *
list_copy(struct list *list)
{
	struct list *res = list_create();
	if (res == NULL) return NULL;

	struct list_elem *cur = list->first;
	while (cur != NULL) {
		if (!list_push_back(res, cur->data)) {
			list_destroy(res);
			return NULL;
		}
		cur = cur->next;
	}
	return res;
}

void **
list_to_array(struct list *list, size_t *size)
{
	size_t len = list_length(list);
	void **res = malloc(len * sizeof(void *));
	if (res == NULL) return NULL;
	if (size != NULL) *size = len;

	struct list_elem *cur = list->first;
	void **pos = res;
	while (cur != NULL) {
		*pos = cur->data;
		cur = cur->next;
		pos++;
	}

	return res;
}

/* ---------- helper things ---------- */

/* A helper function for sorting algorithm. */
static void
destroy_nested(void *list)
{
	list_destroy(list);
}

#define MERGE_BODY(group_a, group_b, cmp, desc, cur_a, cur_b, cmp_line) \
{ \
	struct list *res = list_create(); \
	if (res == NULL) return NULL; \
 \
	struct list_elem *cur_a = group_a->first; \
	struct list_elem *cur_b = group_b->first; \
	while (cur_a != NULL && cur_b != NULL) { \
		int cmp_res = cmp_line; \
		struct list_elem *next_a = cur_a->next; \
		struct list_elem *next_b = cur_b->next; \
		/* Figure out which data to take. */ \
		if (desc && cmp_res < 0 || !desc && cmp_res > 0) { \
			list_extract_back(res, group_b, cur_b); \
			cur_b = next_b; \
		} else { \
			list_extract_back(res, group_a, cur_a); \
			cur_a = next_a; \
		} \
	} \
	if (cur_a != NULL) list_append_d(res, group_a, 0); \
	if (cur_b != NULL) list_append_d(res, group_b, 0); \
	return res; \
} \

/* A helper function for sorting algorithm to merge two lists. 
 * Both groups will be exausted after the function is done.
 * Return NULL on an OOM condition. */
static struct list *
merge(struct list *group_a, struct list *group_b,
		int (*cmp)(void *left, void *right), int desc)
{
	MERGE_BODY(group_a, group_b, cmp, desc, cur_a, cur_b, 
			cmp(cur_a->data, cur_b->data));
}

/* Same, but the comparison function takes an extra argument. */
static struct list *
merge_ex(struct list *group_a, struct list *group_b,
		int (*cmp)(void *left, void *right, void *arg), int desc, void *arg)
{
	MERGE_BODY(group_a, group_b, cmp, desc, cur_a, cur_b,
			cmp(cur_a->data, cur_b->data, arg));
}

static struct list *
split_into_groups(struct list *list)
{
	struct list *groups = list_create();
	if (groups == NULL) return NULL;

	struct list_elem *cur = list->first;
	while (cur != NULL) {
		struct list *new_list = list_create();
		if (new_list == NULL) {
			list_destroy_ex(groups, &destroy_nested);
			return NULL;
		}
		if (!list_push(new_list, cur->data)) {
			list_destroy_ex(groups, &destroy_nested);
			list_destroy(new_list);
			return NULL;
		}
		if (!list_push_back(groups, new_list)) {
			list_destroy_ex(groups, &destroy_nested);
			list_destroy(new_list);
			return NULL;
		}
		cur = cur->next;
	}
	return groups;
}

static int
append_trailing(struct list *next_groups, struct list_elem *trailing_group)
{
	struct list *extract = list_create();
	if (extract == NULL) return 0;
	list_append_d(extract, trailing_group->data, 0);
	if (!list_push_back(next_groups, extract)) {
		list_destroy(extract);
		return 0;
	}
	return 1;
}

static int
sort_step(struct list **groups, int (*cmp)(void *, void *), int desc)
{
	struct list *new_groups = list_create();
	if (new_groups == NULL) return 0;

	struct list_elem *first = (*groups)->first;
	struct list_elem *second = first->next;
	while (first != NULL && second != NULL) {
		struct list *merged = merge(first->data, second->data, cmp, desc);
		if (merged == NULL) {
			list_destroy_ex(new_groups, &destroy_nested);
			return 0;
		}
		if (!list_push_back(new_groups, merged)) {
			list_destroy(merged);
			list_destroy_ex(new_groups, &destroy_nested);
			return 0;
		}
		first = second->next;
		second = first == NULL ? NULL : first->next;
	}
	if (first != NULL)
		append_trailing(new_groups, first);
	list_destroy_ex(*groups, &destroy_nested);
	*groups = new_groups;
	return 1;
}

static int
sort_step_ex(struct list **groups, int (*cmp)(void *, void *, void *), int desc, void *arg)
{
	struct list *new_groups = list_create();
	if (new_groups == NULL) return 0;

	struct list_elem *first = (*groups)->first;
	struct list_elem *second = first->next;
	while (first != NULL && second != NULL) {
		struct list *merged = merge_ex(first->data, second->data, cmp, desc, arg);
		if (merged == NULL) {
			list_destroy_ex(new_groups, &destroy_nested);
			return 0;
		}
		if (!list_push_back(new_groups, merged)) {
			list_destroy(merged);
			list_destroy_ex(new_groups, &destroy_nested);
			return 0;
		}
		first = second->next;
		second = first == NULL ? NULL : first->next;
	}
	if (first != NULL)
		append_trailing(new_groups, first);
	list_destroy_ex(*groups, &destroy_nested);
	*groups = new_groups;
	return 1;
}
