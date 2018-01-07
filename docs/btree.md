
# Binary tree module `<misc/btree.h>`

This module provides binary search trees. Functions to create, destroy, insert
into, look up, remove from and traverse trees are provided.

*Note*: whereever direction is required, 0 means left and 1 means right.

## Data types

The data type for the binary trees is `struct btree`. The trees are fully 
threaded.

The data type for a binary tree traversal is `struct btt`. Traversals can be of
several types (a value of type `enum btt_type` is used to distinguish them),
currently the following are provided:
- `BTT_INORDER`, for ascending inorder traversal;
- `BTT_INORDER_REV`, for descending inorder traversal.

There are also two typedefs for functions used to compare binary tree elements:
- `typedef int (*btree_cmp_fn)(void *left, void *right)`
- `typedef int (*btree_cmp_ex_fn)(void *left, void *right, void *extra)`
Both should return a negative value if `left < right`, 0 if `left == right` and
a positive value if `left > right`.

## Functions - creation

### `btree_create`

```
struct btree *
btree_create(void *data)
```

Create and return a new binary tree node with data `data`.

Return NULL if an OOM condition has occured.

## Functions - destruction

Any of these can be used on a subtree, the parent tree will be updated to
exclude the destroyed part.

### `btree_destroy`

```
void
btree_destroy(struct btree *btree)
```

Free memory used by `btree` itself, its data is not affected.

### `btree_destroy_ex`

```
void
btree_destroy_ex(struct btree *btree, void (*destroyer)(void *data))
```

Run every element of `btree` through `destroyer`, then free the memory of the
tree itself.

### `btree_destroy_exx`

```
void
btree_destroy_exx(struct btree *btree, void (*destroyer)(void *data, void *arg), void *arg)
```

Run `destroyer` on every element of `btree` as the first argument and with `arg`
as the second, then free the memory of the tree itself.

## Functions - information retrieval

### `btree_data`

```
void *
btree_data(struct btree *btree)
```

Return the data contained in `btree`.

### `btree_left`

```
void *
btree_left(struct btree *btree)
```

Return the left link of `btree`. Due to threading, this is not guaranteed to be
a child node.

### `btree_right`

```
void *
btree_right(struct btree *btree)
```

Return the right link of `btree`. Due to threading, this is not guaranteed to 
be a child node.

### `btree_thread`

```
int
btree_thread(struct btree *tree, int dir)
```

Return 1 if `tree` is threaded in the direction given by `dir`, 0 otherwise.

### `btree_link`

```
struct btree *
btree_link(struct btree *tree, int dir)
```

Return either the left link of `tree` (if `dir` is 0), or the right link (if 
`dir` is non-zero). Due to threading, it's not guaranteed to be a child node.

### `btree_has_children`

```
int
btree_has_children(struct btree *tree)
```

Return 1 if `tree` has children, 0 otherwise.

### `btree_has_child`

```
int
btree_has_child(struct btree *tree, int dir)
```

Return 1 if `tree` has a child in direction `dir`, 0 otherwise.

### `btree_num_children`

```
int
btree_num_children(struct btree *tree)
```

Return the number of children of `tree`.

### `btree_is_a_leaf`

```
int
btree_is_a_leaf(struct btree *tree)
```

Return 1 if `tree` is a leaf node, 0 otherwise.

### `btree_link_dir`

```
int
btree_link_dir(struct btree *tree, struct btree *link)
```

Return 0 if `link` is the left link of `tree`, 1 if `link` is the right link of
`tree`, and -1 if it's not a link of `tree` at all.

### `btree_size`

```
size_t
btree_size(struct btree *tree)
```

Return the number of nodes in `tree`.

### `btree_parent`

```
struct btree *
btree_parent(struct btree *tree)
```

Return the parent node of `tree`, or NULL if `tree` is the root.

### `btree_successor`

```
struct btree *
btree_successor(struct btree *tree)
```

Return the inorder successor of the subtree given by `tree`.

### `btree_predecessor`

```
struct btree *
btree_predecessor(struct btree *tree)
```

Return the inorder predecessor of the subtree given by `tree`.

### `btree_outermost`

```
struct btree *
btree_outermost(struct btree *tree, int dir)
```

Return either left or right (depending on `dir`) outermost element of the 
subtree given by `tree`.

### `btree_after_outermost`

```
struct btree *
btree_after_outermost(struct btree *tree, int dir)
```

Return a node after the outermost node in the subtree given by `tree` in the
direction given by `dir`.

## Functions - insertion

### `btree_insert`

```
struct btree *
btree_insert(struct btree *tree, void *data, btree_cmp_fn cmp)
```

Insert `data` into `tree` using `cmp` to compare elements of `tree` with `data`.
Return the freshly created node.

Return NULL if an OOM condition has occured.

### `btree_insert_ex`

```
struct btree *
btree_insert_ex(struct btree *tree, void *data, btree_cmp_ex_fn cmp, void *arg)
```

Insert `data` into `tree` using `cmp` to compare elements of `tree` with `data`
(and with `arg` as the third argument). Return the freshly created node.

Return NULL if an OOM condition has occured.

## Functions - searching

### `btree_find`

```
int
btree_find(struct btree *tree, void *data, btree_cmp_fn cmp, void **res)
```

Try to find `data` in `tree` using comparison function `cmp`. If `data` is in
`tree`, return 1 and fill `res` (unless it's NULL) with the data from the `tree`
(not from `data`! depending on what `cmp` does, they might be different).
Otherwise return 0 and do nothing with `res`.

### `btree_find_ex`

```
int
btree_find_ex(struct btree *tree, void *data, btree_cmp_ex_fn cmp, void *arg, void **res)
```

Try to find `data` in `tree` using comparison function `cmp` (with `arg` as the
third argument). If `data` is in `tree`, return 1 and fill `res` (unless it's
NULL) with the data from the `tree` (not from `data`! depending on what `cmp`
does, they might be different). Otherwise return 0 and do nothing with `res`.

### `btree_find_node`

```
struct btree *
btree_find_node(struct btree *tree, void *data, btree_cmp_fn cmp)
```

Return the node from `tree` that contains `data`, using `cmp` for comparing
elements of `tree` with `data`. If no such node is found, return NULL.

### `btree_find_node_ex`

```
struct btree *
btree_find_node_ex(struct btree *tree, void *data, btree_cmp_ex_fn cmp, void *arg)
```

Return the node from `tree` that contains `data`, using `cmp` for comparing
elements of `tree` with `data` (with `arg` as the third argument). If no such
node is found, return NULL.

## Functions - deletion

If you wish to delete a subtree, use `btree_destroy[_ex[x]]` on it. The 
following functions are for single node deletion only.

### `btree_delete`

```
int
btree_delete(struct btree *tree, void *data, btree_cmp_fn cmp,
	struct btree **tree_after, void **deleted_data)
```

Try to delete `data` from `tree`, using `cmp` to compare elements of `tree` with
`data`. Return 1 if `data` was found in `tree`, 0 otherwise.

Fill `tree_after` with the root of the tree after deletion (might be NULL if
the last node was removed). Pass NULL to avoid filling. 

Fill `deleted_data` will the data of the deleted node. Pass NULL to avoid 
filling.

### `btree_delete_ex`

```
int
btree_delete_ex(struct btree *tree, void *data, btree_cmp_ex_fn cmp, void *arg,
	struct btree **tree_after, void **deleted_data)
```

Try to delete `data` from `tree`, using `cmp` (with `arg` as the third
argument) to compare elements of `tree` with `data`. Return 1 if `data` was
found in `tree`, 0 otherwise.

Fill `tree_after` with the root of the tree after deletion (might be NULL if
the last node was removed). Pass NULL to avoid filling. 

Fill `deleted_data` will the data of the deleted node. Pass NULL to avoid 
filling.

## Functions - traversal-related

### `btt_create`

```
struct btt *
btt_create(struct btree *tree, enum btt_type type)
```

Create and return a traversal object of type `type`. For a list of supported
traversal types see the module overview at the top of this document.

Return NULL if an OOM condition has occured.

### `btt_init`

```
int
btt_init(struct btt *btt, struct btree *tree, enum btt_type type)
```

Initialize a traversal object of type `type`. For a list of supported traversal
types see the module overview at the top of this document.

Return 1 on success, 0 on failure.

The following traversal types always succeed, as there's no memory allocations
involved:
- `BTT_INORDER`
- `BTT_INORDER_REV`

### `btt_this_node`

```
struct btree *
btt_this_node(struct btt *btt)
```

Return the current node of a traversal, or NULL if the traversal has finished.

### `btt_this`

```
void *
btt_this(struct btt *btt)
```

Return the data of the current node of a traversal. 

**Note:** this function is unsafe and should only be used on traversals that
have not finished.

### `bbt_next_node`

```
struct btree *
btt_next_node(struct btt *btt)
```

Advance the traversal `btt` and return the new node it points to. Return NULL
if after the advancement the traversal is finished.

### `btt_done`

```
int
btt_done(struct btt *btt)
```

Return 1 if the traversal is finished, 0 otherwise.

### `btt_rewind`

```
void
btt_rewind(struct btt *btt)
```

Rewind a traversal to point to the first node again.

### `btt_destroy`

```
void 
btt_destroy(struct btt *btt)
```

Free `btt` and the state it holds.

### `btt_fin`

```
void
btt_fin(struct btt *btt)
```

Free the state `btt` holds without freeing `btt` itself.

## Functions - other

### `btree_unlink`

```
void
btree_unlink(struct btree *tree)
```

Unlink `tree` from its parent tree. The parent tree will be updated to exclude
`tree`, and `tree` will be updated not to refer in any way to the parent tree.

