#pragma once
#include "../content.hpp"

namespace rcwg::util {
namespace {
static inline constinit bool is_verbose = false;
}

namespace io {
NOINLINE void PrintF(const char *format, ...)
    __attribute__((format(printf, 1, 2)));
NOINLINE void VerbosePrintF(const char *format, ...)
    __attribute__((format(printf, 1, 2)));
ALWAYS_INLINE void SetVerbose() { is_verbose = true; }

} // namespace io

} // namespace rcwg::util
