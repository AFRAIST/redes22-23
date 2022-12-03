#include "game.h"

Game g_game;

void game_init(Game *g, size_t plid, u32 n_letters, u32 n_errors) {
    memset(g->word, '_', n_letters);
    g->word[n_letters] = '\0';
    g->attempts = n_errors;
    g->plid = plid;
    g->is_active = true;
}

void game_fini(Game *g) { g->is_active = false; }
