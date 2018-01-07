
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

#define DESTROY_BODY(tree, free_data) \
{ \
	if (tree == NULL) return; \
	btree_unlink(tree); \
 \
	tree = btree_outermost(tree, 0); \
	while (tree != NULL) { \
		struct btree *link = tree->link[1]; \
		int thread = tree->thread[1]; \
		free_data; \
		free(tree); \
		tree = link; \
		if (!thread && link != NULL) { \
			while (!tree->thread[0] && tree->link[0] != NULL) \
				tree = tree->link[0]; \
		} \
	} \
} \

void
btree_destroy(struct btree *tree)
{
	DESTROY_BODY(tree, (void) 0);
}

void
btree_destroy_ex(struct btree *tree, void (*destroyer)(void *data))
{
	DESTROY_BODY(tree, destroyer(tree->data));
}

void
btree_destroy_exx(struct btree *tree, void (*destroyer)(void *data, void *arg), void *arg)
{
	DESTROY_BODY(tree, destroyer(tree->data, arg));
}

/* ---------- traversal ---------- */

/* Helper functions. */

struct btree *
btt_next_inorder(struct btt *btt)
{
	struct btree *cur = btt->cur;
	if (cur->thread[1]) {
		btt->cur = cur->link[1];
	} else {
		cur = cur->link[1];
		while (cur != NULL && !cur->thread[0] && cur->link[0] != NULL) 
			cur = cur->link[0];
		btt->cur = cur;
	}
	return btt->cur;
}

struct btree *
btt_next_inorder_rev(struct btt *btt)
{
	struct btree *cur = btt->cur;
	if (cur->thread[0]) {
		btt->cur = cur->link[0];
	} else {
		cur = cur->link[0];
		while (cur != NULL && !cur->thread[1] && cur->link[1] != NULL) 
			cur = cur->link[1];
		btt->cur = cur;
	}
	return btt->cur;
}

/* Exported functions. */

struct btt *
btt_create(struct btree *tree, enum btt_type type)
{
	struct btt *res = malloc(sizeof(struct btt));
	if (res == NULL) return NULL;

	if (!btt_init(res, tree, type)) {
		free(res);
		return NULL;
	}
	return res;
}

int
btt_init(struct btt *btt, struct btree *tree, enum btt_type type)
{
	btt->tree = tree;
	btt->type = type;
	switch (type) {
		case BTT_INORDER:
			btt->cur = btree_outermost(tree, 0);
			return 1;
		case BTT_INORDER_REV:
			btt->cur = btree_outermost(tree, 1);
			return 1;
	} /* switch type */
}

extern int
btt_done(struct btt *btt);

extern void *
btt_this(struct btt *btt);

extern struct btree *
btt_this_node(struct btt *btt);

void *
btt_next(struct btt *btt)
{
	return btt_next_node(btt)->data;
}

struct btree *
btt_next_node(struct btt *btt)
{
	switch (btt->type) {
		case BTT_INORDER: return btt_next_inorder(btt);
		case BTT_INORDER_REV: return btt_next_inorder_rev(btt);
	}
	return NULL; /* Just to silence a gcc warning. */
}

void
btt_rewind(struct btt *btt)
{
	btt_fin(btt);
	btt_init(btt, btt->tree, btt->type);
}

void
btt_destroy(struct btt *btt)
{
	btt_fin(btt);
	free(btt);
}

void
btt_fin(struct btt *btt)
{
	/* TODO: btt_fin: clear up any state. */
}

/* ---------- information retrieval ---------- */

extern void *
btree_data(struct btree *tree);

extern struct btree *
btree_left(struct btree *tree);

extern struct btree *
btree_right(struct btree *tree);

extern int
btree_thread(struct btree *tree, int dir);

extern struct btree *
btree_link(struct btree *tree, int dir);

int
btree_has_children(struct btree *tree)
{
	if (tree == NULL) return 0;
	for (int i = 0; i < 2; i++) {
		if (tree->thread[i]) continue;
		if (tree->link[i] == NULL) continue;
		return 1;
	}
	return 0;
}

int
btree_has_child(struct btree *tree, int dir)
{
	if (tree == NULL) return 0;
	dir = !!dir;
	return !tree->thread[dir] && tree->link[dir] != NULL;
}

int
btree_num_children(struct btree *tree)
{
	return btree_has_child(tree, 0) + btree_has_child(tree, 1);
}

int
btree_is_a_leaf(struct btree *tree)
{
	if (tree == NULL) return 1;
	return (tree->thread[0] || tree->link[0] == NULL)
		&& (tree->thread[1] || tree->link[1] == NULL);
}

int
btree_link_dir(struct btree *tree, struct btree *link)
{
	if (tree == NULL) return -1;
	for (int i = 0; i < 2; i++)
		if (tree->link[i] == link) return i;
	return -1;
}

size_t
btree_size(struct btree *tree)
{
	if (tree == NULL) return 0;

	size_t res = 0;
	struct btt trav;
	btt_init(&trav, tree, BTT_INORDER);
	for (; !btt_done(&trav); btt_next_node(&trav)) res++;
	btt_fin(&trav);

	return res;
}

struct btree *
btree_parent(struct btree *tree)
{
	if (tree == NULL) return NULL;
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
	if (tree == NULL) return NULL;
	dir = !!dir;
	while (!tree->thread[dir] && tree->link[dir] != NULL)
		tree = tree->link[dir];
	return tree;
}

struct btree *
btree_after_outermost(struct btree *tree, int dir)
{
	if (tree == NULL) return NULL;
	struct btree *outer = btree_outermost(tree, dir);
	return outer->link[dir];
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
	if (tree == NULL) return NULL;
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
	if (tree == NULL) return NULL;
	while(1) {
		int cmp_res = cmp(data, tree->data, cmp_arg);
		if (cmp_res == 0) return tree;

		int dir = cmp_res > 0;
		if (tree->thread[dir] || tree->link[dir] == NULL) return NULL;
		tree = tree->link[dir];
	}
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

/* ---------- deletion ---------- */

#define DELETE_BODY(tree, data, cmp, tree_after, deleted, find) \
{ \
	if (tree_after != NULL) *tree_after = tree; \
	if (deleted != NULL) *deleted = NULL; \
	if (tree == NULL) return 0; \
 \
	struct btree *to_delete = find; \
	if (to_delete == NULL) return 0; \
	void *old_data = to_delete->data; \
 \
	/* The simplest case - the node is a leaf. */ \
	if (btree_is_a_leaf(to_delete)) { \
		btree_unlink(to_delete); \
		if (tree_after != NULL) *tree_after = tree == to_delete ? NULL : tree; \
		if (deleted != NULL) *deleted = old_data; \
		free(to_delete); \
		return 1; \
	} \
 \
	/* A bit harder - the node has a single child. */ \
	if (btree_num_children(to_delete) == 1) { \
		int child_dir = btree_has_child(to_delete, 1); \
		/* Simply replace the node with its child. */ \
		struct btree *child = to_delete->link[child_dir]; \
		struct btree *parent = btree_parent(to_delete); \
		if (parent == NULL) { \
			child->thread[!child_dir] = 0; \
			child->link[!child_dir] = 0; \
			if (tree_after != NULL) *tree_after = child; \
			if (deleted != NULL) *deleted = old_data; \
			free(to_delete); \
			return 1; \
		} else { \
			int dir_to_here = btree_link_dir(parent, to_delete); \
			parent->thread[dir_to_here] = 0; \
			parent->link[dir_to_here] = child; \
			child->thread[!child_dir] = 1; \
			child->link[!child_dir] = parent; \
			free(to_delete); \
			if (tree_after != NULL) *tree_after = parent; \
			if (deleted != NULL) *deleted = old_data; \
			return 1; \
		} /* if parent is null */ \
	} /* if the node has one child */ \
 \
	/* The last case - both children are present. Pretty easy to do, \
	 * actually - find inorder predecessor and replace the node with it. */ \
	struct btree *left_child = to_delete->link[0]; \
	struct btree *predecessor = btree_outermost(left_child, 1); \
	btree_unlink(predecessor); \
	to_delete->data = predecessor->data; \
	if (tree_after != NULL) *tree_after = tree; \
	if (deleted != NULL) *deleted = old_data; \
	free(predecessor); \
	return 1; \
} \

extern int
btree_delete(struct btree *tree, void *data, btree_cmp_fn cmp, 
		struct btree **tree_after, void **deleted)
{
	DELETE_BODY(tree, data, cmp, tree_after, deleted, 
			btree_find_node(tree, data, cmp));
}

extern int
btree_delete_ex(struct btree *tree, void *data, btree_cmp_ex_fn cmp, void *arg,
		struct btree **tree_after, void **deleted)
{
	DELETE_BODY(tree, data, cmp, tree_after, deleted,
			btree_find_node_ex(tree, data, cmp, arg));
}

/* ---------- other ---------- */

void
btree_unlink(struct btree *tree)
{
	struct btree *left = btree_outermost(tree, 0);
	struct btree *right = btree_outermost(tree, 1);

	/* Make the outer tree forget about the inner. */
	struct btree *before_left = left->link[0];
	struct btree *after_right = right->link[1];
	if (before_left != NULL && (before_left->thread[1] || before_left->link[1] == tree)) {
		before_left->thread[1] = after_right != NULL;
		before_left->link[1] = after_right;
	}
	if (after_right != NULL && (after_right->thread[0] || after_right->link[0] == tree)) {
		after_right->thread[0] = before_left != NULL;
		after_right->link[0] = before_left;
	}

	/* Make the inner tree a standalone tree. */
	left->thread[0] = 0;
	left->link[0] = NULL;
	right->thread[1] = 0;
	right->link[1] = NULL;
}
