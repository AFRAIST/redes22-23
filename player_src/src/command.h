#pragma once
#include "rcwg.h"

Result command_start(struct input *inp);
Result command_play(struct input *inp);
Result command_guess(struct input *inp);
Result command_scoreboard(struct input *inp);
Result command_hint(struct input *inp);
Result command_state(struct input *inp);
Result command_quit(struct input *inp);
Result command_exit(struct input *inp);
