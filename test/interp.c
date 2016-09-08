#include <stdlib.h>
#include <check.h>
#include "pforth.h"

void pforth_init();
void _add_int32_t();
void push_int32_t(int32_t value);
int32_t pop_int32_t();
char* strndup(const char* src, size_t len);

START_TEST(hash_test)
{
  static int a = 0;
  void* i = &a;
  dict_t* dict = dict_create(20);
  pforth_word_ptr word = pforth_word_alloc();
  char val[] = "Test-test";
  word->text_code = val;
  word->location = i;
  word->size = 5;
  dict_set(dict, "test", word);
  ck_assert_str_eq(dict_get(dict, "test")->text_code, val);
  ck_assert_int_eq(dict_get(dict, "test")->size, 5);
  dict_free(dict, 20);
  free(dict);
}
END_TEST

START_TEST(add_test)
{
  dict_t* dict = dict_create(20);
  pforth_word_ptr word = pforth_word_alloc();
  word->function = &_add_int32_t;
  dict_set(dict, "test", word);

  push_int32_t(5);
  push_int32_t(10);

  dict_get(dict, "test")->function();
  int32_t r = pop_int32_t();
  ck_assert_int_eq(r, 15);
  pforth_word_free(word);
  dict_free(dict, 20);
  free(dict);
}
END_TEST

START_TEST(eval_empty)
{
  dict_t* dict = dict_create(20);

  eval(dict, "");
  eval(dict, "\n\n");
  eval(dict, "     ");
  dict_free(dict, 20);
  free(dict);
}
END_TEST

START_TEST(eval_add_test)
{
  dict_t* dict = dict_create(20);
  pforth_word_ptr word = pforth_word_alloc();
  word->function = &_add_int32_t;
  dict_set(dict, "ADD", word);

  push_int32_t(5);
  push_int32_t(10);

  eval(dict, "add");
  int32_t r = pop_int32_t();
  ck_assert_int_eq(r, 15);
  dict_free(dict, 20);
  free(dict);
}
END_TEST

START_TEST(eval_add2_test)
{
  dict_t* dict = dict_create(20);
  pforth_word_ptr word = pforth_word_alloc();
  word->function = &_add_int32_t;
  dict_set(dict, "+", word);

  eval(dict, "5 10 20 + +");
  int32_t r = pop_int32_t();
  ck_assert_int_eq(r, 35);
  dict_free(dict, 20);
  free(dict);
}
END_TEST

START_TEST(eval_recursion_test)
{
  dict_t* dict = dict_create(20);
  pforth_word_ptr word = pforth_word_alloc();
  word->function = &_add_int32_t;
  dict_set(dict, "+", word);

  char word_numbers[] = "5 10 20";
  pforth_word_ptr numbers_word = pforth_word_alloc();
  numbers_word->text_code = word_numbers;
  dict_set(dict, "NUMS", numbers_word);

  eval(dict, "nums + +");
  int32_t r = pop_int32_t();
  ck_assert_int_eq(r, 35);
  dict_free(dict, 20);
  free(dict);
}
END_TEST

START_TEST(eval_comment_test)
{
  dict_t* dict = dict_create(50);
  pforth_word_ptr word = pforth_word_alloc();
  word->function = &_add_int32_t;
  dict_set(dict, "+", word);

  char word_numbers[] = "5 10 (20) 30";
  pforth_word_ptr numbers_word = pforth_word_alloc();
  numbers_word->text_code = strndup(word_numbers, 12);
  numbers_word->location = 0;
  numbers_word->size = 0;
  dict_set(dict, "NUMBERS", numbers_word);

  eval(dict, "numbers + +");
  int32_t r = pop_int32_t();
  ck_assert_int_eq(r, 45);

  eval(dict, "5 \\numbers + +");
  r = pop_int32_t();
  ck_assert_int_eq(r, 5);

  dict_free(dict, 50);
  free(dict);
}
END_TEST


Suite* interp_suite(void)
{
  Suite* s;
  TCase* tc_core;

  s = suite_create("interp");

  /* Core test case */
  tc_core = tcase_create("Core");

  tcase_add_test(tc_core, hash_test);
  tcase_add_test(tc_core, add_test);
  tcase_add_test(tc_core, eval_empty);
  tcase_add_test(tc_core, eval_add_test);
  tcase_add_test(tc_core, eval_add2_test);
  tcase_add_test(tc_core, eval_recursion_test);
  tcase_add_test(tc_core, eval_comment_test);
  suite_add_tcase(s, tc_core);

  return s;
}

int main(void)
{
  int number_failed;
  Suite* s;
  SRunner* sr;

  s = interp_suite();
  sr = srunner_create(s);

  pforth_init();
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  pforth_deinit();
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
