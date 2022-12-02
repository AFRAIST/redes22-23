#include "rcwg.h"

typedef struct {
    char word[WORD_LETTERS+1];
    u32 attempts;
} Game;

extern Game g_game;

void game_init(Game *g, u32 n_letters, u32 n_errors);