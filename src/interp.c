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

struct _loop_struct {
  const char* body_start;       /**< Pointer to the space right after DO */
  FORTH_TYPE value;             /**< Current value of the loop */
  FORTH_TYPE stop_value;        /**< Loop exit condition */
};
static struct _loop_struct loops[MAX_DO_LOOP_DEPTH];
const char* begin_loops[MAX_BEGIN_LOOP_DEPTH];

static FORTH_TYPE if_depth;
static FORTH_TYPE do_loop_depth;
static FORTH_TYPE begin_loop_depth;

void eval(dict_t* dict, const char* line, const char* line_end) {
  DBG("Line:   %.*s\n", (int) (line_end ? line_end - line : strlen(line)), line);
  const char* begin = line;

  while (begin && *begin && (line_end ? begin < line_end : 1)) {
    char* token = NULL;
    const char* end = NULL;
    while (*begin == ' ' && (line_end ? begin < line_end : 1)) {
      begin++;
      if (!*begin || (line_end ? begin > line_end : 0)) /* Trailing whitespaces */
        goto next;
    };

    /* Token start */
    end = begin;
    while (*end && *end != ' ' && (line_end ? end < line_end : 1))
      end++;

    if (*begin == 'I' && end - begin == 1) {
      if (do_loop_depth < 1)
        goto error;
      DBG("I: %d\n", loops[0].value);
      push_int32_t(loops[0].value);
      goto next;
    }

    if (*begin == 'J' && end - begin == 1) {
      if (do_loop_depth < 2)
        goto error;
      DBG("J: %d\n", loops[1].value);
      push_int32_t(loops[1].value);
      goto next;
    }

    if (*begin == 'K' && end - begin == 1) {
      if (do_loop_depth < 3)
        goto error;
      DBG("K: %d\n", loops[2].value);
      push_int32_t(loops[2].value);
      goto next;
    }

    token = strndup(begin, end - begin);

    if (strlen(token) == 0) {
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

      /* Condition is true - continue execution */
      if (TRUE)
        goto next;

      /* Condition is false - look for either ELSE or THEN */
      const char* pos = begin + strlen(IF_TOKEN);
      int if_nest_depth = 0;
      while (*pos && *pos++ == ' ') ;

      while (*pos) {
        /* Check inside the condition body */
        if (strncmp(pos, IF_TOKEN, strlen(IF_TOKEN)) == 0) {
          if_nest_depth++;            /* Found nested IF */
        } else if (strncmp(pos, THEN_TOKEN, strlen(THEN_TOKEN)) == 0) {
          if (!if_nest_depth) {       /* Found our THEN */
            end = pos + strlen(THEN_TOKEN);
            goto next;
          } else {                    /* Found nested THEN */
            if_nest_depth--;
          }
        } else if (strncmp(pos, ELSE_TOKEN, strlen(ELSE_TOKEN)) == 0 &&
                   !if_nest_depth) {  /* Found our ELSE */
          end = pos + strlen(ELSE_TOKEN);
          goto next;
        }
        while (*pos && *pos++ != ' ') ; /* Skip the token */
        while (*pos && *pos++ == ' ') ; /* Skip the spaces */
        pos--;                          /* Set to beginning of the next token */
      }
      goto error;
    }

    /* Conditional: THEN */
    if ((strcmp(token, THEN_TOKEN)) == 0) {
      if (!if_depth)
        goto error;
      if_depth--;
      goto next;
    }

    /* DO LOOP */
    if ((strcmp(token, DO_TOKEN)) == 0) {
      if (do_loop_depth > MAX_DO_LOOP_DEPTH - 1)
        goto error;          /* Can only handle i,j,k for loops */

      if (_depth() < 2)
        goto error;

      do_loop_depth++;
      const int current_loop = do_loop_depth - 1;
      loops[current_loop].stop_value = POP;
      loops[current_loop].value = POP;
      loops[current_loop].body_start = begin + sizeof(DO_TOKEN);
      DBG("DO: from %d to %d\n", loops[current_loop].value, loops[current_loop].stop_value);
      goto next;
    }

    /* DO LOOP */
    if ((strcmp(token, LOOP_TOKEN)) == 0) {
      struct _loop_struct* loop = &loops[do_loop_depth - 1];
      if (loop->value != loop->stop_value) {
        loop->value--;
        end = loop->body_start;
        goto next;
      } else {
        do_loop_depth--;
        goto next;
      }
    }

    /* DO +LOOP */
    if ((strcmp(token, PLOOP_TOKEN)) == 0) {
      if (_depth() < 1)
        goto error;
      FORTH_TYPE increment = POP;
      struct _loop_struct* loop = &loops[do_loop_depth - 1];
      if (loop->value != loop->stop_value) {
        loop->value -= increment;
        end = loop->body_start;
        goto next;
      } else {
        do_loop_depth--;
        goto next;
      }
    }

    /* BEGIN */
    if ((strcmp(token, BEGIN_TOKEN)) == 0) {
      if (begin_loop_depth >= MAX_BEGIN_LOOP_DEPTH)
        goto error;
      begin_loops[begin_loop_depth] = begin + sizeof(BEGIN_TOKEN);
      begin_loop_depth++;
      goto next;
    }

    /* UNTIL */
    if ((strcmp(token, UNTIL_TOKEN)) == 0) {
      if (begin_loop_depth < 1)
        goto error;
      if (TRUE) {
        begin_loop_depth--;
        goto next;
      } else {
        end = begin_loops[begin_loop_depth - 1];
        DBG("UNTIL goes to %s", end);
        goto next;
      }
    }

    /* REPEAT */
    if ((strcmp(token, REPEAT_TOKEN)) == 0) {
      if (begin_loop_depth < 1)
        goto error;
      end = begin_loops[begin_loop_depth - 1];
      DBG("REPEAT goes to %s\n", end);
      goto next;
    }

    /* WHILE */
    if ((strcmp(token, WHILE_TOKEN)) == 0) {
      if (begin_loop_depth < 1)
        goto error;

      if (TRUE)
        goto next;

      /* Condition is false - look for correspoding REPEAT */
      const char* pos = begin + strlen(WHILE_TOKEN);
      int if_nest_depth = 0;
      while (*pos && *pos++ == ' ') ;

      while (*pos) {
        /* Check inside the condition body */
        if (strncmp(pos, BEGIN_TOKEN, strlen(BEGIN_TOKEN)) == 0) {
          if_nest_depth++;            /* Found nested BEGIN */
        } else if (strncmp(pos, REPEAT_TOKEN, strlen(REPEAT_TOKEN)) == 0) {
          if (!if_nest_depth) {       /* Found our REPEAT */
            end = pos + strlen(REPEAT_TOKEN);
            DBG("WHILE goes to %s\n", end);
            begin_loop_depth--;
            goto next;
          } else {                    /* Found nested REPEAT */
            if_nest_depth--;
          }
        }
        while (*pos && *pos++ != ' ') ; /* Skip the token */
        while (*pos && *pos++ == ' ') ; /* Skip the spaces */
        pos--;                          /* Set to beginning of the next token */
      }
      goto error;
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
      if (word->text_code[0] == 0x01) {
        DBG("Variable:   %p\n", word->location);
        _push((void *) &word->location, sizeof(POINTER_TYPE));
        goto next;
      }
      DBG("Word:   %s\n", token);
      if (word->location) {
        DBG("Exec:   %s word from %p\n", token, word->location);
        word->function();
        goto next;
      }
      else {
        DBG("Eval:   %s word from '%s'\n", token, word->text_code);
        eval(dict, word->text_code, NULL);
        goto next;
      }
    }
    /* VARIABLE */
    if ((strcmp(token, VARIABLE_TOKEN)) == 0) {
      DBG("Variable token! %d\n", 0);
      const char* pos = begin + strlen(VARIABLE_TOKEN);
      while (*pos && *pos++ == ' ') ;
      const char* name_start = --pos;
      while (*pos && *pos++ != ' ') ; /* Skip the token */
      const char* name = strndup(name_start, pos - name_start);
      create_variable(name);
      free((void *) name);
      end = pos;
      goto next;
    }
  error:
    DBG("Can't parse:   %s!\n", token);
    free(token);
    break;
  next:
    free(token);
    if (*begin == 0 || (line_end ? begin > line_end : 0))
      break;
    begin = end;
  }
}
