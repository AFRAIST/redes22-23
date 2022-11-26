#include "rcwg.hpp"

namespace rcwg::util {
NOINLINE void io::PrintF(const char *format, ...) {
    va_list argptr;
    va_start(argptr, format);

    std::vprintf(format, argptr);

    va_end(argptr);
}

NOINLINE void io::VerbosePrintF(const char *format, ...) {
    if (BRANCH_UNLIKELY(is_verbose)) {
        va_list argptr;
        va_start(argptr, format);

        std::vprintf(format, argptr);

        va_end(argptr);
    }
}
} // namespace rcwg::util
