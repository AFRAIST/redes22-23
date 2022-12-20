#include "command.h"
#include "game.h"
#include "tcp_sender.h"
#include "udp_sender.h"

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

    if (inp->appendix == NULL) {
        printf("Input invalido. :c\n");
        return EXIT_FAILURE;
    }

    if (get_plid(inp->appendix, &inp->plid) == EXIT_FAILURE) {
        printf("Please, insert a valid plid.\n");
        return EXIT_SUCCESS;
    } else {
        printf("%06zu c:\n", inp->plid);
    }

    const size_t send_buf_sz = sizeof("SNG 000000\n");
    char send_buf[send_buf_sz];

    /* Will not include the null. */
    ssize_t sz = snprintf(send_buf, send_buf_sz, "SNG %06zu\n", inp->plid);

    R_FAIL_RETURN(EXIT_FAILURE, udp_sender_try_init() == -1, E_FAILED_SOCKET);

    R_FAIL_RETURN(EXIT_FAILURE,
                  udp_sender_send((u8 *)send_buf, sz) != (ssize_t)sz,
                  E_FAILED_REPLY);

    const size_t recv_buf_sz = sizeof("RSG ERR 33 9\n");
    char recv_buf[recv_buf_sz];
    /* -1 because of '\x00' */
    R_FAIL_RETURN(EXIT_FAILURE,
                  (sz = udp_sender_recv((u8 *)recv_buf, recv_buf_sz - 1)) == -1,
                  E_FAILED_RECEIVE);
    recv_buf[sz] = '\x00';

    // memset(recv_buf, 0, recv_buf_sz);
    // strcpy(recv_buf, "RSG OK 10 8\n");

    R_FAIL_RETURN(EXIT_FAILURE,
                  BufNotContainsInvalidNull(recv_buf, sz) == EXIT_FAILURE,
                  E_INVALID_SERVER_REPLY);

    char *next;
    char *tok;

    tok = BufTokenizeOpt(recv_buf, " ", &next);

    R_FAIL_RETURN(EXIT_FAILURE, strcmp(tok, "RSG"), E_INVALID_SERVER_REPLY);

    tok = BufTokenizeOpt(next, " ", &next);

    /* Check status. */
    if (!strcmp(tok, "NOK\n")) {
        printf("There is an already ongoing game, use quit to finish it.\n");
        return EXIT_SUCCESS;
    }

    R_FAIL_RETURN(EXIT_FAILURE, strcmp(tok, "OK"), E_INVALID_SERVER_REPLY);

    size_t n_letters, n_errors;

    tok = BufTokenizeOpt(next, " ", &next);
    R_FAIL_RETURN(EXIT_FAILURE,
                  strtoul_check((ssize_t *)&n_letters, tok) == EXIT_FAILURE ||
                      !IS_IN_RANGE(n_letters, 3, 30),
                  E_INVALID_SERVER_REPLY);

    R_FAIL_RETURN(EXIT_FAILURE,
                  final_num(next, &n_errors) == EXIT_FAILURE ||
                      !IS_IN_RANGE(n_errors, 7, 9),
                  E_INVALID_SERVER_REPLY);

    game_init(&g_game, inp->plid, n_letters, n_errors);

    printf("New game started. Guess %zu letter word: %s\n", n_letters,
           g_game.word);

    return EXIT_SUCCESS;
}

static Result finalize_play_opts(char *recv_buf, char c) {
    char *next;
    char *tok;
    size_t attempts;

    tok = BufTokenizeOpt(recv_buf, " ", &next);
    R_FAIL_RETURN(EXIT_FAILURE, strcmp(tok, "RLG"), E_INVALID_SERVER_REPLY);

    tok = BufTokenizeOpt(next, " ", &next);
    if (!strcmp(tok, "ERR\n")) {
        R_FAIL_RETURN(EXIT_FAILURE, *next != '\x00', E_INVALID_SERVER_REPLY);
        perror(E_SERVER_ERROR);
        return EXIT_FAILURE;
    } else if (!strcmp(tok, "OVR")) {
        R_FAIL_RETURN(EXIT_FAILURE, final_num(next, &attempts) == EXIT_FAILURE,
                      E_INVALID_SERVER_REPLY);
        printf("OWARI DAAAAA.\n");
        g_game.is_active = false;
    } else if (!strcmp(tok, "DUP")) {
        R_FAIL_RETURN(EXIT_FAILURE, final_num(next, &attempts) == EXIT_FAILURE,
                      E_INVALID_SERVER_REPLY);
        printf("Detected repetition.\n");
    } else if (!strcmp(tok, "NOK")) {
        R_FAIL_RETURN(EXIT_FAILURE, final_num(next, &attempts) == EXIT_FAILURE,
                      E_INVALID_SERVER_REPLY);
        printf("The word does not contain a '%c'.\n", c);
    } else if (!strcmp(tok, "INV")) {
        R_FAIL_RETURN(EXIT_FAILURE, final_num(next, &attempts) == EXIT_FAILURE,
                      E_INVALID_SERVER_REPLY);
        perror(E_TRIAL_MISMATCH);
        g_game.cur_attempt = attempts;
        return EXIT_FAILURE;
    } else if (!strcmp(tok, "WIN")) {
        R_FAIL_RETURN(EXIT_FAILURE, final_num(next, &attempts) == EXIT_FAILURE,
                      E_INVALID_SERVER_REPLY);
        str_replace(g_game.word, '-', c);
        printf("You won! The word was: %s\n", g_game.word);
        g_game.is_active = false;
    } else if (!strcmp(tok, "OK")) {
        tok = BufTokenizeOpt(next, " ", &next);
        R_FAIL_RETURN(EXIT_FAILURE, strtoul_check((ssize_t *)&attempts, tok),
                      E_INVALID_COMMAND);

        size_t num, num2;
        tok = BufTokenizeOpt(next, " ", &next);
        R_FAIL_RETURN(EXIT_FAILURE,
                      strtoul_check((ssize_t *)&num, tok) == EXIT_FAILURE ||
                          num > 30 || num == 0,
                      E_INVALID_SERVER_REPLY);

        for (u32 i = 0; i < (num - 1); ++i) {
            tok = BufTokenizeOpt(next, " ", &next);
            R_FAIL_RETURN(EXIT_FAILURE,
                          strtoul_check((ssize_t *)&num2, tok) ==
                                  EXIT_FAILURE ||
                              num2 > 30,
                          E_INVALID_SERVER_REPLY);

            g_game.word[num2 - 1] = c;
        }

        R_FAIL_RETURN(EXIT_FAILURE, final_num(next, &num2) == EXIT_FAILURE,
                      E_INVALID_SERVER_REPLY);
        g_game.word[num2 - 1] = c;

        printf("Guess word: %s\n", g_game.word);
    } else {
        perror(E_INVALID_SERVER_REPLY);
        return EXIT_FAILURE;
    }

    g_game.cur_attempt = attempts;
    return EXIT_SUCCESS;
}

Result command_play(struct input *inp) {
    RETURN_IF_NOT_ACTIVE_GAME();

    if (inp->appendix == NULL) {
        printf("Input invalido. :c\n");
        return EXIT_FAILURE;
    }

    size_t appendix_size = strlen(inp->appendix);

    const char c = toupper(inp->appendix[0]);
    if (appendix_size != 1 || c > 'Z' || c < 'A') {
        printf("Palavra Inválida.\n");
        return EXIT_SUCCESS;
    }

    char buf[0x1000];

    // handle max
    const u32 attempt = ++g_game.cur_attempt;

    size_t sz = (size_t)sprintf(buf, "PLG %06zu %c %u\n", g_game.plid,
                                 c, attempt);

    R_FAIL_RETURN(EXIT_FAILURE, udp_sender_send((u8 *)buf, sz) != (ssize_t)sz,
                  E_FAILED_REPLY);

    const size_t recv_buf_sz = 0x1000;
    char recv_buf[recv_buf_sz];
    R_FAIL_RETURN(EXIT_FAILURE,
                  (sz = udp_sender_recv((u8 *)recv_buf, recv_buf_sz - 1)) ==
                      EXIT_FAILURE,
                  E_FAILED_RECEIVE);
    recv_buf[sz] = '\x00';

    // memset(recv_buf, 0, recv_buf_sz);
    // strcpy(recv_buf, "RLG ERR 7\n");

    R_FAIL_RETURN(EXIT_FAILURE,
                  BufNotContainsInvalidNull(recv_buf, sz) == EXIT_FAILURE,
                  E_INVALID_SERVER_REPLY);

    return finalize_play_opts(recv_buf, c);
}

static Result finalize_guess_opts(char *recv_buf, char *word) {
    char *next;
    char *tok;
    size_t attempts;

    tok = BufTokenizeOpt(recv_buf, " ", &next);
    R_FAIL_RETURN(EXIT_FAILURE, strcmp(tok, "RWG"), E_INVALID_SERVER_REPLY);

    tok = BufTokenizeOpt(next, " ", &next);
    if (!strcmp(tok, "ERR\n")) {
        R_FAIL_RETURN(EXIT_FAILURE, *next != '\x00', E_INVALID_SERVER_REPLY);
        perror(E_SERVER_ERROR);
        return EXIT_FAILURE;
    } else if (!strcmp(tok, "OVR")) {
        R_FAIL_RETURN(EXIT_FAILURE, final_num(next, &attempts) == EXIT_FAILURE,
                      E_INVALID_SERVER_REPLY);
        printf("OWARI DAAAAA.\n");
        g_game.is_active = false;
    }else if (!strcmp(tok, "DUP")) {
        R_FAIL_RETURN(EXIT_FAILURE, final_num(next, &attempts) == EXIT_FAILURE,
                      E_INVALID_SERVER_REPLY);
        printf("Try Another Word! The Word %s was already used\n", word);
    } else if (!strcmp(tok, "NOK")) {
        R_FAIL_RETURN(EXIT_FAILURE, final_num(next, &attempts) == EXIT_FAILURE,
                      E_INVALID_SERVER_REPLY);
        printf("Try again! The word was not: %s\n", word);
    } else if (!strcmp(tok, "INV")) {
        R_FAIL_RETURN(EXIT_FAILURE, final_num(next, &attempts) == EXIT_FAILURE,
                      E_INVALID_SERVER_REPLY);
        perror(E_TRIAL_MISMATCH);
        return EXIT_FAILURE;
    } else if (!strcmp(tok, "WIN")) {
        R_FAIL_RETURN(EXIT_FAILURE, final_num(next, &attempts) == EXIT_FAILURE,
                      E_INVALID_SERVER_REPLY);
        printf("You won! The word was: %s\n", word);
        g_game.is_active = false;
    } else {
        perror(E_INVALID_SERVER_REPLY);
        return EXIT_FAILURE;
    }

    g_game.cur_attempt = attempts;
    return EXIT_SUCCESS;
}

Result command_guess(struct input *inp) {
    RETURN_IF_NOT_ACTIVE_GAME();

    if (inp->appendix == NULL) {
        printf("Input invalido. :c\n");
        return EXIT_SUCCESS;
    }

    size_t appendix_size = strlen(inp->appendix);

    if (appendix_size < 3 || appendix_size > 30) {
        printf("Palavra Invalida");
        return EXIT_SUCCESS;
    }

    for (u32 i = 0; i < appendix_size; i++) {
        if (toupper(inp->appendix[i]) > 'Z' ||
            toupper(inp->appendix[i]) < 'A') {
            printf("Palavra Invalida");
            return EXIT_SUCCESS;
        }
    }

    const size_t buf_sz = 0x1000;
    char buf[buf_sz];

    // handle max
    const u32 attempt = ++g_game.cur_attempt;

    size_t sz = (size_t)snprintf(buf, buf_sz, "PWG %06zu %s %i\n", g_game.plid,
                                 inp->appendix, attempt);

    R_FAIL_RETURN(EXIT_FAILURE, udp_sender_send((u8 *)buf, sz) != (ssize_t)sz,
                  E_FAILED_REPLY);

    const size_t recv_buf_sz = 0x1000;
    char recv_buf[recv_buf_sz];
    R_FAIL_RETURN(EXIT_FAILURE,
                  (sz = udp_sender_recv((u8 *)recv_buf, recv_buf_sz - 1)) ==
                      EXIT_FAILURE,
                  E_FAILED_RECEIVE);
    recv_buf[sz] = '\x00';

    R_FAIL_RETURN(EXIT_FAILURE,
                  BufNotContainsInvalidNull(recv_buf, sz) == EXIT_FAILURE,
                  E_INVALID_SERVER_REPLY);

    return finalize_guess_opts(recv_buf, inp->appendix);
}

/* Used for the TCP communications. */
#define BIG_BUF_SZ (4 * 1024 * 1024)
static u8 big_buffer[BIG_BUF_SZ];

static Result get_file(u32 offset, u32 whence, bool show) {
    /* Data right after status. */
    u8 *buf = big_buffer + offset;
    bool fin;
    /* No file metadata will take 4MiB, so we can nuke false positives here,
     * too. */

    u32 sz = (u32)tcp_sender_recv_all(big_buffer + whence, BIG_BUF_SZ - whence,
                                      &fin);
    R_FAIL_RETURN(EXIT_FAILURE, (s32)sz == -1 || sz == 0, E_FAILED_RECEIVE);

    char *tok;
    char *next;

    char *fname;
    fname = BufTokenizeOpt((char *)buf, " ", &next);

    tok = BufTokenizeOpt((char *)next, " ", &next);
    size_t full_size;
    R_FAIL_RETURN(EXIT_FAILURE,
                  strtoul_check((ssize_t *)&full_size, tok) == EXIT_FAILURE,
                  E_INVALID_SERVER_REPLY);

    /* Derive out. */
    size_t cur_sz = (size_t)((big_buffer + whence + sz) - (u8 *)next);

    int fd = open(fname, O_WRONLY | O_CREAT, 0644);
    R_FAIL_RETURN(EXIT_FAILURE, fd == -1, "[ERROR] Failed to open file.\n");

    if (flock(fd, LOCK_EX) == -1) {
        perror("[ERR] Flock.\n");
        goto error_close;
    }

    printf("Saving file to %s ...\n", fname);

    if (show)
        write(1, next, !fin ? cur_sz : cur_sz - 1);

    if (write(fd, next, !fin ? cur_sz : cur_sz - 1) == -1) {
        perror("[ERROR] Failed to write file.\n");
        goto error;
    }

    /* In caise it does not get looped, to validate the restriction. */
    sz += whence - 1;

    while (cur_sz != (full_size+1)) {
        if (fin) {
            /* Contradiction with the packet info... */
            perror(E_INVALID_SERVER_REPLY);
            goto error;
        }

        sz = (u32)tcp_sender_recv_all(big_buffer, BIG_BUF_SZ, &fin);

        if ((s32)sz == -1 || sz == 0) {
            perror(E_FAILED_RECEIVE);
            goto error;
        }

        cur_sz += sz;
        if (fin) {
            sz -= 1;
        }

        if (show)
            write(1, big_buffer, sz);

        if (write(fd, big_buffer, sz) == -1) {
            perror("[ERROR] Failed to write file.\n");
            goto error;
        }
    }

    /* Final packet accuracy assert. */
    if (big_buffer[sz] != '\n') {
        perror(E_INVALID_SERVER_REPLY);
        goto error;
    }

    if (!fin) {
        perror(E_FAILED_RECEIVE);
        goto error;
    }

    if (flock(fd, LOCK_UN) == -1) {
        perror("[ERR] Flock.\n");
        goto error_close;
    }

    R_FAIL_RETURN(EXIT_FAILURE, close(fd) == -1, "[ERROR] Failed to close file.\n");

    return EXIT_SUCCESS;

error:
    if (flock(fd, LOCK_UN) == -1) {
        perror("[ERR] Flock.\n");
    }

error_close: 
    if (close(fd) == -1) {
        perror("[ERROR] Failed to close file.\n");
    }

    return EXIT_FAILURE;
}

static Result scoreboard_impl() {
    R_FAIL_RETURN(EXIT_FAILURE, tcp_sender_handshake() == -1,
                  E_HANDSHAKE_FAILED);

    R_FAIL_RETURN(EXIT_FAILURE, tcp_sender_send((u8 *)"GSB\n", 4) == -1,
                  E_FAILED_REPLY);

    const u32 lim = STR_SIZEOF("RSB EMPTY\n") + 1; // for EOF detect

    u32 sz;
    bool fin;
    /* -1 for OK leetter */
    /* It is safe to assume there is no data incoming shorter than EMPTY. */
    R_FAIL_RETURN(EXIT_FAILURE,
                  (s32)(sz = tcp_sender_recv_all(big_buffer, lim, &fin)) == -1,
                  E_INVALID_SERVER_REPLY);

    big_buffer[sz] = '\x00';

    // memset(big_buffer, 0, 1024);
    // strcpy(big_buffer, "RSB OK wwww");

    if (!strcmp((char *)big_buffer, "RSB EMPTY\n")) {
        R_FAIL_RETURN(EXIT_FAILURE, fin == false, E_INVALID_SERVER_REPLY);
        printf("The scoreboard is empty...\n");
        return EXIT_SUCCESS;
    }

    R_FAIL_RETURN(EXIT_FAILURE, strncmp((char *)big_buffer, "RSB OK ", 7),
                  E_INVALID_SERVER_REPLY);

    /* Save file. */
    if (get_file(7, sz, true) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

Result command_scoreboard(struct input *inp) {
    (void)inp;

    R_FAIL_RETURN(EXIT_FAILURE, tcp_sender_try_init() != EXIT_SUCCESS,
                  E_FAILED_SOCKET);

    const Result rc = scoreboard_impl();

    R_FAIL_RETURN(EXIT_FAILURE, tcp_sender_fini() == -1, E_CLOSE_SOCKET);

    return rc;
}

static Result hint_impl() {
    R_FAIL_RETURN(EXIT_FAILURE, tcp_sender_handshake() == -1,
                  E_HANDSHAKE_FAILED);

    const size_t send_buf_sz = sizeof("GHL 000000\n");
    char send_buf[send_buf_sz];
    snprintf(send_buf, send_buf_sz, "GHL %06zu\n", g_game.plid);

    R_FAIL_RETURN(EXIT_FAILURE,
                  tcp_sender_send((u8 *)send_buf, send_buf_sz - 1) == -1,
                  E_FAILED_REPLY);

    const u32 lim = STR_SIZEOF("RHL NOK\n") + 1; // for EOF detct

    u32 sz;
    bool fin;
    /* It is safe to assume there is no data incoming shorter than EMPTY. */
    R_FAIL_RETURN(EXIT_FAILURE,
                  (s32)(sz = tcp_sender_recv_all(big_buffer, lim, &fin)) == -1,
                  E_INVALID_SERVER_REPLY);

    big_buffer[sz] = '\x00';

    if (!strcmp((char *)big_buffer, "RHL NOK\n")) {
        R_FAIL_RETURN(EXIT_FAILURE, fin == false, E_INVALID_SERVER_REPLY);
        printf("Server could not provide a hint file...\n");
        return EXIT_SUCCESS;
    }

    R_FAIL_RETURN(EXIT_FAILURE, strncmp((char *)big_buffer, "RHL OK ", 7),
                  E_INVALID_SERVER_REPLY);

    /* Save file. */
    if (get_file(7, sz, false) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

Result command_hint(struct input *inp) {
    (void)inp;
    puts("ufeuefg");
    R_FAIL_RETURN(EXIT_FAILURE, tcp_sender_try_init() != EXIT_SUCCESS,
                  E_FAILED_SOCKET);

    const Result rc = hint_impl();

    R_FAIL_RETURN(EXIT_FAILURE, tcp_sender_fini() == -1, E_CLOSE_SOCKET);

    return rc;
}

static Result state_impl() {
    R_FAIL_RETURN(EXIT_FAILURE, tcp_sender_handshake() == -1,
                  E_HANDSHAKE_FAILED);

    const size_t send_buf_sz = sizeof("STA 000000\n");
    char send_buf[send_buf_sz];
    snprintf(send_buf, send_buf_sz, "STA %06zu\n", g_game.plid);

    R_FAIL_RETURN(EXIT_FAILURE,
                  tcp_sender_send((u8 *)send_buf, send_buf_sz - 1) == -1,
                  E_FAILED_REPLY);

    const u32 lim = STR_SIZEOF("RST NOK\n") + 1; // for eof detect

    u32 sz;
    bool fin;
    /* It is safe to assume there is no data incoming shorter than EMPTY. */
    R_FAIL_RETURN(EXIT_FAILURE,
                  (s32)(sz = tcp_sender_recv_all(big_buffer, lim, &fin)) == -1,
                  E_INVALID_SERVER_REPLY);

    big_buffer[sz] = '\x00';

    if (!strcmp((char *)big_buffer, "RST NOK\n")) {
        R_FAIL_RETURN(EXIT_FAILURE, fin == false, E_INVALID_SERVER_REPLY);
        printf("Server could not find a valid game...\n");
        return EXIT_SUCCESS;
    }

    R_FAIL_RETURN(EXIT_FAILURE, strncmp((char *)big_buffer, "RST ", 4),
                  E_INVALID_SERVER_REPLY);

    bool term;
    if (!strncmp((char *)big_buffer, "RST ACT ", 8)) {
        term = true;
    } else if (!strncmp((char *)big_buffer, "RST FIN ", 8)) {
        term = false;
    } else {
        perror(E_INVALID_COMMAND);
        return EXIT_FAILURE;
    }

    if (get_file(8, lim, true) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    g_game.is_active = term;

    return EXIT_SUCCESS;
}

Result command_state(struct input *inp) {
    (void)inp;
    RETURN_IF_NOT_ACTIVE_GAME();

    R_FAIL_RETURN(EXIT_FAILURE, tcp_sender_try_init() != EXIT_SUCCESS,
                  E_FAILED_SOCKET);

    const Result rc = state_impl();

    R_FAIL_RETURN(EXIT_FAILURE, tcp_sender_fini() == -1, E_CLOSE_SOCKET);

    return rc;
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
    recv_buf[sz] = '\x00';

    puts(recv_buf);

    char *tok;
    char *next;

    tok = BufTokenizeOpt(recv_buf, " ", &next);
    R_FAIL_RETURN(EXIT_FAILURE, strcmp(tok, "RQT"), E_INVALID_SERVER_REPLY);

    tok = BufTokenizeOpt(next, " ", &next);
    R_FAIL_RETURN(EXIT_FAILURE, !strcmp(tok, "ERR\n"), E_SERVER_ERROR);

    if (!strcmp(tok, "NOK\n")) {
        printf("No need to requit.\n");
        return EXIT_SUCCESS;
    }

    R_FAIL_RETURN(EXIT_FAILURE, strcmp(tok, "OK\n"), E_INVALID_SERVER_REPLY);

    game_fini(&g_game);
    return EXIT_SUCCESS;
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
