#pragma once
#include "rcwg.h"

typedef struct __attribute__((packed)) {
    char letter;
} LetterGuess;

typedef struct __attribute__((packed)) {
    char *word;
} WordGuess;

typedef struct __attribute__((packed)) {
    /* This way, we can point to the entry without memlayout concerns. */
    size_t cur_entry;
    /* LetterGuess can be arrayd out. */
    LetterGuess letter_guess[40];
    WordGuess word_guess[40];
    /* Error counter. */
    u32 errors;
    /* Then, we'll have to array wordguess too.*/
} ServGame;

Result StartGame();
Result GameAcquire(size_t plid);
Result ExitAndSerializeGame();
bool GameHasMoves();
Result GameRelease();
void RegisterLetterTrial();
void RegisterWordGuess();
const char *GetCurWord();
