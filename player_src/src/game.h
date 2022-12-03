#include "rcwg.h"

typedef struct {
    char word[WORD_LETTERS + 1];
    size_t plid;
    u32 attempts;
    bool is_active;
} Game;

extern Game g_game;

void game_init(Game *g, size_t plid, u32 n_letters, u32 n_errors);
void game_fini(Game *g);
