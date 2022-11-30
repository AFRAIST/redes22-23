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

Result BufContainsInvalidNull(char *b, size_t sz) {
    if (BRANCH_UNLIKELY(b == NULL || sz < 2))
        return EXIT_SUCCESS;
 
    for(u32 i = 0; i < (sz - 1); ++i)
        if(b[i] == '\0' && b[i+1] != '\0')
            return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
