#include "rcwg.h"

bool is_verbose = false;

NOINLINE void VerbosePrintF(const char *format, ...) {
    if (BRANCH_UNLIKELY(is_verbose)) {
        va_list argptr;
        va_start(argptr, format);

        vprintf(format, argptr);

        va_end(argptr);
    }
}
