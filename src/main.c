#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "pforth.h"

int main() {
  pforth_init();
  char line[1024];
  while (fgets(line, 1024, stdin) != 0) {
    eval(forth_dict, line);
    PRINT("  %s\n", "ok")
  }
  pforth_deinit();
  return 0;
}
