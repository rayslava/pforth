#include <stdio.h>
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
  printf("Result: %d", pop_int32_t());
  return 0;
}
