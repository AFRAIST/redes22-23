#pragma once

#include "rcwg.h"

typedef struct {
    int sock_fd;
    char *player_ip;
    char *player_port;
} Player;

extern Player g_players[PLAYER_AMOUNT];

void init_players(Player *players, size_t size);
