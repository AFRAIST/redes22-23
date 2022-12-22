#include "Dictionary.h"

Dictionary dict_instance;

static const char delim[] = {' ', '\n'};
static const size_t minimal_size = sizeof("a a\n") / sizeof(char);

static inline void ParseDataFromFile(Dictionary *dict, FILE *fp, char **data,
                                     size_t *size) {
    fseek(fp, 0, SEEK_END);
    *size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    const size_t alloc_size = *size / minimal_size;

    /* This inits everything! Dangerous! */
    /* ENTRIES + DATA! */
    dict->buffer = (u8 *)malloc(alloc_size * sizeof(DictionaryEntry) +
                                (*size + 1) * sizeof(char));

    *data = (char *)(dict->buffer + (alloc_size * sizeof(DictionaryEntry)));

    /* Read all text! */
    fread(*data, 1, *size, fp);

    if ((*data)[*size - 1] != '\n')
        (*data)[(*size)++] = '\n';

    /* We don't need it anymore! */
    fclose(fp);
    fp = NULL;
}

size_t random_entry(Dictionary *dict) {
    #ifndef FOR_TEST
    return rand() % dict->amt;
    #else
    u32 cur_id;
    R_FAIL_EXIT_IF(flock(g_shm_fd, LOCK_EX) == -1, "[ERROR] Shared memory flock.\n");
    u32 *shmptr = (u32*)shmat(g_shmid, NULL, 0);
    cur_id = *shmptr;
    *shmptr = *shmptr + 1;
    msync(shmptr, 0x1000, MS_SYNC);
    munmap(shmptr, 0x1000);
    R_FAIL_EXIT_IF(flock(g_shm_fd, LOCK_UN) == -1, "[ERROR] Shared memory flock.\n");
    return cur_id % dict->amt; 
    #endif
}

void InitDictionary(Dictionary *dict, FILE *fp) {
    char *data;
    size_t size;
    ParseDataFromFile(dict, fp, &data, &size);

    /* Index data. */
    u32 counter = 0;

    char cc = delim[0];
    size_t amt = 0;
    for (size_t i = 0; i != size; ++i) {
        R_FAIL_EXIT_IF(BRANCH_UNLIKELY(data[i] == '\x00' || data[i] == 0x1A),
                       DICT_INVALID_CHARACTER);
        if (cc == delim[0]) {
            /* Keep checking word validity. */
            R_FAIL_EXIT_IF(BRANCH_UNLIKELY(data[i] == delim[1] || counter > 30),
                           DICT_INVALID_WORD_SIZE_ERROR);

            if (data[i] == cc) {
                /* Assert size again. */
                R_FAIL_EXIT_IF(BRANCH_UNLIKELY(counter < 3),
                               DICT_INVALID_WORD_SIZE_ERROR);

                cc = delim[1];
                dict->entries[amt].word = &data[i - counter];

                counter = data[i] = 0;
                continue;
            }
        } else /*if (cc == delim[1])*/ {
            R_FAIL_EXIT_IF(BRANCH_UNLIKELY(data[i] == delim[0]),
                           DICT_INVALID_DELIMITER_ERROR);

            if (data[i] == cc) {
                cc = delim[0];
                dict->entries[amt++].word_class = &data[i - counter];

                counter = data[i] = 0;
                continue;
            }
        }

        ++counter;
    }

    R_FAIL_EXIT_IF(BRANCH_UNLIKELY(cc != delim[0] || data[size - 1] != '\x00'),
                   DICT_INVALID_LAST_LINE);
    dict->amt = amt;
}

void FiniDictionary(Dictionary *dict) {
    if (dict->buffer != NULL) {
        free(dict->buffer);
        dict->buffer = NULL;
    }
}
