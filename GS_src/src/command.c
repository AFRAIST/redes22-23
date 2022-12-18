#include "command.h"
#include "Dictionary.h"
#include "serv_game.h"
#include "udp_sender.h"

static inline u32 get_errs(u32 size) {
    if (size < 7)
        return 7;
    else if (size < 11)
        return 8;

    return 9;
}

static Result start_impl(struct output *outp) {
    if (StartGame() == EXIT_FAILURE) {
        if (udp_sender_send((u8 *)"RSG ERR\n", 8) != 8) {
            perror(E_FAILED_REPLY);
        }

        return EXIT_FAILURE;
    }

    const char *w = GetCurWord();
    const u32 size = strlen(w);
    const u32 errs = get_errs(size);

    const size_t send_buf_sz = sizeof("RSG ERR 33 9\n");
    char send_buf[send_buf_sz];
    
    snprintf(send_buf, send_buf_sz, "RSG %s %zu %u\n",
    GameHasMoves() ? "NOK" : "OK",
    outp->plid,
    errs);
    
    const size_t send_len = strlen(send_buf);
    if ((size_t)udp_sender_send((u8 *)send_buf, send_len) != send_len) {
        perror(E_FAILED_REPLY);
        return EXIT_FAILURE;
    }

    VerbosePrintF("%s\n", GetCurWord());
    return EXIT_SUCCESS;
}

void command_start(struct output *outp) {
    if (outp->err) {
        R_FAIL_EXIT_IF(udp_sender_send((u8 *)"RSG ERR\n", 8) != 8,
                      E_FAILED_REPLY);

        exit(EXIT_SUCCESS);
    }

    if (GameAcquire(outp->plid) == EXIT_FAILURE) {
        perror(E_ACQUIRE_ERROR);
        R_FAIL_EXIT_IF(udp_sender_send((u8 *)"RSG ERR\n", 8) != 8,
                      E_FAILED_REPLY);
    
        exit(EXIT_FAILURE);
    }

    Result rc;
    if((rc = start_impl(outp)) == EXIT_SUCCESS) {
        rc = ExitAndSerializeGame();
    }

    R_FAIL_EXIT_IF(GameRelease() == EXIT_FAILURE, E_RELEASE_ERROR);
    exit(rc);
}

Result command_play(struct output *outp) {
    (void)outp;
    /*
    int plid, trials;
    char letter;
    printf("AAAAAAAA\n");
#define OPT_NUM 3
    char *opts[OPT_NUM + 1] = {NULL};

    BufTokenizeOpts(outp->buff, opts, COMMAND_BUF_SZ);

    sscanf(outp->buff, "%i %c %i\n", &plid, &letter, &trials);
    printf("%i %c %i\n", plid, letter, trials);
    */
    return EXIT_SUCCESS;
#undef OPT_NUM
}

Result command_guess(struct output *outp) {
    (void)outp;
    /*
    int plid, trials;
    char word[31];
    printf("AAAAAAAA\n");
    sscanf(outp->buff, "%i %s %i\n", &plid, word, &trials);
    printf("%i %s %i\n", plid, word, trials);
    */
    return EXIT_SUCCESS;
}

Result command_scoreboard(struct output *outp) {
    (void)(outp);
    R_NOT_IMPLEMENTED();
    return EXIT_SUCCESS;
}

Result command_hint(struct output *outp) {
    (void)(outp);
    R_NOT_IMPLEMENTED();
    return EXIT_SUCCESS;
}

Result command_state(struct output *outp) {
    (void)(outp);
    R_NOT_IMPLEMENTED();
    return EXIT_SUCCESS;
}

Result command_quit(struct output *outp) {
    (void)(outp);
    R_NOT_IMPLEMENTED();
    return EXIT_SUCCESS;
}

Result command_exit(struct output *outp) {
    (void)(outp);
    R_NOT_IMPLEMENTED();
    return EXIT_SUCCESS;
}
