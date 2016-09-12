#include <stdlib.h>
#include <check.h>
#include <stdio.h>
#include <unistd.h>

#include "pforth.h"

#define MAX_LEN 40

void pforth_init();
char* strndup(const char* src, size_t len);

START_TEST(preprocess_test)
{
  char l[] = "test TEst ( ) \n teeeeest        \t\t test; tEs\nT\n\\testetest\n1\n.\" aSdF\"";
  char e[] = "TEST TEST       TEEEEEST           TEST; TES T            1 .\" aSdF\"";
  preprocess(l);
  ck_assert_str_eq(l, e);
}
END_TEST


START_TEST(replace_test)
{
  dict_t* dict = dict_create(20);
  pforth_word_ptr word = pforth_word_alloc();
  word->function = &_gt_int32_t;
  dict_set(dict, ">", word);

  eval(dict, "5 3 >");
  int32_t r = pop_int32_t();
  ck_assert_int_eq(r, -1);

  word->function = &_lt_int32_t;
  dict_set(dict, ">", word);

  eval(dict, "5 3 >");
  r = pop_int32_t();
  ck_assert_int_eq(r, 0);

  dict_free(dict, 20);
  free(dict);
}
END_TEST

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

START_TEST(emit_test)
{
  char buffer[MAX_LEN + 1] = {0};
  int out_pipe[2];
  int saved_stdout;

  saved_stdout = dup(STDOUT_FILENO);
  if (pipe(out_pipe) != 0) exit(1);
  dup2(out_pipe[1], STDOUT_FILENO);
  close(out_pipe[1]);

  eval(forth_dict, "10 .");
  fflush(stdout);
  int r = read(out_pipe[0], buffer, MAX_LEN);
  ck_assert_str_eq(buffer, "10");

  eval(forth_dict, "48 EMIT 49 EMIT 50 EMIT");
  fflush(stdout);
  r = read(out_pipe[0], buffer, MAX_LEN);
  ck_assert_str_eq(buffer, "012");

  dup2(saved_stdout, STDOUT_FILENO);
}
END_TEST

START_TEST(if_tests)
{
  eval(forth_dict, "0 1 2 > IF 1 THEN");
  int32_t r = pop_int32_t();
  ck_assert_int_eq(r, 0);

  eval(forth_dict, "0 2 1 > IF 1 THEN");
  r = pop_int32_t();
  ck_assert_int_eq(r, 1);

  eval(forth_dict, "2 1 > IF 1 ELSE 0 THEN");
  r = pop_int32_t();
  ck_assert_int_eq(r, 1);

  eval(forth_dict, "1 2 > IF 1 ELSE 0 THEN");
  r = pop_int32_t();
  ck_assert_int_eq(r, 0);

  eval(forth_dict, "0 1 2 > IF 1 then");
  r = pop_int32_t();
  ck_assert_int_eq(r, 0);

  eval(forth_dict, "1 2 > IF 1 else 0 THEN");
  r = pop_int32_t();
  ck_assert_int_eq(r, 0);
}
END_TEST

Suite* interp_suite(void)
{
  Suite* s;
  TCase* tc_core;

  s = suite_create("interp");

  /* Core test case */
  tc_core = tcase_create("Core");

  tcase_add_test(tc_core, preprocess_test);
  tcase_add_test(tc_core, replace_test);
  tcase_add_test(tc_core, hash_test);
  tcase_add_test(tc_core, add_test);
  tcase_add_test(tc_core, eval_empty);
  tcase_add_test(tc_core, eval_add_test);
  tcase_add_test(tc_core, eval_add2_test);
  tcase_add_test(tc_core, eval_recursion_test);
  tcase_add_test(tc_core, emit_test);
  tcase_add_test(tc_core, if_tests);
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
