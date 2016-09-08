#include <string.h>
#include "pforth.h"

uint8_t* data_stack_top; /**< The global data stack top pointer for the FORTH machine */
void* return_stack_top;

pforth_word_ptr pforth_word_alloc() {
  pforth_word_ptr new_word = malloc(sizeof(pforth_word));
  new_word->text_code = NULL;
  new_word->location = NULL;
  new_word->size = 0;
  return new_word;
}

void pforth_word_free(const pforth_word_ptr word) {
  if (word->location && word->text_code != NULL)
    free(word->location);
  free(word->text_code);
  free(word);
}

void pforth_init() {
  data_stack_top = malloc(sizeof(uint8_t) * 1024);
  return_stack_top = malloc(sizeof(uint8_t) * 1024);
}

void pforth_deinit() {
  free(data_stack_top);
  free(return_stack_top);
}

pforth_word_ptr pforth_word_copy(const pforth_word_ptr dest, const pforth_word_ptr src) {
  dest->size = src->size;
  /* Created word */
  if (src->text_code) {
    const int text_len = strlen(src->text_code);
    dest->text_code = calloc(1, text_len + 1);
    if (dest->text_code == NULL) {
      return NULL;
    }
    memcpy(dest->text_code, src->text_code, text_len);
    if (dest->size) {
      dest->location = malloc(dest->size);
      if (dest->location == NULL) {
        free(dest->text_code);
        return NULL;
      }
      memcpy(dest->location, src->location, dest->size);
    }
  } else {
    /* Native word */
    dest->text_code = NULL;
    dest->location = src->location;
  }
  return dest;
}
