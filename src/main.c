#include <stdio.h>
#include "pforth.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
  unsigned char str[8];
  time_t t;
  /* Intializes random number generator */
  srand((unsigned) time(&t));
  for (int i = 0; i < 20; i++) {
    for (int c = 0; c < 8; c++)
      str[c] = rand() % 26 + 'a';
    printf("#%02d std: %8s hash: %08x\n", i, str, hash(str));
  }
  return 0;
}
