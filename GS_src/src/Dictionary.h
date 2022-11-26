#pragma once
#include "rcwg.h"

typedef struct {
    const char *word;
    const char *word_class;
} DictionaryEntry;

typedef struct {
    DictionaryEntry *entries;
} Dictionary;

const char *const invalid_word_size_error = "Dictionary: Invalid word size!";
const char *const invalid_delimiter_error =
    "Dictionary: Invalid delimiter formatting!";

void InitDictionary(Dictionary *dict, char *data, size_t size);
