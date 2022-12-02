#include "game.h"

Game g_game;

void game_init(Game *g, u32 n_letters, u32 n_errors) {
    memset(g->word, '_', n_letters);
    g->word[n_letters] = '\0';
    g->attempts = n_errors;
}
