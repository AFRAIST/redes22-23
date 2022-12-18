#pragma once
#include "rcwg.h"

typedef struct {
    const char *word;
    const char *word_class;
} DictionaryEntry;

typedef struct {
    union {
        DictionaryEntry *entries;
        u8 *buffer;
    };
    size_t amt;
} Dictionary;

extern Dictionary dict_instance;

#define DICT_INVALID_WORD_SIZE_ERROR "Dictionary: Invalid word size!"
#define DICT_INVALID_DELIMITER_ERROR "Dictionary: Invalid delimiter formatting!"
#define DICT_INVALID_LAST_LINE "Dictionary: Invalid last line!"
#define DICT_INVALID_CHARACTER "Dictionary: Invalid character!"

size_t random_entry(Dictionary *dict);
void InitDictionary(Dictionary *dict, FILE *fp);
void FiniDictionary(Dictionary *dict);
