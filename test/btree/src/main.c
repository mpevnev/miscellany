
#include <check.h>
#include "btree.h"

START_TEST(test_create_and_destroy)
{

}
END_TEST;

Suite *
btree_suite(void)
{
	Suite *res = suite_create("BTree");

	/* Core tests. */
	TCase *core_tests = tcase_create("Core");
	tcase_add_test(core_tests, test_create_and_destroy);

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
