
#include "btree.h" /* DEBUG */
#include <stdlib.h>
#include "btree.h"

/* ---------- creation ---------- */

struct btree *
btree_create(void *data)
{
	struct btree *res = malloc(sizeof(struct btree));
	if (res == NULL) return NULL;

	res->data = data;
	res->parent = NULL;
	res->child[0] = NULL;
	res->child[1] = NULL;

	return res;
}

/* ---------- destruction ---------- */

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

/* ---------- searching ---------- */

int
btree_find(struct btree *tree, void *data, btree_cmp_fn cmp, void **res)
{
	struct btree *found = btree_find_node(tree, data, cmp);
	if (found == NULL) {
		return 0;
	} else {
		*res = found->data;
		return 1;
	}
}

int 
btree_find_ex(struct btree *tree, void *data, btree_cmp_ex_fn cmp, void *arg, void **res)
{
	struct btree *found = btree_find_node_ex(tree, data, cmp, arg);
	if (found == NULL) {
		return 0;
	} else {
		*res = found->data;
		return 1;
	}
}

struct btree *
btree_find_node(struct btree *tree, void *data, btree_cmp_fn cmp)
{
	while(1) {
		int cmp_res = cmp(data, tree->data);
		if (cmp_res == 0) {
			return tree;
		} else if (cmp_res < 0) {
			/* Data belongs to the left of this node. */
			if (tree->child[0] == NULL) 
				return NULL;
			tree = tree->child[0];
		} else {
			/* Data belongs to the right of this node. */
			if (tree->child[1] == NULL)
				return NULL;
			tree = tree->child[1];
		}
	}
}

struct btree *
btree_find_node_ex(struct btree *tree, void *data, btree_cmp_ex_fn cmp, void *arg)
{
	while(1) {
		int cmp_res = cmp(data, tree->data, arg);
		if (cmp_res == 0) {
			return tree;
		} else if (cmp_res < 0) {
			/* Data belongs to the left of this node. */
			if (tree->child[0] == NULL) 
				return NULL;
			tree = tree->child[0];
		} else {
			/* Data belongs to the right of this node. */
			if (tree->child[1] == NULL)
				return NULL;
			tree = tree->child[1];
		}
	}
}

struct btree *
btree_predecessor(struct btree *tree)
{
	if (tree->child[0] == NULL) return NULL;

	tree = tree->child[0];
	while (tree->child[1] != NULL)
		tree = tree->child[1];
	return tree;
}

struct btree *
btree_successor(struct btree *tree)
{
	if (tree->child[1] == NULL) return NULL;

	tree = tree->child[1];
	while (tree->child[0] != NULL)
		tree = tree->child[0];
	return tree;
}

/* ---------- insertion ---------- */

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

/* ---------- other ---------- */

void
btree_unlink(struct btree *tree)
{
	if (tree->parent == NULL) return;
	struct btree *parent = tree->parent;
	if (parent->child[0] == tree)
		parent->child[0] = NULL;
	else if (parent->child[1] == tree)
		parent->child[1] = NULL;
	tree->parent = NULL;
}
