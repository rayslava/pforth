#include "pforth.h"

void push_int32_t(int32_t value);

char* strndup(const char* src, size_t len) {
  char* result = calloc(1, len + 1);
  strncpy(result, src, len);
  return result;
}

char upcase(char c) {
  if (c >= 'a' && c <= 'z')
    c += 'A' - 'a';
  return c;
}

void upstring(char* line) {
  char* c = line;
  while ((*c = upcase(*c)))
    c++;
}

void eval(dict_t* dict, const char* line) {
  const char* begin = line;

  while (*begin) {
    while (*begin == ' ') {
      begin++;
      if (!*begin)
        goto next;
    }
    ;
    /* Token start */
    const char* end = begin;
    while (*end && *end != ' ')
      end++;
    char* token = strndup(begin, end - begin);
    upstring(token);

    /* Now our need to detect what token is */
    char* err = NULL;
    long int num = strtol(token, &err, 10);
    if (!*err) {
      push_int32_t(num);
      goto next;
    }

    /* Try to call the word */
    pforth_word_ptr word;
    if ((word = dict_get(dict, token)) != NULL)
      word->function();
  next:
    if (*begin == 0)
      break;
    free(token);
    begin = end;
  }
}
