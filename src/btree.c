
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
		while (!cur->thread[0] && cur->link[0] != NULL) 
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
		while (!cur->thread[1] && cur->link[1] != NULL) 
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

	btt_init(res, tree, type);
	return res;
}

void
btt_init(struct btt *btt, struct btree *tree, enum btt_type type)
{
	btt->tree = tree;
	btt->type = type;
	switch (type) {
		case BTT_INORDER:
			btt->cur = btree_outermost(tree, 0);
			break;
		case BTT_INORDER_REV:
			btt->cur = btree_outermost(tree, 1);
			break;
	} /* switch type */
}

int
btt_done(struct btt *btt)
{
	return btt->cur == NULL;
}

void *
btt_this(struct btt *btt)
{
	return btt->cur->data;
}

struct btree *
btt_this_node(struct btt *btt)
{
	return btt->cur;
}

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

void *
btree_data(struct btree *tree)
{
	return tree->data;
}

struct btree *
btree_left(struct btree *tree)
{
	return tree->link[0];
}

struct btree *
btree_right(struct btree *tree)
{
	return tree->link[1];
}

int
btree_thread(struct btree *tree, int dir)
{
	return tree->thread[!!dir];
}

struct btree *
btree_link(struct btree *tree, int dir)
{
	return tree->link[!!dir];
}

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
	struct btree *left = btree_outermost(tree, 0);
	struct btree *right = btree_outermost(tree, 1);

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

	left->thread[0] = 0;
	left->link[0] = NULL;
	right->thread[1] = 0;
	right->link[1] = NULL;
}
