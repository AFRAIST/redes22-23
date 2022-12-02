#include "command.h"

Result command_start(struct output *outp) {
    int plid;

    printf("AAAAAAAA\n");
    sscanf(outp->buff,"%i\n", &plid);
    printf("%i", plid);
    return EXIT_SUCCESS;
}

Result command_play(struct output *outp) {
    int plid, trials;
    char letter;
    printf("AAAAAAAA\n");
    sscanf(outp->buff,"%i %c %i\n", &plid, &letter, &trials);
    printf("%i %c %i\n", plid, letter, trials);
    return EXIT_SUCCESS;
}

Result command_guess(struct output *outp) {
    int plid, trials;
    char word[31];
    printf("AAAAAAAA\n");
    sscanf(outp->buff,"%i %s %i\n", &plid, word, &trials);
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
