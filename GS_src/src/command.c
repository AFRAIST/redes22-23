#include "command.h"
#include "Dictionary.h"
#include "udp_sender.h"
#include "serv_game.h"

static Result start_impl(struct output *outp) {
    (void)outp;
    
    StartGame(); 

    printf("%s\n", GetCurWord());

    return EXIT_SUCCESS;
}

Result command_start(struct output *outp) {
    if (outp->err) {
        R_FAIL_RETURN(EXIT_FAILURE, udp_sender_send((u8 *)"RSG ERR\n", 8) != 8,
                      E_FAILED_REPLY);

        return EXIT_SUCCESS;
    }

    R_FAIL_RETURN(EXIT_FAILURE, GameAcquire(outp->plid) == EXIT_FAILURE, E_ACQUIRE_ERROR);

    const Result rc = start_impl(outp); 
  
    ExitAndSerializeGame();
    R_FAIL_RETURN(EXIT_FAILURE, GameRelease() == EXIT_FAILURE, E_RELEASE_ERROR);
    return rc;
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
