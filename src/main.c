#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "pforth.h"

int main() {
  pforth_init();
  eval(forth_dict, "  5 10 20 add add   ");
  printf("Result: %d\n", pop_int32_t());

  char word_numbers[] = "5 10 (20) 30";
  pforth_word_ptr numbers_word = pforth_word_alloc();
  numbers_word->text_code = word_numbers;
  dict_set(forth_dict, "NUMS", numbers_word);

  eval(forth_dict, "nums + +");
  printf("Result: %d\n", pop_int32_t());

  eval(forth_dict, "1 2 >=");
  printf("Result: %d\n", pop_int32_t());

  pforth_deinit();
  return 0;
}
