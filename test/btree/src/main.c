
#include <check.h>
#include <stdlib.h>

#include "btree.h"

#include "test.h"

START_TEST(test_insert_and_find)
{
	struct btree *t = btree_create(mkint(0));
	ck_assert_msg(t != NULL, "Failed to create a tree");

	void *pos1 = mkint(-1);
	void *pos2 = mkint(2);
	void *pos3 = mkint(-10);
	void *pos4 = mkint(-5);

	int neg1 = 20;
	int neg2 = 1;
	int neg3 = -100;
	int neg4 = -7;

	struct btree *n1 = btree_insert(t, pos1, &cmp_ints);
	struct btree *n2 = btree_insert(t, pos2, &cmp_ints);
	struct btree *n3 = btree_insert(t, pos3, &cmp_ints);
	struct btree *n4 = btree_insert(t, pos4, &cmp_ints);

	ck_assert_msg(n1 != NULL, "Failed to insert -1 into the tree");
	ck_assert_msg(n2 != NULL, "Failed to insert 2 into the tree");
	ck_assert_msg(n3 != NULL, "Failed to insert -10 into the tree");
	ck_assert_msg(n4 != NULL, "Failed to insert -5 into the tree");

	ck_assert_msg(btree_find(t, pos1, &cmp_ints, NULL), "Failed to find -1 in the tree");
	ck_assert_msg(btree_find(t, pos2, &cmp_ints, NULL), "Failed to find 2 in the tree");
	ck_assert_msg(btree_find(t, pos3, &cmp_ints, NULL), "Failed to find -10 in the tree");
	ck_assert_msg(btree_find(t, pos4, &cmp_ints, NULL), "Failed to find -5 in the tree");

	ck_assert_msg(!btree_find(t, &neg1, &cmp_ints, NULL), "Found 20 in the tree");
	ck_assert_msg(!btree_find(t, &neg2, &cmp_ints, NULL), "Found 1 in the tree");
	ck_assert_msg(!btree_find(t, &neg3, &cmp_ints, NULL), "Found -100 in the tree");
	ck_assert_msg(!btree_find(t, &neg4, &cmp_ints, NULL), "Found -7 in the tree");

	btree_destroy_ex(t, &free);
}
END_TEST;

START_TEST(test_insert_and_find_ex)
{
	struct btree *t = btree_create(mkint(0));
	ck_assert_msg(t != NULL, "Failed to create a tree");

	void *pos1 = mkint(-1);
	void *pos2 = mkint(2);
	void *pos3 = mkint(-10);
	void *pos4 = mkint(-5);

	int neg1 = 20;
	int neg2 = 1;
	int neg3 = -100;
	int neg4 = -7;

	struct btree *n1 = btree_insert_ex(t, pos1, &cmp_ints_ex, NULL);
	struct btree *n2 = btree_insert_ex(t, pos2, &cmp_ints_ex, NULL);
	struct btree *n3 = btree_insert_ex(t, pos3, &cmp_ints_ex, NULL);
	struct btree *n4 = btree_insert_ex(t, pos4, &cmp_ints_ex, NULL);

	ck_assert_msg(n1 != NULL, "Failed to insert -1 into the tree");
	ck_assert_msg(n2 != NULL, "Failed to insert 2 into the tree");
	ck_assert_msg(n3 != NULL, "Failed to insert -10 into the tree");
	ck_assert_msg(n4 != NULL, "Failed to insert -5 into the tree");

	ck_assert_msg(btree_find_ex(t, pos1, &cmp_ints_ex, NULL, NULL), 
			"Failed to find -1 in the tree");
	ck_assert_msg(btree_find_ex(t, pos2, &cmp_ints_ex, NULL, NULL), 
			"Failed to find 2 in the tree");
	ck_assert_msg(btree_find_ex(t, pos3, &cmp_ints_ex, NULL, NULL),
		       	"Failed to find -10 in the tree");
	ck_assert_msg(btree_find_ex(t, pos4, &cmp_ints_ex, NULL, NULL), 
			"Failed to find -5 in the tree");

	ck_assert_msg(!btree_find_ex(t, &neg1, &cmp_ints_ex, NULL, NULL), "Found 20 in the tree");
	ck_assert_msg(!btree_find_ex(t, &neg2, &cmp_ints_ex, NULL, NULL), "Found 1 in the tree");
	ck_assert_msg(!btree_find_ex(t, &neg3, &cmp_ints_ex, NULL, NULL), "Found -100 in the tree");
	ck_assert_msg(!btree_find_ex(t, &neg4, &cmp_ints_ex, NULL, NULL), "Found -7 in the tree");

	btree_destroy_ex(t, &free);
}
END_TEST;

START_TEST(test_varsearch)
{
	struct btree *t = btree_create(mkint(0));
	ck_assert_msg(t != NULL, "Failed to create a tree");

	int i1 = -10;
	int i2 = 10;
	int i3 = 5;
	int i4 = -5;
	int i5 = 20;

	void *d1 = mkint(i1);
	void *d2 = mkint(i2);
	void *d3 = mkint(i3);
	void *d4 = mkint(i4);
	void *d5 = mkint(i5);

	struct btree *n1 = btree_insert(t, d1, &cmp_ints);
	struct btree *n2 = btree_insert(t, d2, &cmp_ints);
	struct btree *n3 = btree_insert(t, d3, &cmp_ints);
	struct btree *n4 = btree_insert(t, d4, &cmp_ints);
	struct btree *n5 = btree_insert(t, d5, &cmp_ints);

	ck_assert_msg(n1 != NULL, "Failed to insert -10 into the tree");
	ck_assert_msg(n2 != NULL, "Failed to insert 10 into the tree");
	ck_assert_msg(n3 != NULL, "Failed to insert 5 into the tree");
	ck_assert_msg(n4 != NULL, "Failed to insert -5 into the tree");
	ck_assert_msg(n5 != NULL, "Failed to insert 20 into the tree");

	ck_assert_msg(btree_parent(t) == NULL, "Non-null parent for the root");
	ck_assert_msg(btree_parent(n1) == t, "Wrong parent for the -10 node");
	ck_assert_msg(btree_parent(n2) == t, "Wrong parent for the 10 node");
	ck_assert_msg(btree_parent(n3) == n2, "Wrong parent for the 5 node");
	ck_assert_msg(btree_parent(n4) == n1, "Wrong parent for the -5 node");
	ck_assert_msg(btree_parent(n5) == n2, "Wrong parent for the 20 node");

	ck_assert_msg(btree_outermost(t, 0) == n1, "Wrong leftmost node for the entire tree");
	ck_assert_msg(btree_outermost(t, 1) == n5, "Wrong rightmost node for the entire tree");
	ck_assert_msg(btree_outermost(n2, 0) == n3, "Wrong leftmost node for the 10 subtree");
	ck_assert_msg(btree_outermost(n2, 1) == n5, "Wrong rightmost node for the 10 subtree");

	btree_destroy_ex(t, &free);
}
END_TEST;

START_TEST(test_inorder)
{
	void *root = mkint(0);
	struct btree *t = btree_create(root);
	ck_assert_msg(t != NULL, "Failed to create a tree");

	void *i1 = mkint(-10);
	void *i2 = mkint(-5);
	void *i3 = mkint(5);
	void *i4 = mkint(10);
	void *i5 = mkint(20);

	ck_assert_msg(btree_insert(t, i1, &cmp_ints) != NULL, "Failed to insert -10 "
			"into the tree");
	ck_assert_msg(btree_insert(t, i2, &cmp_ints) != NULL, "Failed to insert -5 "
			"into the tree");
	ck_assert_msg(btree_insert(t, i4, &cmp_ints) != NULL, "Failed to insert 10 "
			"into the tree");
	ck_assert_msg(btree_insert(t, i3, &cmp_ints) != NULL, "Failed to insert 5 "
			"into the tree");
	ck_assert_msg(btree_insert(t, i5, &cmp_ints) != NULL, "Failed to insert 20 "
			"into the tree");

	struct btt trav;
	btt_init(&trav, t, BTT_INORDER);

	ck_assert_msg(inttree_eq(btt_this_node(&trav), i1), "First element is not -10");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), i2), "Second element is not -5");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), root), "Third element is not 0");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), i3), "Fourth element is not 5");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), i4), "Fifth element is not 10");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), i5), "Sixth element is not 20");

	btt_rewind(&trav);

	ck_assert_msg(inttree_eq(btt_this_node(&trav), i1), "After rewind, the "
			"first element is not -10");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), i2), "After rewind, the second "
		       "element is not -5");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), root), "After rewind, the third "
		       "element is not 0");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), i3), "After rewind, the fourth "
		       "element is not 5");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), i4), "After rewind, the fifth "
		       "element is not 10");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), i5), "After rewind, the sixth "
		       "element is not 20");

	btree_destroy_ex(t, &free);
}
END_TEST;

START_TEST(test_inorder_rev)
{
	void *root = mkint(0);
	struct btree *t = btree_create(root);
	ck_assert_msg(t != NULL, "Failed to create a tree");

	void *i1 = mkint(-10);
	void *i2 = mkint(-5);
	void *i3 = mkint(5);
	void *i4 = mkint(10);
	void *i5 = mkint(20);

	ck_assert_msg(btree_insert(t, i1, &cmp_ints) != NULL, "Failed to insert -10 "
			"into the tree");
	ck_assert_msg(btree_insert(t, i2, &cmp_ints) != NULL, "Failed to insert -5 "
			"into the tree");
	ck_assert_msg(btree_insert(t, i4, &cmp_ints) != NULL, "Failed to insert 10 "
			"into the tree");
	ck_assert_msg(btree_insert(t, i3, &cmp_ints) != NULL, "Failed to insert 5 "
			"into the tree");
	ck_assert_msg(btree_insert(t, i5, &cmp_ints) != NULL, "Failed to insert 20 "
			"into the tree");

	struct btt trav;
	btt_init(&trav, t, BTT_INORDER_REV);

	ck_assert_msg(inttree_eq(btt_this_node(&trav), i5), "The first element is not 20");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), i4), "The second element is not 10");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), i3), "The third element is not 5");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), root), "Thr fourth element is not 0");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), i2), "The fifth element is not -5");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), i1), "The last element is not -10");

	btt_rewind(&trav);
	ck_assert_msg(inttree_eq(btt_this_node(&trav), i5), "After rewind "
			"the first element is not 20");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), i4), "After rewind "
			"the second element is not 10");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), i3), "After rewind "
			"the third element is not 5");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), root), "After rewind "
			"thr fourth element is not 0");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), i2), "After rewind "
			"the fifth element is not -5");
	ck_assert_msg(inttree_eq(btt_next_node(&trav), i1), "After rewind "
			"the last element is not -10");

	btree_destroy_ex(t, &free);
}
END_TEST;

Suite *
btree_suite(void)
{
	Suite *res = suite_create("BTree");

	/* Core tests. */
	TCase *core_tests = tcase_create("Core");
	tcase_add_test(core_tests, test_insert_and_find);
	tcase_add_test(core_tests, test_insert_and_find_ex);
	tcase_add_test(core_tests, test_varsearch);
	tcase_add_test(core_tests, test_inorder);
	tcase_add_test(core_tests, test_inorder_rev);

	suite_add_tcase(res, core_tests);

	return res;
}

int
main(int argc, char **argv)
{
	int failed = 0;
	Suite *suite = btree_suite();
	SRunner *runner = srunner_create(suite);

	srunner_run_all(runner, CK_NORMAL);
	failed = srunner_ntests_failed(runner);
	srunner_free(runner);

	return (failed == 0) ? 0 : 1;
}

/* ---------- helper functions ---------- */

int
cmp_ints(void *a, void *b)
{
	int left = *(int *)a;
	int right = *(int *)b;

	return left < right ? -1 : left == right ? 0 : 1;
}

int 
cmp_ints_ex(void *a, void *b, void *extra_arg)
{
	return cmp_ints(a, b);
}

int *
mkint(int i)
{
	int *res = malloc(sizeof(int));
	*res = i;
	return res;
}

int
int_eq(void *i1, int i2)
{
	return *(int *)i1 == i2;
}

int
inttree_eq(struct btree *t, void *i)
{
	int *left = btree_data(t);
	int *right = i;
	return *left == *right;
}
