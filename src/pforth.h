#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>


#ifdef DEBUG
#define DBG(fmt, ...) fprintf(stderr, "%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__);
#else
#define DBG(fmt, ...) {}
#endif

#define PRINT(fmt, ...) fprintf(stdout, fmt, __VA_ARGS__);

#define LO(x) ((unsigned char) ((x) & 0xFF))
#define HI(x) ((unsigned char) (((x) >> 8) & 0xFF))

/**
   Maximal dictionary size
 */
#define FORTH_DICT_SIZE 1000

/**
   Default data stack type definition
 */
#define FORTH_TYPE int32_t

/**
   Comma separated list of types to generate math operations for
 */
#define INT_TYPE_LIST FORTH_TYPE

/**
   Type of function to be called from the word.
 */
typedef void (* word_function)();

/**
   The single FORTH word description
 */
struct _pforth_word {
  /**
     ASCII representation of word.

     The actual program.
     \p NULL for native words.
   */
  char* text_code;
  uint8_t size;               /**< Code size (for binary code in \p location/\p function) */
  union {
    void* location;           /**< Binary location to jump to. NULL if not compiled  */
    word_function function;   /**< Function to call when compiling the word */
  };
};

typedef struct _pforth_word pforth_word;
typedef pforth_word * pforth_word_ptr;

extern uint8_t* data_stack_top;
extern void* return_stack_top;

struct _dict_entry {
  char* key;
  pforth_word_ptr word;
  struct _dict_entry* next;
};

typedef struct _dict_entry dict_entry;

struct dict_s {
  int size;
  struct _dict_entry** table;
};

typedef struct dict_s dict_t;

dict_t* dict_create(uint32_t size);
void dict_free(dict_t* dict, uint32_t size);
void dict_set(dict_t* dict, const char* key, const pforth_word_ptr value);
pforth_word_ptr dict_get(dict_t* dict, const char* key);

extern dict_t* forth_dict;

/**
   Init the FORTH machine.

   Allocate stacks, create dictionary.
 */
void pforth_init();

/**
   Free the FORTH machine internal structures.

   Deallocate stacks, remove dictionary.
 */
void pforth_deinit();

/**
   Copy the word from \p src to \p dest

   The \p dest must be allocated beforehand.

   \param src source word
   \param dest destination buffer
   \return pointer to the buffer (equal to \p dest)
   \return \p NULL if not enough memory
 */
pforth_word_ptr pforth_word_copy(const pforth_word_ptr dest, const pforth_word_ptr src);

/**
   Create a duplicate of the word \p src

   The memory is allocated inside and must be freed in caller

   \param src source word
   \return pointer to new word
   \return \p NULL if not enough memory
 */
pforth_word_ptr pforth_word_dup(const pforth_word_ptr src);

/**
   Allocate memory for new word

   \return pointer to new word
   \return \p NULL if not enough memory
 */
pforth_word_ptr pforth_word_alloc();

/**
   Free the word structure

   \param word the word to free
 */
void pforth_word_free(const pforth_word_ptr word);

/**
   Call the word function.
 */
#define PFORTH_WORD_RUN(w) { word_function f = *(word->function); f(); }

/**
    Preprocess the FORTH line.

    The preprocessing is done in-place, comments and newlines are removed by
    replacing with space characters. There's no space compression pass - it's
    easier to do that right in eval().

    \param line the line to be preprocessed.
 */
int preprocess(char* line);

/**
   Evaluate the FORTH line

   \param dict the dictionary to use
   \param line the FORTH line
 */
void eval(dict_t* dict, const char* line);

void register_native(const char* op, word_function f);

#include "words.h"
