#include "pforth.h"
#include "token.h"

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

char lowcase(char c) {
  if (c >= 'A' && c <= 'Z')
    c -= 'A' - 'a';
  return c;
}

void upstring(char* line) {
  char* c = line;
  while ((*c = upcase(*c)))
    c++;
}

/* Slow but working implemenation of strcasestr */
const char* find_token(const char* haystack, const char* needle) {
  if (haystack == NULL || needle == NULL) return NULL;
  const char* haystack_pos = haystack;
  const char* haystack_pos_last;
  const char* needle_pos;
  char haystack_char;
  char needle_char;
  int found = 0;

  while (*haystack_pos != 0) {
    found = 1;
    if (*haystack_pos != *needle) {
      haystack_char = lowcase(*haystack_pos);
      needle_char = lowcase(*needle);

      if (haystack_char == needle_char)
        found = 1;
    }

    if (found) {
      haystack_pos_last = haystack_pos;
      needle_pos = needle;
      while (*haystack_pos_last != 0) {
        haystack_char = lowcase(*haystack_pos_last);
        needle_char = lowcase(*needle_pos);
        if (haystack_char != needle_char)
          break;
        needle_pos++;
        if (*needle_pos == 0)
          return haystack_pos_last - (needle_pos - needle - 0x01);
        haystack_pos_last++;
      }
    }
    haystack_pos++;
  }
  return NULL;
}

static int if_depth;

void eval(dict_t* dict, const char* line) {
  DBG("Line:   %s\n", line);
  const char* begin = line;

  while (begin && *begin) {
    char* token = NULL;
    const char* end = NULL;
    while (*begin == ' ') {
      begin++;
      if (!*begin || *begin == '\n')   /* Trailing whitespaces */
        goto next;
    };

    /* Parse comments */
    if (*begin == '(') {
      DBG("Comment! %s\n", begin);
      while (*begin && *begin != ')' && *begin != '\n')
        begin++;
      if (*begin != ')') {
        goto error;
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
    while (*end && *end != ' ' && *end != '\n')
      end++;

    token = strndup(begin, end - begin);
    upstring(token);

    if (strlen(token) == 0 && *end == '\n') {
      ++end;
      goto next;
    }

    DBG("Token:  %s\n", token);

    /* Now our need to detect what token is */

    /* Number */
    char* err = NULL;
    long int num = strtol(token, &err, 10);
    if (!*err) {
      DBG("Push:   %ld\n", num);
      push_int32_t(num);
      goto next;
    }

    /* Conditional: IF */
    if ((strcmp(token, IF_TOKEN)) == 0) {
      if_depth++;
      const char* then_pos = NULL;
      const char* else_pos = NULL;
      else_pos = find_token(end, ELSE_TOKEN);
      if ((then_pos = find_token(end, THEN_TOKEN)) == NULL)
        goto error;

      if (!TRUE) {
        /* Condition is false */
        if (else_pos && else_pos < then_pos) {
          /* Jump to else if there's one */
          end = else_pos + strlen(ELSE_TOKEN);
          goto next;
        } else {
          /* Jump to then */
          end = then_pos + strlen(THEN_TOKEN);
          goto next;
        }
      } else {
        /* Condition is true */
        goto next;
      }
    }

    /* Conditional: THEN */
    if ((strcmp(token, THEN_TOKEN)) == 0) {
      if (!if_depth)
        goto error;
      if_depth--;
      goto next;
    }

    /* Try to call the word */
    pforth_word_ptr word;
    if ((word = dict_get(dict, token)) != NULL) {
      DBG("Word:   %s\n", token);
      if (word->location) {
        DBG("Exec:   %s word from %p\n", token, word->location);
        word->function();
        goto next;
      }
      else {
        DBG("Eval:   %s word from '%s'\n", token, word->text_code);
        eval(dict, word->text_code);
        goto next;
      }
    }
  error:
    DBG("Cant parse:   %s!\n", token);
    free(token);
    break;
  next:
    free(token);
    if (*begin == 0)
      break;
    begin = end;
  }
}
