
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

	int *i1 = malloc(sizeof(int));
	int *i2 = malloc(sizeof(int));
	int *i3 = malloc(sizeof(int));
	int *i4 = malloc(sizeof(int));

	ck_assert_msg(arr_append(arr, i1), "Failed to append first value to the array");
	ck_assert_msg(arr_append(arr, i2), "Failed to append first value to the array");
	ck_assert_msg(arr_append(arr, i3), "Failed to append first value to the array");
	ck_assert_msg(arr_append(arr, i4), "Failed to append first value to the array");

	arr_destroy_ex(arr, &free);
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
