#include "command.h"
#include "Dictionary.h"
#include "serv_game.h"
#include "udp_sender.h"
#include "tcp_sender.h"

static inline u32 get_errs(u32 size) {
    if (size < 7)
        return 7;
    else if (size < 11)
        return 8;

    return 9;
}

static Result start_impl(struct output *outp) {
    (void)outp;
    if (StartGame() == EXIT_FAILURE) {
        if (udp_sender_send((u8 *)"RSG ERR\n", 8) != 8) {
            perror(E_FAILED_REPLY);
        }

        return EXIT_FAILURE;
    }

    const char *w = GetCurWord();
    const u32 size = strlen(w);
    const u32 errs = get_errs(size);

    g_serv_game->max_errors = errs;

    const size_t send_buf_sz = sizeof("RSG ERR 33 9\n");
    char send_buf[send_buf_sz];
    
    snprintf(send_buf, send_buf_sz, "RSG %s %u %u\n",
    GameTrials() != 0 ? "NOK" : "OK",
    size,
    errs);
    
    const size_t send_len = strlen(send_buf);
    if ((size_t)udp_sender_send((u8 *)send_buf, send_len) != send_len) {
        perror(E_FAILED_REPLY);
        return EXIT_FAILURE;
    }

    VerbosePrintF("%s %s\n", GetCurWord(), GetCurRepr());
    return EXIT_SUCCESS;
}

void command_start(struct output *outp) {
    if (outp->err) {
        R_FAIL_EXIT_IF(udp_sender_send((u8 *)"RSG ERR\n", 8) != 8,
                      E_INVALID_CLIENT_REPLY);

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


static Result play_impl(struct output *outp) {
    (void)outp;

    R_FAIL_RETURN(EXIT_FAILURE, StartGame() == EXIT_FAILURE, E_FAILED_SERIAL_READ);

    const char *word = GetCurWord();
    char *tok;

    tok = BufTokenizeOpt(outp->next, " ", &outp->next);

    R_FAIL_RETURN(EXIT_FAILURE, strlen(tok) > 1, E_INVALID_CLIENT_REPLY);
    char ch = (char)tolower(*tok);
    R_FAIL_RETURN(EXIT_FAILURE, (toupper(ch) > 'Z' || toupper(ch) < 'A'), E_INVALID_CLIENT_REPLY);

    size_t out;
    R_FAIL_RETURN(EXIT_FAILURE, final_num(outp->next, &out), E_INVALID_CLIENT_REPLY);

    if (out != GameRegTrial()) {
        tok = "INV";
        goto no_work;
    }

    if (g_serv_game->errors == g_serv_game->max_errors) {
        tok = "OVR";
        goto no_work;
    }

    u32 i = 0;
    for(; i < 40 && g_serv_game->letter_guess[i].letter != 0; ++i) {
        if (g_serv_game->letter_guess[i].letter == ch) {
            tok = "DUP";
            goto no_work;
        }
    }
    g_serv_game->letter_guess[i].letter = ch;

    /* Map with each pos. */
    u32 data[31];
    u32 len = strlen(word);
    u32 cw = 0;
    for(u32 i = 0; i < len; ++i) {
        if (toupper(word[i]) == toupper(ch)) {
            data[cw++] = i+1;
            g_serv_game->word_state[i] = ch;
        }
    }

    if (cw == 0) {
        g_serv_game->errors++;
        puts("buuuut");
        tok = "NOK";
        goto no_work;
    }

    
    #define suc_buf_sz (STR_SIZEOF("RLG NOK 9 00 ") + STR_SIZEOF("00 ") * 30 - sizeof(char) + sizeof("\n"))
    char suc_buf[suc_buf_sz];

    // RLG OK 1 2 3 6

    snprintf(suc_buf, suc_buf_sz, "RLG OK %u %u ", GameTrials(), cw);
    char *p = suc_buf + strlen(suc_buf);
    for (u32 i = 0; i < cw; ++i) {
        sprintf(p, "%u ", data[i]);
        p += strlen(p);
    }
    *(p-1) = '\n';


    printf("lol %s\n", suc_buf);
    const size_t suc_sz = strlen(suc_buf); 
    R_FAIL_RETURN(EXIT_FAILURE, (size_t)udp_sender_send((u8 *)suc_buf, strlen(suc_buf)) != suc_sz, E_FAILED_REPLY);
    return EXIT_SUCCESS;

no_work:
    #define fmt_sz (sizeof("RLG NOK 1\n"))
    char send_buf[fmt_sz];

    snprintf(send_buf, fmt_sz, "RLG %s %u\n", tok, GameTrials());
    const size_t send_buf_sz = strlen(send_buf);
    R_FAIL_RETURN(EXIT_FAILURE, (size_t)udp_sender_send((u8 *)send_buf, send_buf_sz) != send_buf_sz, E_FAILED_REPLY);

    return EXIT_SUCCESS;
    #undef fmt_sz
    #undef suc_buf_sz
}

Result command_play(struct output *outp) {
    Result rc = EXIT_SUCCESS;

    if (outp->err)
        goto out;

    if ((rc = GameAcquire(outp->plid)) == EXIT_FAILURE) {
        perror(E_ACQUIRE_ERROR);
        goto out;
    }

    if ((rc = play_impl(outp)) == EXIT_SUCCESS) {
        rc = ExitAndSerializeGame();
    } else {
        goto out;
    } 

    R_FAIL_EXIT_IF(GameRelease() == EXIT_FAILURE, E_RELEASE_ERROR);
    exit(rc);

out:
    R_FAIL_EXIT_IF(udp_sender_send((u8 *)"RSG ERR\n", 8) != 8,
                      E_FAILED_REPLY);

    exit(EXIT_SUCCESS);
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

static Result state_impl(struct output *outp) {
    u8 *r_buf = malloc(0xA000);
    if(StartGame() == EXIT_FAILURE)
        goto no_work;

    char * const a_buf = (char *)r_buf + 0x1000;
    char *buf = a_buf;

    sprintf(buf, "     Active game found for player %06zu\n", outp->plid);
    buf += strlen(buf);
    sprintf(buf, "     -- Transactions found: %u ---\n", g_serv_game->trials);
    buf += strlen(buf);

    char ch;
    for(u32 i = 0; i < 40 && (ch = g_serv_game->letter_guess[i].letter) != 0; ++i) {
        sprintf(buf, "     Letter trial: %c\n", ch);
        buf += strlen(buf);
    }

    const size_t file_sz = strlen(a_buf);
    buf[0] = '\n';
    buf[1] = '\x00';

    char dat[0x1000];
    sprintf(dat, "RST ACT STATE_%06zu %zu ", outp->plid, file_sz);

    const size_t diff = strlen(dat);
    u8 *send_buf = (u8 *)a_buf - diff;
    memcpy(send_buf, dat, diff);

    VerbosePrintF("Sending the data!\n");
    if(tcp_sender_send(send_buf, (size_t)strlen((char *)send_buf)) == -1) {
        goto no_work;
    }

    free(r_buf);
    return EXIT_SUCCESS;

no_work:
    free(r_buf);
    return EXIT_FAILURE;
}

Result command_state(struct output *outp) {
    Result rc = EXIT_SUCCESS;

    if (outp->err)
        goto out;

    if ((rc = GameAcquire(outp->plid)) == EXIT_FAILURE) {
        perror(E_ACQUIRE_ERROR);
        goto out;
    }
    
    
    if ((rc = state_impl(outp)) == EXIT_FAILURE) {
        goto out;
    } 

    R_FAIL_EXIT_IF(GameRelease() == EXIT_FAILURE, E_RELEASE_ERROR);
    exit(rc);

out:
    R_FAIL_EXIT_IF(tcp_sender_send((u8 *)"STA ERR\n", 8) != 8,
                      E_FAILED_REPLY);

    exit(EXIT_FAILURE);
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
