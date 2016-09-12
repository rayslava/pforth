#include "pforth.h"
#include <stdlib.h>
#include <check.h>

START_TEST(dup_tests)
{
  eval(forth_dict, "1024 DUP");
  ck_assert_int_eq(pop_int32_t(), 1024);
  ck_assert_int_eq(pop_int32_t(), 1024);
  eval(forth_dict, "1024 2048 2DUP");
  ck_assert_int_eq(pop_int32_t(), 2048);
  ck_assert_int_eq(pop_int32_t(), 1024);
  ck_assert_int_eq(pop_int32_t(), 2048);
  ck_assert_int_eq(pop_int32_t(), 1024);
  eval(forth_dict, "1024 0 ?DUP");
  ck_assert_int_eq(pop_int32_t(), 0);
  ck_assert_int_eq(pop_int32_t(), 1024);
}
END_TEST

START_TEST(drop_tests)
{
  eval(forth_dict, "1024 2048 DROP");
  ck_assert_int_eq(pop_int32_t(), 1024);
  eval(forth_dict, "512 1024 2048 2DROP");
  ck_assert_int_eq(pop_int32_t(), 512);
}
END_TEST

START_TEST(rot_tests)
{
  eval(forth_dict, "1 2 3 ROT");
  ck_assert_int_eq(pop_int32_t(), 1);
  ck_assert_int_eq(pop_int32_t(), 3);
  ck_assert_int_eq(pop_int32_t(), 2);
  eval(forth_dict, "1 2 3 -ROT");
  ck_assert_int_eq(pop_int32_t(), 2);
  ck_assert_int_eq(pop_int32_t(), 1);
  ck_assert_int_eq(pop_int32_t(), 3);

}
END_TEST

START_TEST(swap_tests)
{
  eval(forth_dict, "1024 2048 SWAP");
  ck_assert_int_eq(pop_int32_t(), 1024);
  ck_assert_int_eq(pop_int32_t(), 2048);
}
END_TEST

START_TEST(core_tests)
{
  eval(forth_dict, "1024 2048 NIP");
  ck_assert_int_eq(pop_int32_t(), 2048);
  eval(forth_dict, "1024 2048 TUCK");
  ck_assert_int_eq(pop_int32_t(), 2048);
  ck_assert_int_eq(pop_int32_t(), 1024);
  ck_assert_int_eq(pop_int32_t(), 2048);
}
END_TEST

Suite* math_suite(void)
{
  Suite* s;
  TCase* tc_core;

  s = suite_create("math");

  /* Core test case */
  tc_core = tcase_create("Core");
  tcase_add_test(tc_core, dup_tests);
  tcase_add_test(tc_core, drop_tests);
  tcase_add_test(tc_core, rot_tests);
  tcase_add_test(tc_core, swap_tests);
  tcase_add_test(tc_core, core_tests);
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
