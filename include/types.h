#pragma once
#include "includes.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef ssize_t Result;

struct input {               // Structure declaration
    char *command;           /*[11]*/
    char *appendix; /*[31]*/ // Member (int variable)
    size_t plid;
};

struct output { // Structure declaration
    u8 gap_x30[0x30];
    char *buff;
};
