#include "command.h"
#include "Dictionary.h"
#include "serv_game.h"
#include "udp_sender.h"
#include "tcp_sender.h"
#include "score.h"

static inline u32 get_errs(u32 size) {
    if (size < 7)
        return 7;
    else if (size < 11)
        return 8;

    return 9;
}

static Result start_impl(struct output *outp) {
    (void)outp;
    R_FAIL_RETURN(EXIT_FAILURE, StartGame() == EXIT_FAILURE, E_FAILED_SERIAL_READ);
    
    const char *w = GetCurWord();
    const u32 size = strlen(w);
    const u32 errs = get_errs(size);

    g_serv_game->max_errors = errs;

    const size_t send_buf_sz = 0x1000;
    char r_send_buf[send_buf_sz];
    char *send_buf = r_send_buf;

    if(g_serv_game->finished == 0){
    if (GameTrials() != 0 ) {
        send_buf = "RSG NOK\n";
    } else {    
        snprintf(send_buf, send_buf_sz, "RSG OK %u %u\n",
        size,
        errs);
    }
    } else {
        if (RemoveFile(outp->plid) == EXIT_FAILURE) {
            perror("Failed to remove file\n");
            return EXIT_FAILURE;
        }
        if (GameAcquire(outp->plid) == EXIT_FAILURE) {
            perror(E_ACQUIRE_ERROR);
            return EXIT_FAILURE;
        }
        if(start_impl(outp) == EXIT_FAILURE){
            return EXIT_FAILURE;
        };
        return EXIT_SUCCESS;
    }

    
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
        goto out;
    }

    if (GameAcquire(outp->plid) == EXIT_FAILURE) {
        perror(E_ACQUIRE_ERROR);
        goto out;
    }

    Result rc;
    if((rc = start_impl(outp)) == EXIT_SUCCESS) {
        rc = ExitAndSerializeGame();
    } else {
        goto out_release;
    }

    R_FAIL_EXIT_IF(GameRelease() == EXIT_FAILURE, E_RELEASE_ERROR);
    exit(rc);

out_release:
    R_FAIL_EXIT_IF(GameRelease() == EXIT_FAILURE, E_RELEASE_ERROR);
out:
    R_FAIL_EXIT_IF(udp_sender_send((u8 *)"RSG ERR\n", 8) != 8,
                  E_FAILED_REPLY);

    exit(EXIT_FAILURE);
}


static Result play_impl(struct output *outp) {
    R_FAIL_RETURN(EXIT_FAILURE, StartGame() == EXIT_FAILURE, E_FAILED_SERIAL_READ);

    if (g_serv_game->finished != 0) {
        return EXIT_FAILURE;
    }

    const char *word = GetCurWord();
    char *tok;

    tok = BufTokenizeOpt(outp->next, " ", &outp->next);

    R_FAIL_RETURN(EXIT_FAILURE, strlen(tok) > 1, E_INVALID_CLIENT_REPLY);
    char ch = (char)tolower(*tok);
    R_FAIL_RETURN(EXIT_FAILURE, (toupper(ch) > 'Z' || toupper(ch) < 'A'), E_INVALID_CLIENT_REPLY);

    size_t out;
    R_FAIL_RETURN(EXIT_FAILURE, final_num(outp->next, &out), E_INVALID_CLIENT_REPLY);

    u32 i = 0;
    for(; i < 40 && g_serv_game->letter_guess[i].letter != 0; ++i) {
        if (g_serv_game->letter_guess[i].letter == ch) {
            if (g_serv_game->word_guess[i+1].word != 0) {                            
                if (out != GameTrials()+1) {
                    tok = "INV";
                    goto no_work;
                }
                
                tok = "DUP";
                goto no_work;
            } else {
                if (g_serv_game->last_was_incremental) {
                    GameUnregTrial();
                    goto _continue;
                }
            }
        }
    }

    if (out != GameTrials()+1) {
        tok = "INV";
        goto no_work;
    }

_continue:
    g_serv_game->letter_guess[i].letter = ch;

    /* Map with each pos. */
    u32 data[31];
    u32 len = strlen(word);
    u32 cw = 0;
    u32 fespace = 0;
    for(u32 i = 0; i < len; ++i) {
        if (toupper(word[i]) == toupper(ch)) {
            data[cw++] = i+1;
            g_serv_game->word_state[i] = ch;
        }
        if(g_serv_game->word_state[i] == '-') fespace++;
    }
    if(fespace == 0){
        g_serv_game->finished = 1;
        tok = "WIN";
        save_score(outp, word);
        goto no_work;
    }

    if (cw == 0) {
        g_serv_game->errors++;
        if (g_serv_game->errors > g_serv_game->max_errors) {
            g_serv_game->finished = 2;
            tok = "OVR";
            goto no_work;
        }
        GameRegTrial();
        tok = "NOK";
        goto no_work_nok;
    }

    
    #define suc_buf_sz 0x1000
    char suc_buf[suc_buf_sz];

    // RLG OK 1 2 3 6

    GameRegTrial();
    snprintf(suc_buf, suc_buf_sz, "RLG OK %u %u ", GameTrials(), cw);
    char *p = suc_buf + strlen(suc_buf);
    for (u32 i = 0; i < cw; ++i) {
        sprintf(p, "%u ", data[i]);
        p += strlen(p);
    }
    *(p-1) = '\n';


    const size_t suc_sz = strlen(suc_buf); 
    
    if ((size_t)udp_sender_send((u8 *)suc_buf, suc_sz) != suc_sz) {
        perror(E_FAILED_REPLY);
        g_serv_game->last_was_incremental = false;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

no_work:
    g_serv_game->last_was_incremental = false;
no_work_nok:
    ;
    char send_buf[0x1000];

    sprintf(send_buf, "RLG %s %u\n", tok, GameTrials());
    const size_t send_buf_sz = strlen(send_buf);
    if ((size_t)udp_sender_send((u8 *)send_buf, send_buf_sz) != send_buf_sz) {
        perror(E_FAILED_REPLY); 
        g_serv_game->last_was_incremental = false;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
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
    R_FAIL_EXIT_IF(udp_sender_send((u8 *)"RLG ERR\n", 8) != 8,
                      E_FAILED_REPLY);

    exit(EXIT_FAILURE);
}

static Result guess_impl(struct output *outp) {
    R_FAIL_RETURN(EXIT_FAILURE, StartGame() == EXIT_FAILURE, E_FAILED_SERIAL_READ);

    if (g_serv_game->finished != 0)
        return EXIT_FAILURE;
    
    const char *word = GetCurWord();
    char *tok;
    char word_guessed[31];

    tok = BufTokenizeOpt(outp->next, " ", &outp->next);

    R_FAIL_RETURN(EXIT_FAILURE, strlen(tok) > 30, E_INVALID_CLIENT_REPLY);
    u32 i = 0;
    for(; i < strlen(tok); i++){
        word_guessed[i] = (char)tolower(tok[i]);
        R_FAIL_RETURN(EXIT_FAILURE, (toupper(word_guessed[i]) > 'Z' || toupper(word_guessed[i]) < 'A'), E_INVALID_CLIENT_REPLY);
    }
    word_guessed[i] = '\x00';

    VerbosePrintF("Guessed word: %s\n", word);

    size_t out;
    R_FAIL_RETURN(EXIT_FAILURE, final_num(outp->next, &out), E_INVALID_CLIENT_REPLY);


    i = 0;
    u32 guessed_word_size = strlen(word_guessed); 
    for(; i < 40 && g_serv_game->word_guess[i].word != 0; ++i) {
        VerbosePrintF("other attempts: %s\n", g_serv_game->word_guess[i].word);
        //isto vai poder ir para uma funcao compare_words
        if(strcasecmp(g_serv_game->word_guess[i].word, word_guessed) == 0) {
            if (g_serv_game->word_guess[i+1].word != 0) {
                if (out != GameTrials()+1) {
                    tok = "INV";
                    goto no_work;
                }
        
                tok = "DUP";
                goto no_work;
            } else {
                if (g_serv_game->last_was_incremental) {
                    GameUnregTrial();
                    goto _continue;
                }
            }
        }
    }
    

    if (out != GameTrials()+1) {
        tok = "INV";
        goto no_work;
    }

_continue:

    g_serv_game->word_guess[i].word = StrSerializeDup(word_guessed);
    VerbosePrintF("New attempted word: %s %i\n", g_serv_game->word_guess[i].word, i);

    /* Map with each pos. */
    u32 equal_verifier = true;
    u32 len = strlen(word);
    if(guessed_word_size != len)
            equal_verifier = false;
    else {
        if(strcasecmp(word, word_guessed) != 0){
            equal_verifier = false;
        }
    }

    if (equal_verifier == false) {
        g_serv_game->errors++;

        if (g_serv_game->errors == g_serv_game->max_errors) {
            g_serv_game->finished = 2;
            tok = "OVR";
            goto no_work;
        }
        tok = "NOK";
        GameRegTrial();
        goto no_work_nok;
    }

    
    #define suc_buf_sz (0x1000)
    char suc_buf[suc_buf_sz];

    GameRegTrial();
    g_serv_game->finished = 1;
    snprintf(suc_buf, suc_buf_sz, "RWG WIN %u\n", GameTrials());
    save_score(outp, word);

    VerbosePrintF("Sent message: %s\n", suc_buf);
    const size_t suc_sz = strlen(suc_buf); 
    if ((size_t)udp_sender_send((u8 *)suc_buf, strlen(suc_buf)) != suc_sz) {
        perror(E_FAILED_REPLY);
        g_serv_game->last_was_incremental = false;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
no_work:
    g_serv_game->last_was_incremental = false;
no_work_nok:
    ;
    #define fmt_sz (0x1000)
    char send_buf[fmt_sz];

    snprintf(send_buf, fmt_sz, "RWG %s %u\n", tok, GameTrials());
    const size_t send_buf_sz = strlen(send_buf);
    if ((size_t)udp_sender_send((u8 *)send_buf, send_buf_sz) != send_buf_sz) {
        perror(E_FAILED_REPLY);
        g_serv_game->last_was_incremental = false;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
    #undef fmt_sz
    #undef suc_buf_sz
}

Result command_guess(struct output *outp) {
    Result rc = EXIT_SUCCESS;

    if (outp->err)
        goto out;

    if ((rc = GameAcquire(outp->plid)) == EXIT_FAILURE) {
        perror(E_ACQUIRE_ERROR);
        goto out;
    }

    if ((rc = guess_impl(outp)) == EXIT_SUCCESS) {
        rc = ExitAndSerializeGame();
    } else {
        goto out_release;
    } 

    R_FAIL_EXIT_IF(GameRelease() == EXIT_FAILURE, E_RELEASE_ERROR);
    exit(rc);

out_release:
    if (GameRelease() == EXIT_FAILURE) {
        perror(E_RELEASE_ERROR);
    }

out:
    R_FAIL_EXIT_IF(udp_sender_send((u8 *)"RWG ERR\n", 8) != 8,
                      E_FAILED_REPLY);

    exit(EXIT_SUCCESS);
}

Result scoreboard_impl(){
    static u8 _r_buf[0xA000];
    u8 *r_buf = _r_buf;
    //R_FAIL_RETURN(EXIT_FAILURE, StartGame() == EXIT_FAILURE, E_FAILED_SERIAL_READ);
    
    if(count_scores() == 0){
        printf("%i", count_scores());
        if (tcp_sender_send((u8 *)"RSB EMPTY\n", 10) != 10) {
            perror(E_FAILED_REPLY);
            return EXIT_SUCCESS;
        }

        if (tcp_sender_fini() == -1) perror("[ERR] Closing TCP.\n"); \
        exit(EXIT_SUCCESS);
    }

    char * const a_buf = (char *)r_buf + 0x1000;
    char *buf = a_buf;

    ScoreEntry scoreboard_list[TOP_SCORE];

    if(get_scoreboard(scoreboard_list) == EXIT_FAILURE){
        perror("[ERR] Getting Scoreboard"); 
        return EXIT_FAILURE;
    }
    
    sprintf(buf, "-------------------------------- TOP 10 SCORES --------------------------------\n\n");
    buf += strlen(buf);

    sprintf(buf, "    SCORE PLAYER     WORD                             GOOD TRIALS  TOTAL TRIALS\n\n");
    buf += strlen(buf);

    for(u32 i = 0; i < total_scores; i++){
        sprintf(buf," %i - %s\n",i + 1, scoreboard_list[i].score_str);
        buf += strlen(buf);
    }

    const size_t file_sz = strlen(a_buf);
    buf[0] = '\n';
    buf[1] = '\x00';

    char dat[0x1000];
    sprintf(dat, "RSB OK Scoreboard %zu ",  file_sz);

    const size_t diff = strlen(dat);
    u8 *send_buf = (u8 *)a_buf - diff;
    memcpy(send_buf, dat, diff);

    VerbosePrintF("Sending the data!\n");
    if(tcp_sender_send(send_buf, (size_t)strlen((char *)send_buf)) == -1) {
        goto no_work;
    }

    return EXIT_SUCCESS;

no_work:
    return EXIT_FAILURE;
}

Result command_scoreboard(struct output *outp) {
    Result rc = EXIT_SUCCESS;

    if (outp->err)
        goto out;

    if ((rc = scoreboard_impl()) == EXIT_FAILURE) {
        goto out;
    } 

    if (tcp_sender_fini() == -1) perror("[ERR] Closing TCP.\n");
    exit(rc);

out:
    R_FAIL_EXIT_IF(tcp_sender_send((u8 *)"RSB ERR\n", 8) != 8,
                      E_FAILED_REPLY);

    if (tcp_sender_fini() == -1) perror("[ERR] Closing TCP.\n");

    exit(EXIT_FAILURE);
}


int __hint_fd = -1;
static Result hint_impl(struct output *outp) {
    (void)outp;
    R_FAIL_RETURN(EXIT_FAILURE, StartGame() == EXIT_FAILURE, E_FAILED_SERIAL_READ);

    static u8 _r_buf[0x1000 + 4*1024*1024];
    u8 *r_buf = _r_buf;

    const char * const c = dict_instance.entries[g_serv_game->cur_entry].word_class;
    char path[0x1000];

    snprintf(path, 0x1000, "assets/%s", c); 

    VerbosePrintF("Path: %s.\n", path);
    __hint_fd = open(path, O_RDONLY);

    if (flock(__hint_fd, LOCK_SH) == -1) {
        perror("[ERR] Failed to flock.");
        goto error;
    }

    struct stat sb;
    if (fstat(__hint_fd, &sb) == -1) {
        perror("[ERR] Failed to stat.");
        goto error;
    }

    size_t full_size = sb.st_size;

    u8 *buf = r_buf + 0x1000;
    u32 sz;
    if((s32)(sz = read(__hint_fd, buf, 4 * 1024 * 1024)) == -1)
        goto error;


    char dat[0x1000];
    sprintf(dat, "RHL OK %s %zu ", c, full_size);

    const size_t diff = strlen(dat);
    u8 *send_buf = (u8 *)buf - diff;
    memcpy(send_buf, dat, diff);

    if (tcp_sender_send(send_buf, diff+sz) == -1)
        goto error;

    full_size -= sz;
    while (full_size != 0) {
        if((s32)(sz = read(__hint_fd, buf, 4 * 1024 * 1024)) == -1) {
            perror(E_FAILED_REPLY);
            goto skip;
        }

        if (tcp_sender_send(buf, sz) == -1) {
            perror(E_FAILED_REPLY);
            goto skip;
        }
        
        full_size -= sz;
    }
    
    if (tcp_sender_send((u8 *)"\n", 1) == -1)
        perror(E_FAILED_REPLY);

skip:
    if (handle_fd_close(__hint_fd) == -1) {
        perror("[ERR] Failed to close file.\n");
    }
    return EXIT_SUCCESS;

error:
    perror(E_FAILED_REPLY);

    if (handle_fd_close(__hint_fd) == -1) {
        perror("[ERR] Failed to close file.\n");
    }

    return EXIT_FAILURE;
}

Result command_hint(struct output *outp) {
    Result rc = EXIT_SUCCESS;
    
    if (outp->err)
        goto out;

    if ((rc = GameAcquire(outp->plid)) == EXIT_FAILURE) {
        perror(E_ACQUIRE_ERROR);
        goto out;
    }

    bool game_empty;
    if ((rc = GameEmpty(&game_empty)) == EXIT_FAILURE) {
        perror(E_ACQUIRE_ERROR);
        goto out_release;
    }
    
    if (game_empty) { 
        if (tcp_sender_send((u8 *)"RHL NOK\n", 8) != 8) {
            perror(E_FAILED_REPLY);
        }

        if (tcp_sender_fini() == -1) perror("[ERR] Closing TCP.\n");
        exit(EXIT_SUCCESS);
    }

    if ((rc = hint_impl(outp)) == EXIT_FAILURE) {
        goto out;
    } 
    
    if (GameRelease() == EXIT_FAILURE) {
        perror(E_RELEASE_ERROR);
    }

    if (tcp_sender_fini() == -1) perror("[ERR] Closing TCP.\n");
    exit(rc);

out_release:
    if (GameRelease() == EXIT_FAILURE) {
        perror(E_RELEASE_ERROR);
    }

out:
    if(tcp_sender_send((u8 *)"ERR\n", 4) != 4) {
        perror(E_FAILED_REPLY);
    }

    if (tcp_sender_fini() == -1) perror("[ERR] Closing TCP.\n");
    exit(EXIT_FAILURE);
}

static Result state_impl(struct output *outp) {
    static u8 _r_buf[0xA000];
    u8 *r_buf = _r_buf;
    
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

    char *c_word;
    for(u32 i = 0; i < 40 && (c_word = g_serv_game->word_guess[i].word) != NULL; ++i) {
        sprintf(buf, "     Word guess: %s\n", c_word);
        buf += strlen(buf);
    }

    sprintf(buf, "     Solved so far: %s\n", g_serv_game->word_state);
    buf += strlen(buf);

    if (g_serv_game->finished != 0) {
        static const char *terms[] = {NULL, "WIN", "GAME OVER", "QUIT"};
        sprintf(buf, "     Termination: %s\n", terms[g_serv_game->finished]);
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

    return EXIT_SUCCESS;

no_work:
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

    bool game_empty;
    if ((rc = GameEmpty(&game_empty)) == EXIT_FAILURE) {
        perror(E_ACQUIRE_ERROR);
        goto out_release;
    }

    if (game_empty) {
        if (tcp_sender_send((u8 *)"STA NOK\n", 8) != 8) {
            perror(E_FAILED_REPLY);
        }

        if (tcp_sender_fini() == -1) perror("[ERR] Closing TCP.\n");
        exit(EXIT_SUCCESS);
    }

    if ((rc = state_impl(outp)) == EXIT_FAILURE) {
        goto out_release;
    } 

    R_FAIL_EXIT_IF(GameRelease() == EXIT_FAILURE, E_RELEASE_ERROR);
    if (tcp_sender_fini() == -1) perror("[ERR] Closing TCP.\n");
    exit(rc);

out_release:
    if (GameRelease() == EXIT_FAILURE) {
        perror(E_RELEASE_ERROR);
    }

out:
    if (tcp_sender_send((u8 *)"ERR\n", 4) != 4) {
        perror(E_FAILED_REPLY);
    }
    
    if (tcp_sender_fini() == -1) perror("[ERR] Closing TCP.\n");
    exit(EXIT_FAILURE);
}

static Result quit_impl(struct output *outp) {
    (void)outp;
    R_FAIL_RETURN(EXIT_FAILURE, StartGame() == EXIT_FAILURE, E_FAILED_SERIAL_READ);

    char *p;

    bool empty;
    if (GameEmpty(&empty) == EXIT_FAILURE)
        return EXIT_FAILURE;

    p = empty ? "RQT NOK\n" : "RQT OK\n";

    if(!empty) {
        g_serv_game->finished = 3;
        R_FAIL_RETURN(EXIT_FAILURE, ExitAndSerializeGame() == EXIT_FAILURE, "[ERR] Failed serial write.\n");
    }

    const size_t suc_sz = strlen(p);
    if((size_t)udp_sender_send((u8 *)p, suc_sz) != suc_sz) {
        perror(E_FAILED_REPLY);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

Result command_quit(struct output *outp) {
    Result rc = EXIT_SUCCESS;
    
    if (outp->err) {
        goto out;
    }
    
    if ((rc = GameAcquire(outp->plid)) == EXIT_FAILURE) {
        perror(E_ACQUIRE_ERROR);
        goto out;
    }
    
    if ((rc = quit_impl(outp)) == EXIT_FAILURE) {
        goto out_release;
    } 
    
    R_FAIL_EXIT_IF(GameRelease() == EXIT_FAILURE, E_RELEASE_ERROR);
    exit(rc);

out_release:
    if (GameRelease() == EXIT_FAILURE) {
        perror(E_RELEASE_ERROR);
    }

out:
    R_FAIL_EXIT_IF(udp_sender_send((u8 *)"RQT ERR\n", 8) != 8,
                  E_FAILED_REPLY);

    exit(EXIT_FAILURE);
}
