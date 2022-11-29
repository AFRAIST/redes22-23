#include "rcwg.h"

char *StrNSplitSpaceNext(char *s, size_t bounds) {
    if (BRANCH_UNLIKELY(s == NULL))
        return NULL;

    char *const top = s + bounds;

    for (; s < top && *s != ' ' && *s != '\n' && *s != '\t'; ++s)
        ;

    if (s >= top)
        return NULL;

    *s = '\0';

    return s + (1 * sizeof(char));
}
