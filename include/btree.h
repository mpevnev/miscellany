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
	struct btree *parent;
	struct btree *child[2]; /* 0 is left, 1 is right. */
};

/* These should return a negative value if (left < right), 0 if (left == right)
 * and a positive value if (left > right). 
 * */
typedef int (*btree_cmp_fn)(void *left, void *right);
typedef int (*btree_cmp_ex_fn)(void *left, void *right, void *external_arg);

/* ---------- functions ---------- */

/* Creation. */

extern struct btree *
btree_create(void *data);

/* Destruction. */

/* Note that you can freely destroy subtrees, the parent tree (if there's one)
 * will be updated to exclude the destroyed subtree. */

extern void 
btree_destroy(struct btree *);

extern void
btree_destroy_ex(struct btree *, void (*destroyer)(void *data));

extern void 
btree_destroy_exx(struct btree *, void (*destroyer)(void *data, void *arg), void *arg);

/* Accessing struct members and information retrieval. */

#define btree_data(tree) (tree->data)
#define btree_left(tree) (tree->child[0])
#define btree_right(tree) (tree->child[1])
#define btree_parent(tree) (tree->parent)
#define btree_child(tree, dir) (tree->child[!!dir])

#define btree_has_children(tree) (tree->child[0] != NULL || tree->child[1] != NULL)
#define btree_is_a_leaf(tree) (tree->child[0] == NULL && tree->child[1] == NULL)
#define btree_has_parent(tree) (tree->parent != NULL)

/* Searching. */

/* Return 1 if the element was found (and fill res with it), return 0 and do
 * nothing with 'res' otherwise. */
extern int
btree_find(struct btree *tree, void *data, btree_cmp_fn cmp, void **res);

/* Return 1 if the element was found (and fill res with it), return 0 and do
 * nothing with 'res' otherwise. */
extern int
btree_find_ex(struct btree *tree, void *data, btree_cmp_ex_fn cmp, void *cmp_arg, void **res);

extern struct btree *
btree_find_node(struct btree *tree, void *data, btree_cmp_fn cmp);

extern struct btree *
btree_find_node_ex(struct btree *tree, void *data, btree_cmp_ex_fn cmp, void *cmp_arg);

/* These two only search down the tree. */
extern struct btree *
btree_predecessor(struct btree *tree);

extern struct btree *
btree_successor(struct btree *tree);

/* Insertion. */

/* Return the freshly created node, or NULL on an OOM condition. */
extern struct btree *
btree_insert(struct btree *tree, void *data, btree_cmp_fn cmp);

/* Return the freshly created node, or NULL on an OOM condition. */
extern struct btree *
btree_insert_ex(struct btree *tree, void *data, btree_cmp_ex_fn cmp, void *arg);

/* Deletion. */

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

/* Other. */

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
 * original's parent. */
extern struct btree *
btree_copy(struct btree *tree);

/* Unlinks the given subtree from its parent tree. */
extern void
btree_unlink(struct btree *tree);

#endif /* BTREE_H */
