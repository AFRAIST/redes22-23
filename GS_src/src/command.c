#include "command.h"
#include "Dictionary.h"

Result command_start(struct output *outp) {
    (void)outp;
    printf("%s\n", random_word(&dict_instance));
    return EXIT_SUCCESS;
}

Result command_play(struct output *outp) {
    int plid, trials;
    char letter;
    printf("AAAAAAAA\n");
#define OPT_NUM 3
    char *opts[OPT_NUM + 1] = {NULL};

    BufTokenizeOpts(outp->buff, opts, COMMAND_BUF_SZ);

    sscanf(outp->buff, "%i %c %i\n", &plid, &letter, &trials);
    printf("%i %c %i\n", plid, letter, trials);
    return EXIT_SUCCESS;
#undef OPT_NUM
}

Result command_guess(struct output *outp) {
    int plid, trials;
    char word[31];
    printf("AAAAAAAA\n");
    sscanf(outp->buff, "%i %s %i\n", &plid, word, &trials);
    printf("%i %s %i\n", plid, word, trials);
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
