#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "pforth.h"

void push_int32_t(int32_t value);
int32_t pop_int32_t();
void init();
void _add_int32_t();

int main() {
  pforth_init();
  push_int32_t( 5);
  push_int32_t(10);
  _add_int32_t();
  printf("Result: %d\n", pop_int32_t());

  dict_t* dict = dict_create(50);
  pforth_word_ptr word = pforth_word_alloc();
  word->function = _add_int32_t;
  dict_set(dict, "ADD", word);
  dict_set(dict, "+",	word);

  char val[] = "Test-test";
  word->text_code = val;
  word->size = 5;
  dict_set(dict, "test",  word);
  dict_set(dict, "test2", word);
  dict_set(dict, "test3", word);
  dict_set(dict, "test4", word);

  printf("%s\n", dict_get(dict, "test")->text_code);
  printf("%s\n", dict_get(dict, "test2")->text_code);
  printf("%s\n", dict_get(dict, "test3")->text_code);
  printf("%s\n", dict_get(dict, "test4")->text_code);
  push_int32_t( 5);
  push_int32_t(10);

  pforth_word_ptr w = dict_get(dict, "ADD");
  w->function();
  printf("Result: %d\n", pop_int32_t());

  eval(dict, "  5 10 20 add add   ");
  printf("Result: %d\n", pop_int32_t());

  char word_numbers[] = "5 10 20";
  pforth_word_ptr numbers_word = pforth_word_alloc();
  numbers_word->text_code = word_numbers;
  dict_set(dict, "NUMS", numbers_word);

  eval(dict, "nums + +");
  printf("Result: %d\n", pop_int32_t());

  dict_free(dict, 50);
  pforth_deinit();
  return 0;
}
