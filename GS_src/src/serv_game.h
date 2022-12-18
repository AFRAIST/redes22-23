#pragma once
#include "rcwg.h"

typedef struct __attribute__((packed)) {
    char letter;
} LetterGuess;

typedef struct __attribute__((packed)) {
    const char *cur_word;
    LetterGuess letter_guess[40];
    u32 errors;
} ServGame;

void StartGame();
Result GameAcquire(size_t plid);
Result ExitAndSerializeGame();
Result GameRelease();
void RegisterLetterTrial();
void RegisterWordGuess();
const char *GetCurWord();

