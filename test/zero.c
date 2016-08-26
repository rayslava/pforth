#include <stdlib.h>
#include <check.h>

int a();

START_TEST (test_zero)
{
  ck_assert_int_eq(a(), 3);
}
END_TEST


Suite * zero_suite(void)
{
  Suite* s;
  TCase* tc_core;

  s = suite_create("Money");

  /* Core test case */
  tc_core = tcase_create("Core");

  tcase_add_test(tc_core, test_zero);
  suite_add_tcase(s, tc_core);

  return s;
}

int main(void)
{
  int number_failed;
  Suite* s;
  SRunner* sr;

  s = zero_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
