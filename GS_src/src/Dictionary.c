#include "Dictionary.h"

static const char delim[] = {' ', '\n'};
static const size_t minimal_size = sizeof("a a\n") / sizeof(char);

void InitDictionary(Dictionary *dict, char *data, size_t size) {
    /* Index data. */
    u32 counter = 0;
    const size_t alloc_size = size / minimal_size;

    dict->entries =
        (DictionaryEntry *)malloc(alloc_size * sizeof(DictionaryEntry));

    char cc = delim[0];
    size_t amt = 0;
    for (size_t i = 0; i != size; ++i, ++counter) {
        if (cc == delim[0]) {
            /* Keep checking word validity. */
            R_EXIT_IF(BRANCH_UNLIKELY(data[i] == delim[1] || counter > 30),
                      invalid_word_size_error);

            if (data[i] == cc) {
                /* Assert size again. */
                R_EXIT_IF(BRANCH_UNLIKELY(counter < 3), "Invalid word size!");

                cc = delim[1];
                dict->entries[amt].word = &data[i - counter];

                counter = data[i] = 0;
            }
        } else /*if (cc == delim[1])*/ {
            R_EXIT_IF(BRANCH_UNLIKELY(data[i] == delim[0]),
                      invalid_delimiter_error);

            if (data[i] == cc) {
                cc = delim[0];
                dict->entries[amt++].word_class = &data[i - counter];

                counter = data[i] = 0;
            }
        }
    }

    R_EXIT_IF(BRANCH_UNLIKELY(cc != delim[0] || data[size] != cc),
              "Invalid dictionary last line!");
}
