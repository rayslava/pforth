#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "pforth.h"

/**
   Create a new hash table dictionary.

   \param size number of elements
   \return pointer to created dictionary
 */
dict_t* dict_create(uint32_t size) {
  dict_t* dict = NULL;

  if (size < 1) return NULL;

  /* Allocate the table */
  if ((dict = malloc(sizeof(dict_t))) == NULL) {
    return NULL;
  }

  /* Allocate pointers to the head nodes. */
  if ((dict->table = malloc(sizeof(dict_entry *) * size)) == NULL) {
    free(dict);
    return NULL;
  }

  /* Clean up the bins */
  for (uint32_t i = 0; i < size; i++) {
    dict->table[i] = NULL;
  }

  dict->size = size;

  return dict;
}

void dict_free(dict_t* dict, uint32_t size) {
  for (uint32_t i = 0; i < size; ++i) {
    dict_entry* head = dict->table[i];
    while (head) {
      DBG("Freeing %20s @%p\n", head->key, (void *) head);
      dict_entry* next = head->next;
      pforth_word_free(head->word);
      free(head->key);
      free(head);
      head = next;
    }
  }
  free(dict->table);
}

/**
   djb2 hash function

   Hope it'll be more or less unique to create hash table for FORTH words

   \param hashtable pointer to hash table
   \param key null-terminated key string
   \returns index in hash table
 */
int hash(dict_t* hashtable, const char* key) {

  unsigned long int hash = 5381;
  int c;

  while ((c = *key++))
    hash = ((hash << 5) + hash) + c;

  return hash % hashtable->size;
}

/**
   Create a key-value pair for dictionary.

   Internal function to be used only inside dictionary.

   \param key the FORTH word in ASCII
   \param value pointer to the actual pforth_word structure
   \return newly allocated key-value pair
 */
dict_entry* dict_newkv(const char* key, const pforth_word_ptr value) {
  dict_entry* newpair;

  if ((newpair = malloc(sizeof(dict_entry))) == NULL) {
    return NULL;
  }

  if ((newpair->key = calloc(1, strlen(key) + 1)) == NULL) {
    free(newpair);
    return NULL;
  }
  strcpy(newpair->key, key);

  if ((newpair->word = pforth_word_alloc()) == NULL) {
    free(newpair->key);
    free(newpair);
    return NULL;
  }

  if (value)
    pforth_word_copy(newpair->word, value);
  else
    newpair->word = pforth_word_alloc();

  newpair->next = NULL;

  return newpair;
}

/**
   Insert word into dictionary.

   The word will be \b copied into the dictionary. The \p value must be freed in
   caller afterwards.

   \param dict dictionary to use
   \param key FORTH word in ASCII form
   \param value pointer to pforth_word struct with the word.
   \return pointer to the newly created word
 */
pforth_word_ptr dict_set(dict_t* dict, const char* key, const pforth_word_ptr value) {
  int bin = 0;
  dict_entry* next = NULL;
  dict_entry* last = NULL;

  bin = hash(dict, key);

  next = dict->table[bin];

  while (next != NULL && next->key != NULL && strcmp(key, next->key) > 0) {
    /* Look for the key in this bin */
    last = next;
    next = next->next;
  }

  if (next != NULL && next->key != NULL && strcmp(key, next->key) == 0) {
    /* Found the key in the bin, replacing */
    pforth_word_free(next->word);
    next->word = pforth_word_alloc();
    pforth_word_copy(next->word, value);
    return next->word;
  } else {
    /* Not found: creating */
    dict_entry* new_item = dict_newkv(key, value);

    if (next == dict->table[bin]) {
      /* Creating the first item in the bin */
      new_item->next = next;
      dict->table[bin] = new_item;
    } else if (next == NULL && last != NULL) {
      /* Creating the last item */
      last->next = new_item;
    } else {
      /* Inserting item in the middle. */
      new_item->next = next;
      last->next = new_item;
    }
    return new_item->word;
  }
}

/**
   Retrieve the word from the dictionary.

   \param dict dictionary to use
   \param key FORTH word in ASCII form
   \return pointer to word \b inside the dictionary. Do not harm it.
 */
pforth_word_ptr dict_get(dict_t* dict, const char* key) {
  int bin = 0;
  dict_entry* pair;

  bin = hash(dict, key);

  /* Step through the bin, looking for our value. */
  pair = dict->table[bin];
  while (pair != NULL && pair->key != NULL && strcmp(key, pair->key) > 0) {
    pair = pair->next;
  }

  /* Did we actually find anything? */
  if (pair == NULL || pair->key == NULL || strcmp(key, pair->key) != 0) {
    return NULL;
  } else {
    return pair->word;
  }
}
