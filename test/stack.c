#include <pforth.h>
#include <stdlib.h>
#include <check.h>

START_TEST(test_push_pop)
{
  push_int32_t(0xfefefefe);
  uint32_t r = pop_int32_t();
  ck_assert_int_eq(r, 0xfefefefe);
}
END_TEST

START_TEST(test_new_empty_word)
{
  pforth_word_ptr nw = dict_set(forth_dict, "test_word", NULL);
  /* Try to address the pointer and not get SIGSEGV */
  ck_assert_int_eq(nw->size, 0);
}
END_TEST

Suite* stack_suite(void)
{
  Suite* s;
  TCase* tc_core;

  s = suite_create("stack");

  /* Core test case */
  tc_core = tcase_create("Core");

  tcase_add_test(tc_core, test_push_pop);
  suite_add_tcase(s, tc_core);

  return s;
}

int main(void)
{
  int number_failed;
  Suite* s;
  SRunner* sr;

  s = stack_suite();
  sr = srunner_create(s);

  pforth_init();
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  pforth_deinit();
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
