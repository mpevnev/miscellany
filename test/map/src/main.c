
#include <check.h>
#include <string.h>

#include "map.h"

#include "main.h"

START_TEST(test_creation)
{
	struct map *map = map_create_fs(10, sizeof(int), 0);

	int i1 = 0;
	int i2 = 10;
	int i3 = 2;
	int i4 = 3;

	double f1 = 0;
	double f2 = 0.5;
	double f3 = 1.1;
	double f4 = 10.2;

	ck_assert_msg(map_insert(map, &i1, &f1, &int_eq) == MAPE_OK,
			"Failed to insert (0, 0) into a map");
	ck_assert_msg(map_insert(map, &i2, &f2, &int_eq) == MAPE_OK,
			"Failed to insert (10, 0.5) into a map");
	ck_assert_msg(map_insert(map, &i3, &f3, &int_eq) == MAPE_OK,
			"Failed to insert (2, 1.1) into a map");
	ck_assert_msg(map_insert(map, &i4, &f4, &int_eq) == MAPE_OK,
			"Failed to insert (3, 10.2) into a map");

	map_destroy_ex(map, &no_op_pair_destroyer);
}
END_TEST;

START_TEST(test_lookup)
{
	struct map *map = map_create_fs(10, sizeof(int), 0);

	int i1 = 0;
	int i2 = 10;
	int i3 = 2;
	int i4 = 3;

	double f1 = 0;
	double f2 = 0.5;
	double f3 = 1.1;
	double f4 = 10.2;

	ck_assert_msg(map_insert(map, &i1, &f1, &int_eq) == MAPE_OK,
			"Failed to insert (0, 0) into a map");
	ck_assert_msg(map_insert(map, &i2, &f2, &int_eq) == MAPE_OK,
			"Failed to insert (10, 0.5) into a map");
	ck_assert_msg(map_insert(map, &i3, &f3, &int_eq) == MAPE_OK,
			"Failed to insert (2, 1.1) into a map");
	ck_assert_msg(map_insert(map, &i4, &f4, &int_eq) == MAPE_OK,
			"Failed to insert (3, 10.2) into a map");

	void *res;
	int ok = map_lookup(map, &i1, &int_eq, &res);
	ck_assert_msg(ok, "0 is not found in the map");
	ck_assert_msg(*((double *)res) == f1, "Wrong value is associated with 0");

	ok = map_lookup(map, &i2, &int_eq, &res);
	ck_assert_msg(ok, "10 is not found in the map");
	ck_assert_msg(*((double *)res) == f2, "Wrong value is associated with 10");

	ok = map_lookup(map, &i3, &int_eq, &res);
	ck_assert_msg(ok, "2 is not found in the map");
	ck_assert_msg(*((double *)res) == f3, "Wrong value is associated with 2");

	ok = map_lookup(map, &i4, &int_eq, &res);
	ck_assert_msg(ok, "3 is not found in the map");
	ck_assert_msg(*((double *)res) == f4, "Wrong value is associated with 3");

	map_destroy(map);
}
END_TEST;

START_TEST(test_expand)
{
	struct map *map = map_create_fs(10, sizeof(int), 0);

	ck_assert_msg(map_expand(map, 2, 1), "Failed to expand a map");
	ck_assert_msg(map_num_buckets(map) > 10, "The number of buckets did not increase");

	map_destroy(map);
}
END_TEST;

START_TEST(test_remove)
{
	struct map *map = map_create_fs(10, sizeof(int), 0);

	int i1 = 0;
	int i2 = 10;
	int i3 = 2;
	int i4 = 3;

	double f1 = 0;
	double f2 = 0.5;
	double f3 = 1.1;
	double f4 = 10.2;

	ck_assert_msg(map_insert(map, &i1, &f1, &int_eq) == MAPE_OK,
			"Failed to insert (0, 0) into a map");
	ck_assert_msg(map_insert(map, &i2, &f2, &int_eq) == MAPE_OK,
			"Failed to insert (10, 0.5) into a map");
	ck_assert_msg(map_insert(map, &i3, &f3, &int_eq) == MAPE_OK,
			"Failed to insert (2, 1.1) into a map");
	ck_assert_msg(map_insert(map, &i4, &f4, &int_eq) == MAPE_OK,
			"Failed to insert (3, 10.2) into a map");

	void *removed;
	ck_assert_msg(map_remove(map, &i1, &int_eq, 0, &removed),
			"Failed to remove (0, 0) from a map");
	ck_assert_msg(removed == &f1, "Removed value is not 0");

	void *res;
	int ok;

	ok = map_lookup(map, &i1, &int_eq, &res);
	ck_assert_msg(!ok, "0 is found in the map");

	ok = map_lookup(map, &i2, &int_eq, &res);
	ck_assert_msg(ok, "10 is not found in the map");
	ck_assert_msg(*((double *)res) == f2, "Wrong value is associated with 10");

	ok = map_lookup(map, &i3, &int_eq, &res);
	ck_assert_msg(ok, "2 is not found in the map");
	ck_assert_msg(*((double *)res) == f3, "Wrong value is associated with 2");

	ok = map_lookup(map, &i4, &int_eq, &res);
	ck_assert_msg(ok, "3 is not found in the map");
	ck_assert_msg(*((double *)res) == f4, "Wrong value is associated with 3");

	map_destroy(map);
}
END_TEST;

Suite *
map_suite(void)
{
	Suite *res = suite_create("Map");

	/* Core tests. */
	TCase *core_tests = tcase_create("Core");
	tcase_add_test(core_tests, test_creation);
	tcase_add_test(core_tests, test_lookup);
	tcase_add_test(core_tests, test_expand);
	tcase_add_test(core_tests, test_remove);

	suite_add_tcase(res, core_tests);

	return res;
}

int
main(int argc, char **argv)
{
	int failed = 0;
	Suite *suite = map_suite();
	SRunner *runner = srunner_create(suite);

	srunner_run_all(runner, CK_NORMAL);
	failed = srunner_ntests_failed(runner);
	srunner_free(runner);

	return (failed == 0) ? 0 : 1;
}

/* ---------- helper things ---------- */

void
no_op_pair_destroyer(void *pair)
{
	(void) 0;
}

int
int_eq(void *i1, void *i2)
{
	int *a = i1;
	int *b = i2;
	return *a == *b;
}
