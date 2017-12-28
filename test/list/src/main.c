
#include <check.h>
#include <stdio.h>
#include <stdlib.h>

#include "list.h"

#include "main.h"

START_TEST(test_sorting_1)
{
	struct list *list = list_create();
	ck_assert_msg(list != NULL, "Failed to create a list");

	size_t size = 6;
	int to_be_sorted[] = {1, 9, 8, 7, 6, 5};
	int expected[] = {1, 5, 6, 7, 8, 9};

	for (size_t i = 0; i < size; i++) {
		list_push_back(list, to_be_sorted + i);
	}

	struct list *sorted = list_sort(list, &cmp_ints, 0);  
	void **sorted_array = list_to_array(sorted, NULL);

	ck_assert_msg(cmp_int_array(size, expected, sorted_array) == 0, 
			"Sorting returned unexpected result");

	free(sorted_array);
	list_destroy(list);
	list_destroy(sorted);
}
END_TEST;

START_TEST(test_sorting_2)
{
	struct list *list = list_create();
	ck_assert_msg(list != NULL, "Failed to create a list");

	size_t size = 6;
	int to_be_sorted[] = {6, 5, 4, 3, 2, 1};
	int expected[] = {1, 2, 3, 4, 5, 6};

	for (size_t i = 0; i < size; i++) {
		list_push_back(list, to_be_sorted + i);
	}

	struct list *sorted = list_sort(list, &cmp_ints, 0);  
	void **sorted_array = list_to_array(sorted, NULL);

	ck_assert_msg(cmp_int_array(size, expected, sorted_array) == 0, 
			"Sorting returned unexpected result");

	free(sorted_array);
	list_destroy(list);
	list_destroy(sorted);
}
END_TEST;

START_TEST(test_sorting_3)
{
	struct list *list = list_create();
	ck_assert_msg(list != NULL, "Failed to create a list");

	size_t size = 6;
	int to_be_sorted[] = {1, 2, 3, 1, 2, 3};
	int expected[] = {1, 1, 2, 2, 3, 3};

	for (size_t i = 0; i < size; i++) {
		list_push_back(list, to_be_sorted + i);
	}

	struct list *sorted = list_sort(list, &cmp_ints, 0);  
	void **sorted_array = list_to_array(sorted, NULL);

	ck_assert_msg(cmp_int_array(size, expected, sorted_array) == 0, 
			"Sorting returned unexpected result");

	free(sorted_array);
	list_destroy(list);
	list_destroy(sorted);
}
END_TEST;

Suite *
list_suite(void)
{
	Suite *res = suite_create("List");

	/* Core tests. */
	TCase *core_tests = tcase_create("Core");
	tcase_add_test(core_tests, test_sorting_1);
	tcase_add_test(core_tests, test_sorting_2);
	tcase_add_test(core_tests, test_sorting_3);

	suite_add_tcase(res, core_tests);

	return res;
}

int
main(int argc, char **argv)
{
	int failed = 0;
	Suite *suite = list_suite();
	SRunner *runner = srunner_create(suite);

	srunner_run_all(runner, CK_NORMAL);
	failed = srunner_ntests_failed(runner);
	srunner_free(runner);

	return (failed == 0) ? 0 : 1;
}

/* ---------- helper functions ---------- */

int
cmp_ints(void *left, void *right)
{
	int *a_ptr = left;
	int *b_ptr = right;
	int a = *a_ptr;
	int b = *b_ptr;
	if (a < b) return -1;
	if (a == b) return 0;
	return 1;
}

int
cmp_int_array(size_t size, int *ints, void **other)
{
	for (size_t i = 0; i < size; i++) {
		int left = ints[i];
		int *right = other[i];
		if (left < *right) return -1;
		if (left > *right) return 1;
	}
	return 0;
}
