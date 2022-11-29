#pragma once
#include "rcwg.h"

Result command_start(struct output *inp);
Result command_play(struct output *inp);
Result command_guess(struct output *inp);
Result command_scoreboard(struct output *inp);
Result command_hint(struct output *inp);
Result command_state(struct output *inp);
Result command_quit(struct output *inp);
Result command_exit(struct output *inp);
