#include "pforth.h"
#include <stdlib.h>
#include <check.h>

START_TEST(add)
{
  push_int32_t(10);
  push_int32_t(5);
  _add_int32_t();
  int32_t r = pop_int32_t();
  ck_assert_int_eq(r, 15);
}
END_TEST

START_TEST(sum_tests)
{
  eval(forth_dict, "10 5 +");
  int32_t r = pop_int32_t();
  ck_assert_int_eq(r, 15);

  eval(forth_dict, "1 2 3 4 5 6 + + + + +");
  r = pop_int32_t();
  ck_assert_int_eq(r, 21);
}
END_TEST

START_TEST(comparison_test)
{
  eval(forth_dict, "5 3 >");
  ck_assert_int_eq(TRUE, 1);

  eval(forth_dict, "3 5 >");
  ck_assert_int_eq(TRUE, 0);

  eval(forth_dict, "1 2 ==");
  ck_assert_int_eq(TRUE, 0);

  eval(forth_dict, "1 2 >=");
  ck_assert_int_eq(TRUE, 0);

  eval(forth_dict, "1 1 ==");
  ck_assert_int_eq(TRUE, 1);
}
END_TEST

Suite* math_suite(void)
{
  Suite* s;
  TCase* tc_core;

  s = suite_create("math");

  /* Core test case */
  tc_core = tcase_create("Core");
  tcase_add_test(tc_core,	      add);
  tcase_add_test(tc_core,	sum_tests);
  tcase_add_test(tc_core, comparison_test);
  suite_add_tcase(s, tc_core);
  return s;
}

int main(void)
{
  int number_failed;
  Suite* s;
  SRunner* sr;

  s = math_suite();
  sr = srunner_create(s);

  pforth_init();
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  pforth_deinit();
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
