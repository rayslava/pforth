#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PF_SUCCESS 0

/**
   The dictionary will be increased by this value when not enough memory
   for new words
 */
#define DICTIONARY_INCREASE_STEP 32

/**
 * The single FORTH word description
 */
struct _pforth_word {
  uint8_t size;        /**< Code size */
  void*   location;    /**< Binary location to jump to */
};

typedef struct _pforth_word pforth_word;
typedef pforth_word * pforth_word_ptr;

extern pforth_word_ptr* dictionary;
extern uint32_t dictionary_length;

extern uint8_t* data_stack_top;
extern void* return_stack_top;

void pforth_init();

/**
   djb2 hash function

   Hope it'll be more or less unique to create hash table for FORTH words

   \param str null-terminated input string
   \returns uint32_t hash
 */
uint32_t hash(const unsigned char* str);
