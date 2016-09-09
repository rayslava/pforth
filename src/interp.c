#include "pforth.h"

void push_int32_t(uint32_t value);

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
  DBG("Line:   %s\n", line);
  const char* begin = line;

  while (*begin) {
    char* token = NULL;
    const char* end = NULL;
    while (*begin == ' ') {
      begin++;
      if (!*begin)
        goto next;
    }
    ;
    /* Parse comments */
    if (*begin == '(') {
      DBG("Comment! %s\n", begin);
      while (*begin && *begin != ')' && *begin != '\n')
        begin++;
      if (*begin != ')') {
        /* TODO: Handle parse failure */
      } else {
        end = begin + 1;
        goto next;
      }
    }
    if (*begin == '\\') {
      while (*begin && *begin != '\n')
        begin++;
      if (!*begin)
        goto next;
    }
    /* Token start */
    end = begin;
    while (*end && *end != ' ')
      end++;
    token = strndup(begin, end - begin);
    upstring(token);
    DBG("Token:  %s\n", token);
    /* Now our need to detect what token is */
    char* err = NULL;
    long int num = strtol(token, &err, 10);
    if (!*err) {
      DBG("Push:   %ld\n", num);
      push_int32_t(num);
      goto next;
    }

    /* Try to call the word */
    pforth_word_ptr word;
    if ((word = dict_get(dict, token)) != NULL) {
      if (word->location) {
        DBG("Exec:   %s word from %p\n", token, word->location);
        word->function();
      }
      else {
        DBG("Eval:   %s word from '%s'\n", token, word->text_code);
        eval(dict, word->text_code);
      }
    }
  next:
    if (*begin == 0)
      break;
    free(token);
    begin = end;
  }
}
