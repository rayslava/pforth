#include "pforth.h"

pforth_word_ptr* dictionary;
uint32_t dictionary_size;
uint32_t dictionary_length;

uint8_t* data_stack_top; /**< The global data stack top pointer for the FORTH machine */
void* return_stack_top;

pforth_word_ptr create_word(uint8_t size) {
  pforth_word_ptr new_word = malloc(sizeof(pforth_word));
  new_word->location = malloc(size);
  dictionary[dictionary_length] = new_word;
  dictionary_length++;
  return new_word;
}

int destroy_word(uint16_t id) {
  pforth_word_ptr word = dictionary[id];
  free(word->location);
  free(word);
  return PF_SUCCESS;
}

int increase_dictionary() {
  dictionary_size += DICTIONARY_INCREASE_STEP;
  dictionary = realloc(dictionary, dictionary_size);
  return dictionary_size;
}

void pforth_init() {
  data_stack_top = malloc(sizeof(uint8_t) * 1024);
  return_stack_top = malloc(sizeof(uint8_t) * 1024);
}
