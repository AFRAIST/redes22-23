#pragma once

#define NORETURN __attribute__((noreturn))
#define ALWAYS_INLINE_LAMBDA __attribute__((always_inline))
#define ALWAYS_INLINE inline __attribute__((always_inline))
#define NOINLINE __attribute__((noinline))

#define BITSIZEOF(x) (sizeof(x) << 3)

#define DEFAULT_PORT "58028"
#define DEFAULT_IP "127.0.0.1"

#define BRANCH_PREDICT(expr, value, _probability)                              \
    __builtin_expect_with_probability(expr, value, _probability)

#define BRANCH_PREDICT_TRUE(expr, probability)                                 \
    BRANCH_PREDICT(!!(expr), 1, probability)
#define BRANCH_PREDICT_FALSE(expr, probability)                                \
    BRANCH_PREDICT(!!(expr), 0, probability)

#define BRANCH_LIKELY(expr) BRANCH_PREDICT_TRUE(expr, 1.0)
#define BRANCH_UNLIKELY(expr) BRANCH_PREDICT_FALSE(expr, 1.0)

#define R_EXIT_IF(expr, message)                                               \
    if (expr) {                                                                \
        fprintf(stderr, message);                                              \
        exit(EXIT_FAILURE);                                                    \
    }
