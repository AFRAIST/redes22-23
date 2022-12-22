#include "serv_game.h"
#include "Dictionary.h"

static __attribute__((aligned(1024))) u8 g_file_arena[0x2000];
ServGame *g_serv_game = (ServGame *)((u8 *)g_file_arena + 8);

int g_file_dat = 0;
bool exists;

Result GameAcquire(size_t plid) {
    char dat[19];

    const char *dir = "sv_data";
    if (access(dir, F_OK) == 0) {
        VerbosePrintF("Directory %s exists\n", dir);
    } else {
        if(mkdir(dir, S_IRWXU) != 0) perror(E_SERVER_ERROR);
    }

    sprintf(dat, "sv_data/DAT_%06zu", plid);

    /* Check if file exists. */
    if (access(dat, F_OK) == 0) {
        VerbosePrintF("%s exists.\n", dat);

        g_file_dat = open(dat, O_RDWR, 0644);
        
        if (g_file_dat == -1) {
            perror("Open.\n");
        }

        if (GameEmpty(&exists) == -1)
            return EXIT_FAILURE;

        exists = !exists;
    } else {
        VerbosePrintF("%s does not exist.\n", dat);

        g_file_dat = open(dat, O_RDWR | O_CREAT, 0644);
        
        if (g_file_dat == -1) {
            perror("Open.\n");
        }

        exists = false;
    }

    R_FAIL_RETURN(EXIT_FAILURE, flock(g_file_dat, LOCK_EX) == -1, E_ACQUIRE_ERROR);
    return EXIT_SUCCESS;
}

Result GameEmpty(bool *out) {
    struct stat sb;

    if (fstat(g_file_dat, &sb) == -1) {
        perror("Fstat.\n");
        return EXIT_FAILURE;
    }

    *out = sb.st_size == 0;
    return EXIT_SUCCESS;
}

u32 GameRegTrial() {
    return ++g_serv_game->trials;
}

u32 GameTrials() {
    return g_serv_game->trials;
}

Result GameRelease() {
    R_FAIL_RETURN(EXIT_FAILURE, flock(g_file_dat, LOCK_UN) == -1, E_ACQUIRE_ERROR);
    return EXIT_SUCCESS;
}

Result StartGame() {
    if (!exists) {
        memset(g_serv_game, 0, sizeof(ServGame));
        u64 c_sz;
        c_sz = *(u64 *)&g_file_arena = sizeof(ServGame);

        g_serv_game->cur_entry = random_entry(&dict_instance);
        const u32 s_sz = strlen(GetCurWord());
        g_serv_game->word_state = (char*)g_serv_game+c_sz;
        
        /* Fill with empty. */
        memset(g_serv_game->word_state, '-', s_sz);
        *(g_serv_game->word_state+s_sz) = '\x00';
        *(u64 *)&g_file_arena += s_sz+1;
    } else {
        const ssize_t rd = read(g_file_dat, g_serv_game, 0x1000);

        R_FAIL_RETURN(EXIT_FAILURE, rd == -1 || rd == 0, "[ERROR] Failed to read serial file.\n");
        *(u64 *)&g_file_arena = rd;
    
        /* Rebase pointers. */
        g_serv_game->word_state = (char *)g_serv_game->word_state + (size_t)g_serv_game;
        for(u32 i = 0; i < 40 && g_serv_game->word_guess[i].word != 0; ++i) {
            g_serv_game->word_guess[i].word = g_serv_game->word_guess[i].word + (size_t)g_serv_game; 
        }

    }

    return EXIT_SUCCESS;
}

char *StrSerializeDup(const char *p) {
    u64 c_sz = *(u64 *)&g_file_arena;
    char *r = (char*)g_serv_game+c_sz; 
    
    const size_t sz = strlen(p);
    memcpy(r, p, sz);
    *(r+sz+1) = '\x00';
    *(u64 *)&g_file_arena += (sz+1);

    return r;
}

Result ExitAndSerializeGame() {
    R_FAIL_RETURN(EXIT_FAILURE, lseek(g_file_dat, 0, SEEK_SET),
                  "[ERROR] Failed to seek file.\n");

    /* Debase pointers. */
    g_serv_game->word_state = (char *)g_serv_game->word_state  - (size_t)g_serv_game;
    for(u32 i = 0; i < 40 && g_serv_game->word_guess[i].word != 0; ++i) {
        g_serv_game->word_guess[i].word = g_serv_game->word_guess[i].word - (size_t)g_serv_game; 
    }

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

const char *GetCurWord() { return dict_instance.entries[g_serv_game->cur_entry].word; }

char *GetCurRepr() { return g_serv_game->word_state; }

void RegisterLetterTrial() {}

void RegisterWordGuess() {}
