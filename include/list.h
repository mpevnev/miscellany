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

extern int
list_append(struct list *to, struct list *append);

extern int
list_prepend(struct list *to, struct list *prepend);

/* ---------- popping ---------- */

extern void *
list_pop(struct list *);

extern void *
list_pop_back(struct list *);

/* ---------- information retrieval ---------- */

extern struct list_elem *
list_first(struct list *);

extern struct list_elem *
list_last(struct list *);

extern struct list_elem *
list_nth(struct list *, size_t n);

extern struct list_elem *
list_nth_from_back(struct list *, size_t n);

extern void *
list_data(struct list_elem *);

extern struct list_elem *
list_next(struct list_elem *);

extern struct list_elem *
list_prev(struct list_elem *);

/* ---------- slicing ---------- */

extern struct list_slice *
lslice_create(struct list *);

/* A non-allocating version of the above. */
extern void
lslice_init(struct lslice *, struct list *);

extern void
lslice_destroy(struct lslice *);

extern struct list_elem *
lslice_first(struct lslice *);

extern struct list_elem *
lslice_last(struct lslice *);

extern int
lslice_empty(struct lslice *);

/* Convert a slice to a list, leaving the original list untouched. */
extern struct list *
lslice_to_list(struct lslice *);

/* Extract a slice from its list, destructively. */
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

/* Same, but shift while cond(current_list_element_data) is true. */
extern size_t
lslice_shift_while(struct lslice *, enum lslice_dir end, enum lslice_dir dir,
		list_pred cond);

/* Same, but 'cond' takes an extra 'void *' argument. */
extern size_t
lslice_shift_while_ex(struct lslice *, enum lslice_dir end, enum lslice_dir dir,
		list_pred_ex cond, void *cond_arg);

/* ---------- other ---------- */

/* Sort the list in-place in ascending order, if 'desc' is 0, otherwise sort in
 * descending order. 
 * 'cmp' should return a negative value if 'left < right', 0 if they are equal,
 * and a positive value if 'left > right'. */
void
list_sort(struct list *, int (*cmp)(void *left, void *right), int desc);

/* Same, but 'cmp' takes an extra argument. */
void
list_sort_ex(struct list *, int (*cmp)(void *left, void *right, void *external_arg),
		void *external_arg);

#endif /* LIST_H */
