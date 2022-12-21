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
    char *word_state;
    /* LetterGuess can be arrayd out. */
    LetterGuess letter_guess[40];
    WordGuess word_guess[40];
    u32 max_errors;
    /* Error counter. */
    u32 errors;
    /* Number of games. */
    u32 trials;
    /* Then, we'll have to array wordguess too.*/
    bool finished;
} ServGame;

extern ServGame *g_serv_game;

Result StartGame();
Result GameAcquire(size_t plid);
Result ExitAndSerializeGame();
bool GameHasMoves();
u32 GameRegTrial();
u32 GameTrials();
Result GameRelease();
void RegisterLetterTrial();
void RegisterWordGuess();
const char *GetCurWord();
char *GetCurRepr();
char *StrSerializeDup(const char *p);
Result GameEmpty(bool *out);
