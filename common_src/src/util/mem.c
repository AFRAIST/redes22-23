#include "rcwg.h"

char *StrNSplitSpaceNext(char *s, size_t bounds) {
    if (BRANCH_UNLIKELY(s == NULL))
        return NULL;

    char *const top = s + bounds;

    for (; s <= top && *s != ' ' && *s != '\n' && *s != '\t'; ++s)
        ;

    if (s > top)
        return NULL;

    *s = '\0';

    return s + (1 * sizeof(char));
}

Result BufNotContainsInvalidNull(char *b, size_t sz) {
    if (BRANCH_UNLIKELY(b == NULL || sz < 2))
        return EXIT_SUCCESS;

    for (size_t i = 0; i < (sz - 1); ++i)
        if (b[i] == '\0' && b[i + 1] != '\0')
            return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

Result BufNotContainsNull(char *b, size_t sz) {
    for (size_t i = 0; i < sz; ++i)
        if (b[i] == '\0')
            return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

char *BufTokenizeOpt(char *b, const char *delim, char **next) {
    char *end;

    end = b + strcspn(b, delim);
    if (*end == '\x00') {
        *next = end;
        return b;
    }

    *end = '\x00';
    *next = end + 1;
    return b;
}

Result BufTokenizeOpts(char *b, char **opts, size_t sz) {
    if (BufNotContainsNull(b, sz) == EXIT_FAILURE || !sz || b[sz - 1] != '\n' ||
        *b == '\0')
        return EXIT_FAILURE;

    b[sz - 1] = '\0';

    char *const top = b + sz - 1;

    size_t j = 0;
    char *cur = b;
    for (; b < top; ++b) {
        if (*b == ' ') {
            if ((b + 1) >= top || *(b + 1) == ' ' || j >= (sz - 1))
                return EXIT_FAILURE;

            *b = 0;
            opts[j++] = cur;
            cur = b + 1;
        }

        if (*b == '\n')
            return EXIT_FAILURE;
    }

    opts[j] = cur;

    return EXIT_SUCCESS;
}

bool all_digits(const char *s) {
    for (; *s; s++)
        if (*s < '0' || *s > '9')
            return false;

    return true;
}

void str_replace(char *s, char old, char new) {
    for (; *s; ++s)
        if (*s == old)
            *s = new;
}

Result strtoul_check(ssize_t *out, const char *s) {
    if (!all_digits(s))
        return EXIT_FAILURE;

    *out = strtol(s, NULL, 10);

    if (errno != 0 || *out < 0)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
