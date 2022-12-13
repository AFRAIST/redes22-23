#pragma once

#include "rcwg.h"

extern FILE *g_file_dat;

void acquire_player_file(size_t plid);
void release_player_file();
