#pragma once

#define NORETURN __attribute__((noreturn))
#define ALWAYS_INLINE inline __attribute__((always_inline))
#define NOINLINE __attribute__((noinline))

#define BITSIZEOF(x) (sizeof(x) << 3)
#define STR_SIZEOF(s) (sizeof(s) - 1)

#define BRANCH_PREDICT(expr, value, _probability)                              \
    __builtin_expect_with_probability(expr, value, _probability)

#define BRANCH_PREDICT_TRUE(expr, probability)                                 \
    BRANCH_PREDICT(!!(expr), 1, probability)
#define BRANCH_PREDICT_FALSE(expr, probability)                                \
    BRANCH_PREDICT(!!(expr), 0, probability)

#define BRANCH_LIKELY(expr) BRANCH_PREDICT_TRUE(expr, 1.0)
#define BRANCH_UNLIKELY(expr) BRANCH_PREDICT_FALSE(expr, 1.0)

#define R_ABORT(message)                                                       \
    ({                                                                         \
        perror(message);                                                       \
        abort();                                                               \
    })

#define R_ABORT_IF(expr, message)                                              \
    if (expr) {                                                                \
        R_ABORT(message);                                                      \
    }

#define R_NOT_IMPLEMENTED_MSG(msg) ({ R_ABORT(msg); })

#define R_NOT_IMPLEMENTED() R_NOT_IMPLEMENTED_MSG("Not implemented!")

#define R_EXIT(res, message)                                                   \
    ({                                                                         \
        perror(message);                                                       \
        exit(res);                                                             \
    })

#define R_FAIL_RETURN(res, cond, message)                                      \
    if (cond) {                                                                \
        perror(message);                                                       \
        return (res);                                                          \
    }

#define R_EXIT_IF(res, expr, message)                                          \
    if (expr) {                                                                \
        R_EXIT(res, message);                                                  \
    }

#define R_FAIL_EXIT_IF(expr, message) R_EXIT_IF(EXIT_FAILURE, expr, message)

#define COND_COMP_STRINGS_2(command, str1, str2)                               \
    (strcmp(command, str1) == 0 || strcmp(command, str2) == 0)

#define COND_COMP_STRINGS_1(command, str1) (strcmp(command, str1) == 0)

#define INIT_INPUT(X) struct input X = {.plid_exists = false}

#include "defines_specific.h"
