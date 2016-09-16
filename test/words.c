#include "pforth.h"
#include <stdlib.h>
#include <check.h>

START_TEST(dup_tests)
{
  eval(forth_dict, "1024 DUP", NULL);
  ck_assert_int_eq(pop_int32_t(), 1024);
  ck_assert_int_eq(pop_int32_t(), 1024);
  eval(forth_dict, "1024 2048 2DUP", NULL);
  ck_assert_int_eq(pop_int32_t(), 2048);
  ck_assert_int_eq(pop_int32_t(), 1024);
  ck_assert_int_eq(pop_int32_t(), 2048);
  ck_assert_int_eq(pop_int32_t(), 1024);
  eval(forth_dict, "1024 0 ?DUP", NULL);
  ck_assert_int_eq(pop_int32_t(), 0);
  ck_assert_int_eq(pop_int32_t(), 1024);
}
END_TEST

START_TEST(drop_tests)
{
  eval(forth_dict, "1024 2048 DROP", NULL);
  ck_assert_int_eq(pop_int32_t(), 1024);
  eval(forth_dict, "512 1024 2048 2DROP", NULL);
  ck_assert_int_eq(pop_int32_t(), 512);
}
END_TEST

START_TEST(rot_tests)
{
  eval(forth_dict, "1 2 3 ROT", NULL);
  ck_assert_int_eq(pop_int32_t(), 1);
  ck_assert_int_eq(pop_int32_t(), 3);
  ck_assert_int_eq(pop_int32_t(), 2);
  eval(forth_dict, "1 2 3 -ROT", NULL);
  ck_assert_int_eq(pop_int32_t(), 2);
  ck_assert_int_eq(pop_int32_t(), 1);
  ck_assert_int_eq(pop_int32_t(), 3);

}
END_TEST

START_TEST(swap_tests)
{
  eval(forth_dict, "1024 2048 SWAP", NULL);
  ck_assert_int_eq(pop_int32_t(), 1024);
  ck_assert_int_eq(pop_int32_t(), 2048);
}
END_TEST

START_TEST(core_tests)
{
  eval(forth_dict, "1024 2048 NIP", NULL);
  ck_assert_int_eq(pop_int32_t(), 2048);
  eval(forth_dict, "1024 2048 TUCK", NULL);
  ck_assert_int_eq(pop_int32_t(), 2048);
  ck_assert_int_eq(pop_int32_t(), 1024);
  ck_assert_int_eq(pop_int32_t(), 2048);
}
END_TEST

START_TEST(depth_tests)
{
  eval(forth_dict, "DEPTH", NULL);
  ck_assert_int_eq(pop_int32_t(), 0);
  eval(forth_dict, "1024 2048 DEPTH", NULL);
  ck_assert_int_eq(pop_int32_t(), 2);
}
END_TEST


START_TEST(loop_tests)
{
  eval(forth_dict, "10 0 DO I LOOP", NULL);
  for (int i = 0; i < 11; i++)
    ck_assert_int_eq(pop_int32_t(), i);

  eval(forth_dict, "0 10 DO I -1 +LOOP", NULL);
  for (int32_t i = 10; i >= 0; i--)
    ck_assert_int_eq(pop_int32_t(), i);

  eval(forth_dict, "10 0 DO 0 10 DO J -1 +LOOP LOOP", NULL);
  for (int i = 0; i < 11; i++)
    for (int j = 10; j >= 0; j--)
      ck_assert_int_eq(pop_int32_t(), j);

  eval(forth_dict, "10 0 DO 10 0 DO 10 0 DO K LOOP LOOP LOOP", NULL);
  for (int i = 0; i < 11; i++)
    for (int j = 0; j < 11; j++)
      for (int k = 0; k < 11; k++)
        ck_assert_int_eq(pop_int32_t(), k);
}
END_TEST

START_TEST(begin_tests)
{
  eval(forth_dict, "5 BEGIN DUP 1- DUP 0 == UNTIL", NULL);
  for (int i = 0; i < 6; i++)
    ck_assert_int_eq(pop_int32_t(), i);

  eval(forth_dict, "3 BEGIN 1 BEGIN DUP 1- DUP 0 == UNTIL ROT 1- DUP 0 == UNTIL DROP", NULL);
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 2; j++)
      ck_assert_int_eq(pop_int32_t(), j);

  eval(forth_dict, "5 BEGIN 1- DUP 0 > WHILE DUP REPEAT", NULL);
  for (int i = 0; i < 5; i++)
    ck_assert_int_eq(pop_int32_t(), i);

  eval(forth_dict, "5 BEGIN 1- DUP 0 > WHILE DUP BEGIN 0 1 == WHILE 42 EMIT REPEAT REPEAT", NULL);
  dump_stack();
  for (int i = 0; i < 5; i++)
    ck_assert_int_eq(pop_int32_t(), i);
}
END_TEST

START_TEST(if_tests)
{
  eval(forth_dict, "0 1 2 > IF 1 THEN", NULL);
  int32_t r = pop_int32_t();
  ck_assert_int_eq(r, 0);

  eval(forth_dict, "0 2 1 > IF 1 THEN", NULL);
  r = pop_int32_t();
  ck_assert_int_eq(r, 1);

  eval(forth_dict, "2 1 > IF 1 ELSE 0 THEN", NULL);
  r = pop_int32_t();
  ck_assert_int_eq(r, 1);

  eval(forth_dict, "1 2 > IF 1 ELSE 0 THEN", NULL);
  r = pop_int32_t();
  ck_assert_int_eq(r, 0);

  eval(forth_dict, "0 1 2 > IF 1 THEN", NULL);
  r = pop_int32_t();
  ck_assert_int_eq(r, 0);

  eval(forth_dict, "1 2 > IF 1 ELSE 0 THEN", NULL);
  r = pop_int32_t();
  ck_assert_int_eq(r, 0);

  eval(forth_dict, "1 2 > IF 1 ELSE 2 1 > IF 0 ELSE 1 THEN THEN", NULL);
  r = pop_int32_t();
  ck_assert_int_eq(r, 0);

  eval(forth_dict, "1 2 > IF 1 ELSE 1 2 > IF 0 ELSE 1 THEN THEN", NULL);
  r = pop_int32_t();
  ck_assert_int_eq(r, 1);

  eval(forth_dict, "1 2 > IF 2 1 > IF 3 ELSE 4 THEN  ELSE 0 THEN", NULL);
  r = pop_int32_t();
  ck_assert_int_eq(r, 0);

}
END_TEST

Suite* words_suite(void)
{
  Suite* s;
  TCase* tc_core;

  s = suite_create("words");

  /* Core test case */
  tc_core = tcase_create("Core");
  tcase_add_test(tc_core, dup_tests);
  tcase_add_test(tc_core, drop_tests);
  tcase_add_test(tc_core, rot_tests);
  tcase_add_test(tc_core, swap_tests);
  tcase_add_test(tc_core, core_tests);
  tcase_add_test(tc_core, depth_tests);
  tcase_add_test(tc_core, if_tests);
  tcase_add_test(tc_core, loop_tests);
  tcase_add_test(tc_core, begin_tests);
  tcase_set_timeout(tc_core, 50);
  suite_add_tcase(s, tc_core);
  return s;
}

int main(void)
{
  int number_failed;
  Suite* s;
  SRunner* sr;

  s = words_suite();
  sr = srunner_create(s);

  pforth_init();
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  pforth_deinit();
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
