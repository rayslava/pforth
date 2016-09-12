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

int preprocess(char* line) {
  char* pos = line;
  while (*pos) {
    *pos = upcase(*pos);
    if (*pos == '.' && *(pos + 1) && *(pos + 1) == '"') {
      /* Text literal shouldn't be changed, skip it */
      pos += 2;
      while (*pos && *pos != '"') pos++;
      if (!*pos)
        return -1; /* String literal isn't closed */
    }

    /* In-line comment */
    if (*pos == '(') {
      while (*pos && *pos != ')') {
        if (*pos == '\n' || *pos == 0)
          return -1; /* Single line comment error */
        *pos++ = ' ';
      }
      *pos = ' ';
    }

    /* Line comment */
    if (*pos == '\\') {
      while (*pos && *pos != '\n')
        *pos++ = ' ';
      continue;
    }

    if (*pos == '\n' || *pos == '\t') {
      while (*pos && (*pos == ' ' || *pos == '\n' || *pos == '\t'))
        *pos++ = ' ';
      continue;
    }
    pos++;
  };
  return 0;
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
      if (!*begin)   /* Trailing whitespaces */
        goto next;
    };

    /* Token start */
    end = begin;
    while (*end && *end != ' ')
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
      else_pos = strstr(end, ELSE_TOKEN);
      if ((then_pos = strstr(end, THEN_TOKEN)) == NULL)
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

    /* New word definition */
    if (*begin == ':') {
      begin++;
      while (*begin && *begin == ' ')
        begin++;

      if (!*begin)   /* EOL when looking for word name */
        goto error;

      end = begin;
      while (*end && *end != ' ')
        end++;

      if (!*end)   /* No actual line */
        goto error;

      char* word_key = strndup(begin, end - begin);
      pforth_word_ptr new_word;
      if ((new_word = pforth_word_alloc()) == NULL) {
        /* Not enough memory */
        free(word_key);
        goto error;
      }
      pforth_word_ptr dict_word = dict_set(dict, word_key, new_word);
      free(word_key);
      pforth_word_free(new_word);
      begin = end + 1;
      while (*end && *end != ';')
        end++;

      if (!*end)
        goto error;

      dict_word->text_code = strndup(begin, end - begin);
      end++;
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
