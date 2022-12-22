#pragma once
#include "../content.h"

extern bool is_verbose;

NOINLINE void VerbosePrintF(const char *format, ...)
    __attribute__((format(printf, 1, 2)));
ALWAYS_INLINE void SetVerbose() { is_verbose = true; }
