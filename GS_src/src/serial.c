#include "serial.h"

FILE *g_file_dat = NULL;

void serialize_current_game() {}

void acquire_player_file(size_t plid) {
    char dat[19];
    sprintf(dat, "sv_data/DAT_%06zu", plid);

    /* Check if file exists. */
    if (access(dat, F_OK) == 0) {
        printf("%s exists.\n", dat);
    } else {
        printf("%s does not exist.\n", dat);
    }

    /* Change later. */
    g_file_dat = fopen(dat, "w+b");
    R_FAIL_EXIT_IF(flock(fileno(g_file_dat), LOCK_EX) == -1, E_ACQUIRE_ERROR);
}

void release_player_file() {
    R_FAIL_EXIT_IF(flock(fileno(g_file_dat), LOCK_UN) == -1, E_ACQUIRE_ERROR);
}