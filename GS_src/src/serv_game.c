#include "serv_game.h"
#include "Dictionary.h"

static __attribute__((aligned(1024))) u8 g_file_arena[0x1000];
ServGame *g_serv_game = (ServGame *)((u8 *)g_file_arena+8);

int g_file_dat = 0;

Result GameAcquire(size_t plid) {
    char dat[19];
    sprintf(dat, "sv_data/DAT_%06zu", plid);

    /* Check if file exists. */
    if (access(dat, F_OK) == 0) {
        VerbosePrintF("%s exists.\n", dat);
        
        g_file_dat = open(dat, O_RDWR, 0644);
    } else {
        VerbosePrintF("%s does not exist.\n", dat);
    
        g_file_dat = open(dat, O_RDWR | O_CREAT, 0644);
    }

    R_FAIL_EXIT_IF(flock(g_file_dat, LOCK_EX) == -1, E_ACQUIRE_ERROR);
    return EXIT_SUCCESS;
}

Result GameRelease() {
    R_FAIL_EXIT_IF(flock(g_file_dat, LOCK_UN) == -1, E_ACQUIRE_ERROR);
    return EXIT_SUCCESS;
}

void StartGame() {
    memset(g_serv_game, 0, sizeof(ServGame));
    *(u64 *)&g_file_arena = sizeof(ServGame);

    g_serv_game->cur_word = random_word(&dict_instance);

    const u64 reg = *(u64 *)&g_file_arena;
    *(u64 *)&g_file_arena += strlen(g_serv_game->cur_word);
   
    strcpy((char *)g_file_arena + reg, g_serv_game->cur_word);
    g_serv_game->cur_word = (char *)g_file_arena + reg;
}

Result ExitAndSerializeGame() {
    g_serv_game->cur_word = (const char*)((const char*)(g_serv_game->cur_word) - ((const char*)g_file_arena+8));
    R_FAIL_RETURN(EXIT_FAILURE, lseek(g_file_dat, 0, SEEK_SET), "[ERROR] Failed to seek file.\n");

    const u64 sz = *(u64 *)&g_file_arena;

    Result rc = EXIT_SUCCESS;

    if (write(g_file_dat, g_serv_game, sz) != (ssize_t)sz) {
        perror("[ERROR] Failed to write serialization file.\n");
        rc = EXIT_FAILURE;
    }

    else if (ftruncate(g_file_dat, sz) == -1) {
        perror("[ERROR] Failed to truncate serialization file.\n");
        rc = EXIT_FAILURE;
    }
    
    if (close(g_file_dat) == -1) {
        perror("[ERROR] Failed to close serialization file.\n");
        rc = EXIT_FAILURE;
    }
    
    g_file_dat = 0;
    return rc;
}

const char *GetCurWord() {
    return g_serv_game->cur_word;
}

void RegisterLetterTrial() {
}

void RegisterWordGuess() {

}

