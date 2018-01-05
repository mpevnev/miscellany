
#include <check.h>
#include <stdlib.h>

#include "array.h"

#include "main.h"

START_TEST(test_creation)
{
	struct array *arr = arr_create(4, sizeof(int));

	int i1 = 0;
	int i2 = 10;
	int i3 = 2;
	int i4 = 3;

	ck_assert_msg(arr_append(arr, &i1), "Failed to append 0 to the array");
	ck_assert_msg(arr_append(arr, &i2), "Failed to append 10 to the array");
	ck_assert_msg(arr_append(arr, &i3), "Failed to append 2 to the array");
	ck_assert_msg(arr_append(arr, &i4), "Failed to append 3 to the array");

	arr_destroy(arr);
}
END_TEST;

START_TEST(test_ex_destruction)
{
	struct array *arr = arr_create(4, sizeof(int *));

	int *i1 = mk_int(1);
	int *i2 = mk_int(2);
	int *i3 = mk_int(3);
	int *i4 = mk_int(4);

	ck_assert_msg(arr_append(arr, &i1), "Failed to append first value to the array");
	ck_assert_msg(arr_append(arr, &i2), "Failed to append first value to the array");
	ck_assert_msg(arr_append(arr, &i3), "Failed to append first value to the array");
	ck_assert_msg(arr_append(arr, &i4), "Failed to append first value to the array");

	arr_destroy_ex(arr, &free_int);
}
END_TEST;

START_TEST(test_appending)
{
	int a[2] = {1, 2};
	int b[2] = {3, 4};

	struct array *arr_a = arr_from_data(2, sizeof(int), a);
	struct array *arr_b = arr_from_data(2, sizeof(int), b);

	ck_assert_msg(arr_a != NULL, "Failed to create the first array");
	ck_assert_msg(arr_b != NULL, "Failed to create the second array");

	int i = 5;
	int append_one = arr_append(arr_b, &i);
	ck_assert_msg(append_one, "Failed to append 5 to the second array");

	int must_be[5] = {1, 2, 3, 4, 5};
	int append_arr = arr_append_a(arr_a, arr_b);
	ck_assert_msg(append_arr, "Failed to append arrays");
	ck_assert_msg(int_arr_eq(arr_a, must_be), "After appending, the array "
			"is not {1, 2, 3, 4, 5}");

	arr_destroy(arr_a);
	arr_destroy(arr_b);
}
END_TEST;

START_TEST(test_prepending)
{
	int a[2] = {1, 2};
	int b[2] = {3, 4};

	struct array *arr_a = arr_from_data(2, sizeof(int), a);
	struct array *arr_b = arr_from_data(2, sizeof(int), b);

	ck_assert_msg(arr_a != NULL, "Failed to create the first array");
	ck_assert_msg(arr_b != NULL, "Failed to create the second array");

	int i = 5;
	int prepend_one = arr_prepend(arr_b, &i);
	ck_assert_msg(prepend_one, "Failed to prepend 5 to the second array");

	int must_be[5] = {5, 3, 4, 1, 2};
	int append_arr = arr_prepend_a(arr_a, arr_b);
	ck_assert_msg(append_arr, "Failed to prepend arrays");
	ck_assert_msg(int_arr_eq(arr_a, must_be), "After prepending, the array "
			"is not {5, 3, 4, 1, 2}");

	arr_destroy(arr_a);
	arr_destroy(arr_b);
}
END_TEST;

Suite *
array_suite(void)
{
	Suite *res = suite_create("Array");

	/* Core tests. */
	TCase *core_tests = tcase_create("Core");
	tcase_add_test(core_tests, test_creation);
	tcase_add_test(core_tests, test_ex_destruction);
	tcase_add_test(core_tests, test_appending);
	tcase_add_test(core_tests, test_prepending);

	suite_add_tcase(res, core_tests);

	return res;
}

int
main(int argc, char **argv)
{
	int failed = 0;
	Suite *suite = array_suite();
	SRunner *runner = srunner_create(suite);

	srunner_run_all(runner, CK_NORMAL);
	failed = srunner_ntests_failed(runner);
	srunner_free(runner);

	return (failed == 0) ? 0 : 1;
}

/* ---------- helper functions ---------- */

#include <stdio.h>

void
free_int(void *ptr)
{
	int **i = ptr;
	free(*i);
}

int *
mk_int(int i)
{
	int *res = malloc(sizeof(int));
	*res = i;
	return res;
}

int
int_arr_eq(struct array *array, int *ints)
{
	for (size_t i = 0; i < arr_size(array); i++) {
		int *val = arr_ix(array, i);
		if (*val != ints[i]) return 0;
	}
	return 1;
}
