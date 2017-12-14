#ifndef BTREE_H
#define BTREE_H

/** A binary tree module. 
 *
 * Based on a tutorial by Eternally Confuzzled. 
 *
 */

/* ---------- base stuff ---------- */

struct btree
{
	void *data;
	/* 0 for left, 1 for right. */
	int thread[2];
	struct btree *link[2]; /* A threaded link or a child. */
};

/* These should return a negative value if (left < right), 0 if (left == right)
 * and a positive value if (left > right). 
 * */
typedef int (*btree_cmp_fn)(void *left, void *right);
typedef int (*btree_cmp_ex_fn)(void *left, void *right, void *external_arg);

/* ---------- creation ---------- */

extern struct btree *
btree_create(void *data);

/* ---------- destruction ---------- */

/* Note that you can freely destroy subtrees, the parent tree (if there's one)
 * will be updated to exclude the destroyed subtree. */

extern void 
btree_destroy(struct btree *);

extern void
btree_destroy_ex(struct btree *, void (*destroyer)(void *data));

extern void 
btree_destroy_exx(struct btree *, void (*destroyer)(void *data, void *arg), void *arg);

/* ---------- accessing struct members and information retrieval ---------- */

#define btree_data(tree) (tree->data)
#define btree_left(tree) (tree->child[0])
#define btree_right(tree) (tree->child[1])
#define	btree_thread(tree, dir) (tree->is_thread[!!dir])
#define btree_link(tree, dir) (tree->child[!!dir])

extern int
btree_has_children(struct btree *);

/* Return the index of the link in the links array of the given tree, or -1 if
 * the link is not there. */
extern int
btree_link_dir(struct btree *tree, struct btree *link);

/* ---------- searching ---------- */

/* Return 1 if the element was found (and fill res with it), return 0 and do
 * nothing with 'res' otherwise. Pass NULL as 'res' to avoid filling it with
 * anything. */
extern int
btree_find(struct btree *tree, void *data, btree_cmp_fn cmp, void **res);

/* Return 1 if the element was found (and fill res with it), return 0 and do
 * nothing with 'res' otherwise. Pass NULL as 'res' to avoid filling it with
 * anything. */
extern int
btree_find_ex(struct btree *tree, void *data, btree_cmp_ex_fn cmp, void *cmp_arg, void **res);

extern struct btree *
btree_find_node(struct btree *tree, void *data, btree_cmp_fn cmp);

extern struct btree *
btree_find_node_ex(struct btree *tree, void *data, btree_cmp_ex_fn cmp, void *cmp_arg);

extern struct btree *
btree_parent(struct btree *tree);

/* Note that this searches for a successor of the entire subtree. */
extern struct btree *
btree_successor(struct btree *tree);

/* Note that this searches for a predecessor of the entire subtree. */
extern struct btree *
btree_predecessor(struct btree *tree);

/* Return either left or right outermost subnode of the subtree. */
extern struct btree *
btree_outermost(struct btree *tree, int dir);

/* Return either predecessor of the leftmost subnode or a successor of the 
 * rightmost subnode. */
extern struct btree *
btree_after_outermost(struct btree *tree, int dir);

/* ---------- insertion ---------- */

/* Return the freshly created node, or NULL on an OOM condition. */
extern struct btree *
btree_insert(struct btree *tree, void *data, btree_cmp_fn cmp);

/* Return the freshly created node, or NULL on an OOM condition. */
extern struct btree *
btree_insert_ex(struct btree *tree, void *data, btree_cmp_ex_fn cmp, void *arg);

/* ---------- deletion ---------- */

/* There's no function to delete subtrees - this is done with btree_destroy[_ex]. */

/* Return 1 if the data was found in the tree, 0 otherwise. Fills 'tree_after'
 * with the tree after the deletion of the data (might be NULL if the last node
 * in the tree was deleted). Fills 'deleted' with the data from the tree that
 * was deleted (not from the 'data' argument! they might be different depending
 * on what 'cmp' function does). */
extern int
btree_delete(struct btree *tree, void *data, btree_cmp_fn cmp, 
		struct btree **tree_after, void **deleted);

/* Ditto. */
extern int
btree_delete_ex(struct btree *tree, void *data, btree_cmp_ex_fn cmp, void *arg,
		struct btree **tree_after, void **deleted);

/* ---------- other ---------- */

extern void
btree_rebalance(struct btree *tree, btree_cmp_fn cmp);

extern void
btree_rebalance_ex(struct btree *tree, btree_cmp_ex_fn cmp, void *cmp_arg);

extern void
btree_reorder(struct btree *tree, btree_cmp_fn new_cmp);

extern void
btree_reorder_ex(struct btree *tree, btree_cmp_ex_fn new_cmp, void *cmp_arg);

/* This is shallow copying. Also note that if you're copying a subtree, its 
 * copy will be made into a standalone tree not referring in any way to the
 * original's parent tree. */
extern struct btree *
btree_copy(struct btree *tree);

/* Unlink the given subtree from its tree. */
extern void
btree_unlink(struct btree *tree);

#endif /* BTREE_H */