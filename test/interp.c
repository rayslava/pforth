#include <pforth.h>
#include <stdlib.h>
#include <check.h>

void pforth_init();

START_TEST (hash_test)
{
  const unsigned char printf_line[] = "printf";
  const unsigned char print_line[] = "print";
  ck_assert_uint_eq(hash(printf_line), 0x156b2bb8);
  ck_assert_uint_eq(hash(print_line),  0x102a0912);
}
END_TEST


Suite * interp_suite(void)
{
  Suite* s;
  TCase* tc_core;

  s = suite_create("interp");

  /* Core test case */
  tc_core = tcase_create("Core");

  tcase_add_test(tc_core, hash_test);
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
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
