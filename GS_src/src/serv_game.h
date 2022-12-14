#pragma once
#include "rcwg.h"

typedef struct {
    char *cur_word;
    u32 transactions;
    u32 errors;
    u8 _gap[200];
} ServGame;
