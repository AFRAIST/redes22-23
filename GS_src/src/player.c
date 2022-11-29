#include "player.h"

/* Singleton. */
Player g_players[PLAYER_AMOUNT];

void init_players(Player *players, size_t size) {
    Player *const top = players + size;
    for (; players < top; ++players) {
        players->sock_fd = -1;
        players->player_port = players->player_ip = NULL;
    }
}
