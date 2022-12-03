#include "command.h"
#include "game.h"
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

Result command_start(struct input *inp) {
    if (g_game.is_active) {
        printf("There is already an ongoing session of a game. Use quit PLID "
               "to stop.\n");
        return EXIT_SUCCESS;
    }

    if (get_plid(inp) == EXIT_FAILURE) {
        printf("Please, insert a valid plid.\n");
        return EXIT_SUCCESS;
    } else {
        printf("%06zu c:\n", inp->plid);
    }

    const size_t send_buf_sz =
        STR_SIZEOF("SNG ") + sizeof('0') * 6 + sizeof('\n') + sizeof('\0');
    char send_buf[send_buf_sz];

    /* Will not include the null. */
    size_t sz = (size_t)sprintf(send_buf, "SNG %06zu\n", inp->plid);

    R_FAIL_RETURN(EXIT_FAILURE, udp_sender_try_init() == EXIT_FAILURE,
                  E_FAILED_SOCKET);

    R_FAIL_RETURN(EXIT_FAILURE,
                  udp_sender_send((u8 *)send_buf, sz) == EXIT_FAILURE,
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

    // printf("%zu %zu\n", n_letters, n_errors);
    game_init(&g_game, inp->plid, n_letters, n_errors);

    return EXIT_SUCCESS;

#undef OPT_NUM
}

Result command_play(struct input *inp) {
    if (!g_game.is_active) {
        printf("Nada fixe :c\n");
        return EXIT_FAILURE;
    }

    int appendix_size = strlen(inp->appendix);

    if (appendix_size != 1) {
        printf("Palavra Invalida");
        return EXIT_FAILURE;
    }

    if (toupper(inp->appendix[0]) > 'Z' || toupper(inp->appendix[0]) < 'A') {
        return EXIT_FAILURE;
    }

    const size_t buf_sz = STR_SIZEOF("PLG") + sizeof('0') * 6 +
                          sizeof(' ') * 3 + sizeof(int) + sizeof(char) +
                          sizeof('\n') + sizeof('\0');
    char buf[buf_sz];

    // i fix later
    size_t sz = (size_t)sprintf(buf, "PLG %zu %s %i\n", inp->plid,
                                inp->appendix, g_game.attempts);

    R_FAIL_RETURN(EXIT_FAILURE, udp_sender_send((u8 *)buf, sz) == EXIT_FAILURE,
                  "[ERROR] Could not send player id to server.\n");

    return EXIT_SUCCESS;
    return EXIT_SUCCESS;
}

Result command_guess(struct input *inp) {
    if (!g_game.is_active) {
        printf("Nada fixe :c\n");
        return EXIT_FAILURE;
    }

    int appendix_size = strlen(inp->appendix);

    if (appendix_size < 3 || appendix_size > 30) {
        printf("Palavra Invalida");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < appendix_size; i++) {
        if (toupper(inp->appendix[i]) > 'Z' ||
            toupper(inp->appendix[i]) < 'A') {
            return EXIT_FAILURE;
        }
    }

    const size_t buf_sz =
        STR_SIZEOF("PWG") + sizeof('0') * 6 + sizeof(' ') * 3 + sizeof(int) +
        sizeof(char) * appendix_size + sizeof('\n') + sizeof('\0');
    char buf[buf_sz];

    // same... i want to sleep after this, sorry...
    size_t sz = (size_t)sprintf(buf, "PWG %zu %s %i\n", inp->plid,
                                inp->appendix, g_game.attempts);

    R_FAIL_RETURN(EXIT_FAILURE, udp_sender_send((u8 *)buf, sz) == EXIT_FAILURE,
                  "[ERROR] Could not send player id to server.\n");

    return EXIT_SUCCESS;
}

Result command_scoreboard(struct input *inp) {
    (void)(inp);
    R_NOT_IMPLEMENTED();
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

    R_FAIL_RETURN(EXIT_FAILURE, udp_sender_try_init() == EXIT_FAILURE,
                  E_FAILED_SOCKET);

    R_FAIL_RETURN(EXIT_FAILURE,
                  udp_sender_send((u8 *)send_buf, sz) == EXIT_FAILURE,
                  E_FAILED_REPLY);

    const size_t recv_buf_sz = sizeof("RQT ERR\n");
    char recv_buf[recv_buf_sz];
    R_FAIL_RETURN(EXIT_FAILURE,
                  (sz = udp_sender_recv((u8 *)recv_buf, recv_buf_sz)) ==
                      EXIT_FAILURE,
                  E_FAILED_RECEIVE);

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
    if (!g_game.is_active) {
        printf("There is no game running.\n");
        return EXIT_SUCCESS;
    }

    return command_quit_impl();
}

Result command_exit() {
    if (g_game.is_active)
        return command_quit_impl();

    return EXIT_SUCCESS;
}