#include "command.h"
#include "game.h"
#include "tcp_sender.h"
#include "udp_sender.h"

static Result get_plid(struct input *inp) {
    char *ap = inp->appendix;
    if (*ap == '\0')
        return EXIT_FAILURE;

    int appendix_size = strlen(inp->appendix);

    if (appendix_size > 6 || appendix_size <= 0)
        return EXIT_FAILURE;

    if (strtoul_check((ssize_t *)&inp->plid, ap) == EXIT_FAILURE)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

#define RETURN_IF_ACTIVE_GAME()                                                \
    if (g_game.is_active) {                                                    \
        printf("There is already an ongoing session of a game. Use quit "      \
               "to stop.\n");                                                  \
        return EXIT_SUCCESS;                                                   \
    }

#define RETURN_IF_NOT_ACTIVE_GAME()                                            \
    if (!g_game.is_active) {                                                   \
        printf("There is no ongoing session of a game. Use start PLID "        \
               "to start one.\n");                                             \
        return EXIT_SUCCESS;                                                   \
    }

Result command_start(struct input *inp) {
    RETURN_IF_ACTIVE_GAME();

    if (get_plid(inp) == EXIT_FAILURE) {
        printf("Please, insert a valid plid.\n");
        return EXIT_SUCCESS;
    } else {
        printf("%06zu c:\n", inp->plid);
    }

    const size_t send_buf_sz = sizeof("SNG 000000\n");
    char send_buf[send_buf_sz];

    /* Will not include the null. */
    size_t sz = (size_t)sprintf(send_buf, "SNG %06zu\n", inp->plid);

    R_FAIL_RETURN(EXIT_FAILURE, udp_sender_try_init() == -1, E_FAILED_SOCKET);

    R_FAIL_RETURN(EXIT_FAILURE,
                  udp_sender_send((u8 *)send_buf, sz) != (ssize_t)sz,
                  E_FAILED_REPLY);

    const size_t recv_buf_sz = sizeof("RSG ERR 33 9\n");
    char recv_buf[recv_buf_sz];
    R_FAIL_RETURN(EXIT_FAILURE,
                  (sz = udp_sender_recv((u8 *)recv_buf, recv_buf_sz)) ==
                      EXIT_FAILURE,
                  E_FAILED_RECEIVE);

#define OPT_NUM 4
    char *opts[OPT_NUM + 1] = {NULL};

    R_FAIL_RETURN(EXIT_FAILURE,
                  BufTokenizeOpts(recv_buf, opts, sz) == EXIT_FAILURE,
                  E_INVALID_SERVER_REPLY);

    R_FAIL_RETURN(EXIT_FAILURE, opts[0] == NULL || strcmp(opts[0], "RSG"),
                  E_INVALID_SERVER_REPLY);

    R_FAIL_RETURN(EXIT_FAILURE, opts[1] != NULL && !strcmp(opts[1], "NOK"),
                  E_INVALID_COMMAND);

    R_FAIL_RETURN(EXIT_FAILURE, opts[1] == NULL || strcmp(opts[1], "OK"),
                  E_INVALID_SERVER_REPLY);

    R_FAIL_RETURN(EXIT_FAILURE, opts[2] == NULL || opts[3] == NULL,
                  E_INVALID_SERVER_REPLY);

    size_t n_letters, n_errors;

    R_FAIL_RETURN(
        EXIT_FAILURE,
        (strtoul_check((ssize_t *)&n_letters, opts[2]) == EXIT_FAILURE ||
         strtoul_check((ssize_t *)&n_errors, opts[3]) == EXIT_FAILURE ||
         !IS_IN_RANGE(n_letters, 3, 30) || !IS_IN_RANGE(n_errors, 7, 9)),
        E_INVALID_NUMBER_REPLY);

    game_init(&g_game, inp->plid, n_letters, n_errors);

    printf("New game started. Guess %zu letter word: %s\n", n_letters,
           g_game.word);

    return EXIT_SUCCESS;

#undef OPT_NUM
}

static Result finalize_play_opts(char **opts, char c) {
    if (!strcmp(opts[1], "ERR")) {
        R_FAIL_RETURN(EXIT_FAILURE, opts[2] != NULL, E_INVALID_SERVER_REPLY);
        perror(E_SERVER_ERROR);
        return EXIT_FAILURE;
    } else {
        size_t num;
        R_FAIL_RETURN(EXIT_FAILURE,
                      opts[2] == NULL || strtoul_check((ssize_t *)&num,
                                                       opts[2]) == EXIT_FAILURE,
                      E_INVALID_NUMBER_REPLY);

        R_FAIL_RETURN(EXIT_FAILURE, g_game.cur_attempt != num,
                      E_INVALID_TRIAL_REPLY);

        if (!strcmp(opts[1], "OK")) {
#define OPT_NUM 34
            R_FAIL_RETURN(EXIT_FAILURE, opts[4] == NULL,
                          E_INVALID_SERVER_REPLY);

            R_FAIL_RETURN(EXIT_FAILURE,
                          strtoul_check((ssize_t *)&num, opts[3]) ==
                              EXIT_FAILURE,
                          E_INVALID_NUMBER_REPLY);

            R_FAIL_RETURN(EXIT_FAILURE, num > 30, E_INVALID_NUMBER_REPLY);

            num += 4;
            for (u32 i = 4; i < num; ++i) {
                R_FAIL_RETURN(EXIT_FAILURE, opts[i] == NULL,
                              E_INVALID_SERVER_REPLY);

                size_t num2;
                R_FAIL_RETURN(EXIT_FAILURE,
                              strtoul_check((ssize_t *)&num2, opts[i]) ==
                                      EXIT_FAILURE ||
                                  num2 > 30,
                              E_INVALID_NUMBER_REPLY);
                g_game.word[num2 - 1] = c;
            }

            R_FAIL_RETURN(EXIT_FAILURE, num < OPT_NUM && opts[num] != NULL,
                          E_INVALID_SERVER_REPLY);

            printf("Guess word: %s\n", g_game.word);

#undef OPT_NUM
        } else if (!strcmp(opts[1], "WIN")) {
            R_FAIL_RETURN(EXIT_FAILURE, opts[3] != NULL,
                          E_INVALID_SERVER_REPLY);
            str_replace(g_game.word, '_', c);
            printf("You won! The word was: %s\n", g_game.word);
            g_game.is_active = false;
        } else if (!strcmp(opts[1], "DUP")) {
            --g_game.cur_attempt;
            R_FAIL_RETURN(EXIT_FAILURE, opts[3] != NULL,
                          E_INVALID_SERVER_REPLY);
            printf("Detected repetition.\n");
        } else if (!strcmp(opts[1], "NOK")) {
            R_FAIL_RETURN(EXIT_FAILURE, opts[3] != NULL,
                          E_INVALID_SERVER_REPLY);
            printf("The word does not contain a '%c'.\n", c);
        } else if (!strcmp(opts[1], "OVR")) {
            --g_game.cur_attempt;
            printf("OWARI DAAAAA.\n");
            g_game.is_active = false;
        } else if (!strcmp(opts[1], "INV")) {
            R_FAIL_RETURN(EXIT_FAILURE, opts[3] != NULL,
                          E_INVALID_SERVER_REPLY);
            perror(E_TRIAL_MISMATCH);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

Result command_play(struct input *inp) {
    RETURN_IF_NOT_ACTIVE_GAME();
    size_t appendix_size = strlen(inp->appendix);

    const char c = toupper(inp->appendix[0]);
    if (appendix_size != 1 || c > 'Z' || c < 'A') {
        printf("Palavra Inválida.\n");
        return EXIT_SUCCESS;
    }

    const size_t buf_sz = sizeof("PLG 000000 A 00\n");
    char buf[buf_sz];

    // handle max
    const u32 attempt = ++g_game.cur_attempt;

    size_t sz =
        (size_t)sprintf(buf, "PLG %06zu %c %u\n", g_game.plid, c, attempt);

    R_FAIL_RETURN(EXIT_FAILURE, udp_sender_send((u8 *)buf, sz) != (ssize_t)sz,
                  E_FAILED_REPLY);

    const size_t recv_buf_sz = STR_SIZEOF("RLG NOK 9 00 ") +
                               STR_SIZEOF("00 ") * 30 - sizeof(char) +
                               sizeof("\n");
    char recv_buf[recv_buf_sz];
    R_FAIL_RETURN(EXIT_FAILURE,
                  (sz = udp_sender_recv((u8 *)recv_buf, recv_buf_sz)) ==
                      EXIT_FAILURE,
                  E_FAILED_RECEIVE);

#define OPT_NUM 34
    char *opts[OPT_NUM + 1] = {NULL};

    puts(recv_buf);
    R_FAIL_RETURN(EXIT_FAILURE,
                  BufTokenizeOpts(recv_buf, opts, sz) == EXIT_FAILURE,
                  E_INVALID_SERVER_REPLY);

    R_FAIL_RETURN(EXIT_FAILURE, opts[0] == NULL || strcmp(opts[0], "RLG"),
                  E_INVALID_SERVER_REPLY);

    R_FAIL_RETURN(EXIT_FAILURE, opts[1] == NULL, E_INVALID_SERVER_REPLY);

    return finalize_play_opts(opts, c);
#undef OPT_NUM
}

static Result finalize_guess_opts(char **opts, char *word) {
#define OPT_NUM 3
    if (!strcmp(opts[1], "ERR")) {
        R_EXIT_IF(EXIT_FAILURE, opts[2] != NULL, E_INVALID_SERVER_REPLY);
        perror(E_SERVER_ERROR);
        return EXIT_FAILURE;
    } else {
        size_t num;
        R_FAIL_RETURN(EXIT_FAILURE,
                      opts[2] == NULL || strtoul_check((ssize_t *)&num,
                                                       opts[2]) == EXIT_FAILURE,
                      E_INVALID_NUMBER_REPLY);

        R_FAIL_RETURN(EXIT_FAILURE, g_game.cur_attempt != num,
                      E_INVALID_TRIAL_REPLY);

        if (!strcmp(opts[1], "WIN")) {
            printf("You won! The word was: %s\n", word);
            g_game.is_active = false;
        } else if (!strcmp(opts[1], "NOK")) {
            printf("Try again! The word was not: %s\n", word);
        } else if (!strcmp(opts[1], "OVR")) {
            --g_game.cur_attempt;
            printf("OWARI DAAAAA.\n");
        } else if (!strcmp(opts[1], "INV")) {
            perror(E_TRIAL_MISMATCH);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;

#undef OPT_NUM
}

Result command_guess(struct input *inp) {
    RETURN_IF_NOT_ACTIVE_GAME();
    size_t appendix_size = strlen(inp->appendix);

    if (appendix_size < 3 || appendix_size > 30) {
        printf("Palavra Invalida");
        return EXIT_SUCCESS;
    }

    for (u32 i = 0; i < appendix_size; i++) {
        if (toupper(inp->appendix[i]) > 'Z' ||
            toupper(inp->appendix[i]) < 'A') {
            return EXIT_SUCCESS;
        }
    }

    const size_t buf_sz =
        STR_SIZEOF("PWG 000000 ") + sizeof(char) * 30 + sizeof("9\n");
    char buf[buf_sz];

    // handle max
    const u32 attempt = ++g_game.cur_attempt;

    size_t sz = (size_t)sprintf(buf, "PWG %06zu %s %i\n", inp->plid,
                                inp->appendix, attempt);

    R_FAIL_RETURN(EXIT_FAILURE, udp_sender_send((u8 *)buf, sz) != (ssize_t)sz,
                  E_FAILED_REPLY);

    const size_t recv_buf_sz = sizeof("RWG ERR 9\n");
    char recv_buf[recv_buf_sz];
    R_FAIL_RETURN(EXIT_FAILURE,
                  (sz = udp_sender_recv((u8 *)recv_buf, recv_buf_sz)) ==
                      EXIT_FAILURE,
                  E_FAILED_RECEIVE);

#define OPT_NUM 3
    char *opts[OPT_NUM + 1] = {NULL};
    R_FAIL_RETURN(EXIT_FAILURE,
                  BufTokenizeOpts(recv_buf, opts, sz) == EXIT_FAILURE,
                  E_INVALID_SERVER_REPLY);

    /* Check for opt size and correct opt. */
    R_FAIL_RETURN(EXIT_FAILURE, opts[1] == NULL || strcmp(opts[0], "RWG"),
                  E_INVALID_SERVER_REPLY);

    return finalize_guess_opts(opts, inp->appendix);
#undef OPT_NUM
}

/* Used for the TCP communications. */
#define BIG_BUF_SZ (4 * 1024 * 1024)
static u8 big_buffer[BIG_BUF_SZ];

static void get_file(u32 offset, u32 whence) {
    u8 *buf = big_buffer + offset;
    (void)buf;
    u32 sz = (u32)tcp_sender_recv_all(big_buffer + whence, BIG_BUF_SZ - whence);
    R_FAIL_EXIT_IF((s32)sz == -1 || sz == 0, E_FAILED_RECEIVE);

    /*
        BufTokenizeOpts
        StrNSplitSpaceNext

        if (sz < BIG_BUF_SZ-whence)
    */
}

Result command_scoreboard(struct input *inp) {
    (void)inp;
    RETURN_IF_NOT_ACTIVE_GAME();

    R_FAIL_EXIT_IF(tcp_sender_try_init() != EXIT_SUCCESS, E_FAILED_SOCKET);

    const pid_t pid = fork();
    if (pid == 0) {
        R_FAIL_EXIT_IF(tcp_sender_handshake() == -1, E_HANDSHAKE_FAILED);

        const u32 lim = STR_SIZEOF("RSB EMPTY\n");

        u32 sz;
        /* -1 for OK leetter */
        R_FAIL_EXIT_IF((sz = tcp_sender_recv_all(big_buffer, lim)) <
                           STR_SIZEOF("RSB OK "),
                       E_INVALID_SERVER_REPLY);

        if (!strncmp((char *)big_buffer, "RSB EMPTY\n", lim)) {
            printf("The scoreboard is empty...\n");
            /* Maybe read more. */
            exit(EXIT_SUCCESS);
        }

        R_FAIL_EXIT_IF(strncmp((char *)big_buffer, "RSB OK ", 7),
                       E_INVALID_SERVER_REPLY);

        /* Save file. */
        get_file(sz, 7);

        exit(EXIT_SUCCESS);
    }

    return EXIT_SUCCESS;
}

Result command_hint(struct input *inp) {
    (void)(inp);
    R_NOT_IMPLEMENTED();
    return EXIT_SUCCESS;
}

Result command_state(struct input *inp) {
    (void)(inp);
    R_NOT_IMPLEMENTED();
    return EXIT_SUCCESS;
}

static Result command_quit_impl() {
    const size_t send_buf_sz = sizeof("QUT 000000\n");
    char send_buf[send_buf_sz];

    /* Will not include the null. */
    size_t sz = (size_t)sprintf(send_buf, "QUT %06zu\n", g_game.plid);

    R_FAIL_RETURN(EXIT_FAILURE,
                  udp_sender_send((u8 *)send_buf, sz) != (ssize_t)sz,
                  E_FAILED_REPLY);

    const size_t recv_buf_sz = sizeof("RQT ERR\n");
    char recv_buf[recv_buf_sz];
    R_FAIL_RETURN(EXIT_FAILURE,
                  (sz = udp_sender_recv((u8 *)recv_buf, recv_buf_sz)) ==
                      EXIT_FAILURE,
                  E_FAILED_RECEIVE);

    puts(recv_buf);
#define OPT_NUM 2
    char *opts[OPT_NUM + 1] = {NULL};

    R_FAIL_RETURN(EXIT_FAILURE,
                  BufTokenizeOpts(recv_buf, opts, sz) == EXIT_FAILURE,
                  E_INVALID_SERVER_REPLY);

    R_FAIL_RETURN(EXIT_FAILURE, opts[0] == NULL || strcmp(opts[0], "RQT"),
                  E_INVALID_SERVER_REPLY);

    R_FAIL_RETURN(EXIT_FAILURE, opts[1] != NULL && !strcmp(opts[1], "ERR"),
                  E_INVALID_COMMAND);

    R_FAIL_RETURN(EXIT_FAILURE, opts[1] == NULL || strcmp(opts[1], "OK"),
                  E_INVALID_SERVER_REPLY);

    game_fini(&g_game);
    return EXIT_SUCCESS;

#undef OPT_NUM
}

Result command_quit() {
    RETURN_IF_NOT_ACTIVE_GAME();

    return command_quit_impl();
}

Result command_exit() {
    if (g_game.is_active)
        return command_quit_impl();

    return EXIT_SUCCESS;
}

#undef RETURN_IF_ACTIVE_GAME
