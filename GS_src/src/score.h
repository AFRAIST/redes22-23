#pragma once
#include "rcwg.h"

#define TOP_SCORE 10
#define PATH_SIZE 256
#define SCORE_STR_SIZE 256
#define DATE_SIZE 64

typedef struct {
    char score_str[256];
} ScoreEntry;

extern u32 total_scores;

int count_scores();
Result save_score(struct output *outp, const char* word);
Result get_scoreboard(ScoreEntry* scoreboard_list);


