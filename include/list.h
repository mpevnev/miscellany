#ifndef LIST_H
#define LIST_H

/** A doubly-linked list module.
 *
 */

/* ---------- base stuff ---------- */

struct list
{
	struct list_elem *first, *last;
};

struct list_elem
{
	void *data;
	struct list_elem *prev, *next;
};

struct lslice
{
	struct list *list;
	struct list_elem *first, *last;
	int empty;
};

enum lslice_dir
{
	LSLICE_LEFT,
	LSLICE_RIGHT,
};

typedef int (*list_pred)(void *);
typedef int (*list_pred_ex)(void *data, void *arg);

/* ---------- creation ---------- */

extern struct list *
list_create(void);

/* ---------- destruction ---------- */

extern void
list_destroy(struct list *);

extern void 
list_destroy_ex(struct list *, void (*destroyer)(void *data));

extern void
list_destroy_exx(struct list *, void (*destroyer)(void *data, void *arg), void *arg);

/* ---------- insertion ---------- */

/* All of these may fail on an OOM condition. If they do, they return 0. */

extern int
list_push(struct list *, void *data);

extern int
list_push_back(struct list *, void *data);

extern int
list_insert_after(struct list *, struct list_elem *after, void *data);

extern int
list_insert_before(struct list *, struct list_elem *before, void *data);

/* Append a list to another list, creating a copy of the list to be appended. */
extern int
list_append(struct list *to, struct list *append);

/* Append a list destructively. The resulting list will reuse elements of the
 * 'append' argument, avoiding copying it.
 * Optionally free the appended list.
 */
extern void
list_append_d(struct list *to, struct list *append, int do_free);

/* Prepend a list to another list, creating a copy of the list to be prepended. */
extern int
list_prepend(struct list *to, struct list *prepend);

/* Prepend a list destructively. The resulting list will reuse elements of the
 * 'prepend' argument, avoiding copying it.
 * Optionally free the prepended list.
 */
extern void
list_prepend_d(struct list *to, struct list *prepend, int do_free);

/* Take an element from 'from' and prepend it to 'into'. No memory allocations
 * or deallocations are performed. */
extern void
list_extract(struct list *into, struct list *from, struct list_elem *which_elem);

/* Take an element from 'from' and append it to 'into'. No memory allocations
 * or deallocations are performed. */
extern void
list_extract_back(struct list *to, struct list *from, struct list_elem *which_elem);

/* ---------- removal ---------- */

extern void *
list_pop(struct list *);

extern void *
list_pop_back(struct list *);

extern void 
list_clear(struct list *);

extern void
list_clear_ex(struct list *, void (*destroyer)(void *data));

extern void 
list_clear_exx(struct list *, void (*destroyer)(void *data, void *arg), void *arg);

/* The element becomes detached in process. It's up to the user to 'free' it. *
 * For convenience, the detached element still points to its previous and next
 * elements. */
extern void 
list_remove(struct list *, struct list_elem *);

/* ---------- information retrieval ---------- */

extern struct list_elem *
list_first(struct list *);

extern struct list_elem *
list_last(struct list *);

extern size_t
list_length(struct list *);

extern int
list_empty(struct list *);

/* Return NULL if the list is shorter than 'n'. Zero-indexed. */
extern struct list_elem *
list_nth(struct list *, size_t n);

/* Return NULL if the list is shorter than 'n'. Zero-indexed. */
extern struct list_elem *
list_nth_from_back(struct list *, size_t n);

extern void *
list_data(struct list_elem *);

extern struct list_elem *
list_next(struct list_elem *);

extern struct list_elem *
list_prev(struct list_elem *);

/* Find the first element upon which cond(elem->data) returns true. */
extern struct list_elem *
list_find(struct list *, list_pred cond);

/* Find the first element upon which cond(elem->data, arg) returns true. */
extern struct list_elem *
list_find_ex(struct list *, list_pred_ex cond, void *arg);

/* Find the first element that compares equal to a given data. */
extern struct list_elem *
list_find_eq(struct list *, void *data, int (*eq)(void *, void *));

extern struct list_elem *
list_find_eq_ex(struct list *, void *data, 
		int (*eq)(void *data1, void *data2, void *arg), void *arg);

/* ---------- slicing ---------- */

/* If 'cover_all' is true, the resulting slice will contain all of the list,
 * otherwise it'll contain just the first element. 
 * Return NULL on an OOM condition or if the list is empty. */
extern struct lslice *
lslice_create(struct list *, int cover_all);

/* A non-allocating version of the above. 
 * Return 0 if the list is empty, 1 otherwise. */
extern int 
lslice_init(struct lslice *, struct list *, int cover_all);

/* May return NULL on an OOM condition. */
extern struct lslice *
lslice_copy(struct lslice *);

extern void
lslice_init_from(struct lslice *init, struct lslice *from);

extern void
lslice_destroy(struct lslice *);

/* Note that this function doesn't detect whether or not the beginning of a 
 * slice is beyond its end. */
extern struct list_elem *
lslice_start(struct lslice *);

/* Note that this function doesn't detect whether or not the beginning of a 
 * slice is beyond its end. */
extern struct list_elem *
lslice_end(struct lslice *);

extern int
lslice_empty(struct lslice *);

/* Convert a slice to a list, leaving the original list untouched.
 * Return NULL on an OOM condition. */
extern struct list *
lslice_to_list(struct lslice *);

/* Extract a slice from its list, destructively.
 * Return NULL on an OOM condition. */
extern struct list *
lslice_extract(struct lslice *);

/* A bit on terminolgy used here: start and end refer to slice's first and last
 * elements respectively, first and last - to those of the list itself. */

extern void
lslice_set_start(struct lslice *, struct list_elem *);

extern void 
lslice_set_end(struct lslice *, struct list_elem *);

extern void
lslice_set_start_to_first(struct lslice *);

extern void
lslice_set_end_to_last(struct lslice *);

extern void
lslice_set_start_to_end(struct lslice *);

extern void
lslice_set_end_to_start(struct lslice *);

/* Neither of the following shift functions will move an end of a slice beyond
 * the last or the first element of the list. */

/* Shift either the left or the right end of a slice (controlled by 'end') in a
 * given direction ('dir') by a given number of elements ('by').
 * This may result in an empty slice if either end moves past another.
 * Return the number of traversed elements. */
extern size_t
lslice_shift(struct lslice *, enum lslice_dir end, size_t by, enum lslice_dir dir);

/* Same, but shift while cond(current_list_element->data) is true. */
extern size_t
lslice_shift_while(struct lslice *, enum lslice_dir end, enum lslice_dir dir,
		list_pred cond);

/* Same, but 'cond' takes an extra 'void *' argument. */
extern size_t
lslice_shift_while_ex(struct lslice *, enum lslice_dir end, enum lslice_dir dir,
		list_pred_ex cond, void *cond_arg);

/* ---------- other ---------- */

/* If 'desc' is false, sort the list in ascending order, otherwise sort it in
 * descending order.
 * 'cmp' should return a negative value if 'left < right', 0 if they are equal,
 * and a positive value if 'left > right'. 
 * Return a new sorted list, or NULL on an OOM condition. */
struct list *
list_sort(struct list *, int (*cmp)(void *left, void *right), int desc);

/* Same, but 'cmp' takes an extra argument. */
struct list *
list_sort_ex(struct list *, int (*cmp)(void *left, void *right, void *external_arg),
		int desc, void *external_arg);

/* A shallow copy. */
struct list *
list_copy(struct list *);

void **
list_to_array(struct list *, size_t *size);

#endif /* LIST_H */
