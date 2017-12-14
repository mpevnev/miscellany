
#include <stdlib.h>

#include "btree.h"

/* ---------- creation ---------- */

struct btree *
btree_create(void *data)
{
	struct btree *res = malloc(sizeof(struct btree));
	if (res == NULL) return NULL;

	res->data = data;
	res->thread[0] = 0;
	res->thread[1] = 0;
	res->link[0] = NULL;
	res->link[1] = NULL;


	return res;
}

/* ---------- destruction ---------- */

#if 0

/* A helper macro to avoid duplicating destruction code thrice. */
#define DESTROY_PROTO(tree, cur, free_line) \
{ \
	btree_unlink(tree); \
	struct btree *cur = tree; \
	while (cur != NULL) { \
		/* Find a leaf, delete it, find the next closest leaf, and so on. */ \
		while (btree_has_children(cur)) { \
			struct btree *left = cur->child[0]; \
			if (left != NULL) { \
				cur = left; \
				continue; \
			} \
			struct btree *right = cur->child[1]; \
			if (right != NULL) { \
				cur = right; \
				continue; \
			} \
		} \
		/* Now 'cur' holds a leaf. */ \
		struct btree *next = cur->parent; \
		if (next != NULL) { \
			if (next->child[0] == cur) \
				next->child[0] = NULL; \
			else if (next->child[1] == cur) \
				next->child[1] = NULL; \
		} \
		cur = next; \
		free_line; \
		free(cur); \
	} \
}

void 
btree_destroy(struct btree *tree)
{
	DESTROY_PROTO(tree, cur, ;);
}

void
btree_destroy_ex(struct btree *tree, void (*destroyer)(void *data))
{
	DESTROY_PROTO(tree, cur, destroyer(cur->data));
}

void 
btree_destroy_exx(struct btree *tree, void (*destroyer)(void *data, void *arg), void *arg)
{
	DESTROY_PROTO(tree, cur, destroyer(cur->data, arg));
}

#endif

/* ---------- information retrieval ---------- */

int
btree_has_children(struct btree *tree)
{
	for (int i = 0; i < 2; i++) {
		if (tree->thread[i]) continue;
		if (tree->link[i] == NULL) continue;
		return 1;
	}
	return 0;
}

int
btree_link_dir(struct btree *tree, struct btree *link)
{
	for (int i = 0; i < 2; i++)
		if (tree->link[i] == link) return i;
	return -1;
}

/* ---------- searching ---------- */

int
btree_find(struct btree *tree, void *data, btree_cmp_fn cmp, void **res)
{
	struct btree *found = btree_find_node(tree, data, cmp);
	if (found == NULL) return 0;
	if (res != NULL) *res = found->data;
	return 1;
}

int 
btree_find_ex(struct btree *tree, void *data, btree_cmp_ex_fn cmp, void *arg, void **res)
{
	struct btree *found = btree_find_node_ex(tree, data, cmp, arg);
	if (found == NULL) return 0;
	if (res != NULL) *res = found->data;
	return 1;
}

struct btree *
btree_find_node(struct btree *tree, void *data, btree_cmp_fn cmp)
{
	while(1) {
		int cmp_res = cmp(data, tree->data);
		if (cmp_res == 0) return tree;

		int dir = cmp_res > 0;
		if (tree->thread[dir] || tree->link[dir] == NULL) return NULL;
		tree = tree->link[dir];
	}
}

struct btree *
btree_find_node_ex(struct btree *tree, void *data, btree_cmp_ex_fn cmp, void *cmp_arg)
{
	while(1) {
		int cmp_res = cmp(data, tree->data, cmp_arg);
		if (cmp_res == 0) return tree;

		int dir = cmp_res > 0;
		if (tree->thread[dir] || tree->link[dir] == NULL) return NULL;
		tree = tree->link[dir];
	}
}

struct btree *
btree_parent(struct btree *tree)
{
	/* If the subtree is a left node, then the parent is the successor of
	 * its rightmost node. If the subtree is a right node, then the parent
	 * is the predecessor of its leftmost node. */
	for (int i = 0; i < 2; i++) {
		struct btree *outer = btree_outermost(tree, i);
		/* If the node is not threaded, then we've walked in the wrong 
		 * direction to the outermost element of the entire tree. */
		if (!outer->thread[i]) continue;
		struct btree *res = outer->link[i];
		/* If it's threaded, we could still have walked in the wrong
		 * direction. Need to check if 'res' is linked to the 'tree'. */
		int is_linked = btree_link_dir(res, tree) >= 0;
		if (is_linked) return res;
	}
	return NULL;
}

struct btree *
btree_predecessor(struct btree *tree)
{
	return btree_after_outermost(tree, 0);
}

struct btree *
btree_successor(struct btree *tree)
{
	return btree_after_outermost(tree, 1);
}

struct btree *
btree_outermost(struct btree *tree, int dir)
{
	dir = !!dir;
	while (!tree->thread[dir] && tree->link[dir] != NULL)
		tree = tree->link[dir];
	return tree;
}

struct btree *
btree_after_outermost(struct btree *tree, int dir)
{
	struct btree *outer = btree_outermost(tree, dir);
	return outer->link[dir];
}

/* ---------- insertion ---------- */

/* A helper macro to avoid code duplication between insert functions. They only
 * differ in a single line, where data comparison takes place. */
#define INSERT_BODY(tree, data, cmp, cmp_line) \
{ \
	struct btree *res = btree_create(data); \
	if (res == NULL) return NULL; \
 \
	while (1) { \
		int cmp_res = cmp_line; \
		int dir = cmp_res > 0; \
		if (tree->link[dir] == NULL || tree->thread[dir]) { \
			struct btree *old_link = tree->link[dir]; \
			/* If a node is inserted to the right, it gets its \
			 * parent's right thread and the parent as its left \
			 * thread. If a node is inserted to the left, the  \
			 * situation is mirrored. */ \
			res->thread[dir] = old_link != NULL; \
			res->link[dir] = old_link; \
			res->thread[!dir] = 1; \
			res->link[!dir] = tree; \
			/* The parent gets the created node as its child. */ \
			tree->thread[dir] = 0; \
			tree->link[dir] = res; \
			/* The threaded node, if it exists, gets threaded to \
			 * the inserted node if it doesn't have a child in this \
			 * direction. */ \
			if (old_link == NULL) return res; \
			if (!old_link->thread[!dir]) return res; \
			old_link->thread[!dir] = 1; \
			old_link->link[!dir] = res; \
			return res; \
		} else { \
			tree = tree->link[dir]; \
		} /* if found insertion location */ \
	} /* while 1 */ \
} 

struct btree *
btree_insert(struct btree *tree, void *data, btree_cmp_fn cmp)
{
	INSERT_BODY(tree, data, cmp, cmp(data, tree->data));
}

struct btree *
btree_insert_ex(struct btree *tree, void *data, btree_cmp_ex_fn cmp, void *cmp_arg)
{
	INSERT_BODY(tree, data, cmp, cmp(data, tree->data, cmp_arg));
}

#if 0
struct btree *
btree_insert(struct btree *tree, void *data, btree_cmp_fn cmp)
{
	while(1) {
		int cmp_res = cmp(data, tree->data);
		int dir = cmp_res > 0; /* 0 for left, 1 for right. */
		if (tree->child[dir] == NULL) {
			struct btree *res = btree_create(data);
			if (res == NULL) return NULL;
			tree->child[dir] = res;
			res->parent = tree;
			return res;
		} else {
			tree = tree->child[dir];
		}
	}
}

struct btree *
btree_insert_ex(struct btree *tree, void *data, btree_cmp_ex_fn cmp, void *arg)
{
	while(1) {
		int cmp_res = cmp(data, tree->data, arg);
		int dir = cmp_res > 0; /* 0 for left, 1 for right. */
		if (tree->child[dir] == NULL) {
			struct btree *res = btree_create(data);
			if (res == NULL) return NULL;
			tree->child[dir] = res;
			res->parent = tree;
			return res;
		} else {
			tree = tree->child[dir];
		}
	}
}

/* ---------- deletion ---------- */

/* A helper macro to avoid code duplication. The only thing that's different 
 * between 'btree_delete' and 'btree_delete_ex' is the first line, the rest is
 * the same. */
#define DELETE_BODY(tree, to_delete, tree_after, deleted) \
{ \
	if (to_delete == NULL) { \
		*tree_after = tree; \
		return 0; \
	} \
 \
	/* The easiest case - the node is a leaf. */ \
	if (btree_is_a_leaf(to_delete)) { \
		btree_unlink(to_delete); \
		*deleted = to_delete->data; \
		if (to_delete == tree)  \
			*tree_after = NULL; \
		else \
			*tree_after = tree; \
		free(to_delete); \
		return 1; \
	} \
 \
	/* Harder cases - the node either has a predecessor or a successor. */ \
	struct btree *predecessor = btree_predecessor(to_delete); \
	if (predecessor != NULL) { \
		btree_unlink(predecessor); \
		void *pred_data = predecessor->data; \
		free(predecessor); \
 \
		void *old_data = to_delete->data; \
		to_delete->data = pred_data; \
		*tree_after = tree; \
		*deleted = old_data; \
		return 1; \
	} \
 \
	/* If we got here, the node has a successor: it is not a leaf and has \
	 * no left subnodes, therefore it has a right subnode. */ \
	struct btree *successor = btree_successor(to_delete); \
	btree_unlink(successor); \
	void *succ_data = successor->data; \
	free(successor); \
 \
	void *old_data = to_delete->data; \
	to_delete->data = succ_data; \
	*tree_after = tree; \
	*deleted = old_data; \
	return 1; \
}

extern int
btree_delete(struct btree *tree, void *data, btree_cmp_fn cmp, 
		struct btree **tree_after, void **deleted)
{
	struct btree *to_delete = btree_find_node(tree, data, cmp);
	DELETE_BODY(tree, to_delete, tree_after, deleted);
}

extern int
btree_delete_ex(struct btree *tree, void *data, btree_cmp_ex_fn cmp, void *arg,
		struct btree **tree_after, void **deleted)
{
	struct btree *to_delete = btree_find_node_ex(tree, data, cmp, arg);
	DELETE_BODY(tree, to_delete, tree_after, deleted);
}

#endif

/* ---------- other ---------- */

void
btree_unlink(struct btree *tree)
{
	struct btree *left = btree_after_outermost(tree, 0);
	struct btree *right = btree_after_outermost(tree, 1);

	if (left != NULL && left->link[1] == tree) {
		left->thread[1] = right != NULL;
		left->link[1] = right;
	}
	if (right != NULL && right->link[0] == tree) {
		right->thread[0] = left != NULL;
		right->link[0] = left;
	}
}