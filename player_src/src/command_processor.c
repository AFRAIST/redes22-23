#include "command_processor.h"
#include "command.h"
#include "udp_sender.h"
#include "tcp_sender.h"

static Result process_buffer(char buffer[], size_t sz, struct input *inp) {
#define MAX_CMD_OP1_SZ 10
#define MAX_CMD_OP2_SZ 30

    /* Reset the input. */
    memset(inp, 0, sizeof(*inp));

    /* Reset the buffer. */
    memset(buffer, 0, sz);

    /* Parse from stdin. Will not trivially fail. */
    fgets(buffer, sz, stdin);

    /* Fail if first char is null. */
    if (*buffer == '\x00')
        return EXIT_FAILURE;

    /* Check if input contains nulls because fgets won't stop at them. */
    if (BufNotContainsInvalidNull(buffer, sz) == EXIT_FAILURE)
        return EXIT_FAILURE;

    /* Parse command logic. */
    inp->command = buffer;
    buffer = StrNSplitSpaceNext(buffer, MAX_CMD_OP1_SZ);

    /* Invalid. */
    if (buffer == NULL)
        return EXIT_FAILURE;

    /* 1 op command. */
    if (*buffer == '\x00')
        return EXIT_SUCCESS;

    /* Next. */
    inp->appendix = buffer;
    buffer = StrNSplitSpaceNext(buffer, MAX_CMD_OP2_SZ);

    /* Invalid. */
    if (buffer == NULL)
        return EXIT_FAILURE;

    /* There are no commands with 3 ops. */
    if (*buffer != '\x00')
        return EXIT_FAILURE;

    /* 2 op command. */
    return EXIT_SUCCESS;

#undef MAX_CMD_OP1_SZ
#undef MAX_CMD_OP2_SZ
}

static void sig_exit() {
    if (command_exit() == EXIT_FAILURE)
        perror(E_QUIT_SERVER);

    extern int __get_file_fd;
    handle_fd_close(__get_file_fd);

    tcp_sender_fini();
    udp_sender_fini();

    fflush(stderr);
    fflush(stdout);

    exit(EXIT_SUCCESS);
}

void command_reader() {
#define CUR_BUFFER_SZ 41
    signal(SIGINT, sig_exit);

    struct input inp;

    char buffer[CUR_BUFFER_SZ];

    while (process_buffer(buffer, CUR_BUFFER_SZ, &inp) == EXIT_FAILURE) {
        printf("Nao sabes ecrever? vai lere o enunciado! esse comando nao é "
               "valido :angry face:\n");
    };

    errno = 0;
    do {
        errno = 0;
        if (COND_COMP_STRINGS_2(inp.command, "start", "sg")) {
            if (command_start(&inp) == EXIT_FAILURE)
                printf("Could not start the game.\n");
        } else if (COND_COMP_STRINGS_2(inp.command, "play", "pl")) {
            if (command_play(&inp) == EXIT_FAILURE)
                printf("Could not play.\n");
        } else if (COND_COMP_STRINGS_2(inp.command, "guess", "gw")) {
            if (command_guess(&inp) == EXIT_FAILURE)
                printf("Could not get word.\n");
        } else if (COND_COMP_STRINGS_2(inp.command, "scoreboard", "sb")) {
            if (command_scoreboard(&inp) == EXIT_FAILURE)
                printf("Could not get scoreboard.\n");
        } else if (COND_COMP_STRINGS_2(inp.command, "hint", "h")) {
            if (command_hint(&inp) == EXIT_FAILURE)
                printf("Could not get hint.\n");
        } else if (COND_COMP_STRINGS_2(inp.command, "state", "st")) {
            if (command_state(&inp) == EXIT_FAILURE)
                printf("Could not get state.\n");
        } else if (COND_COMP_STRINGS_1(inp.command, "quit"))
            if (command_quit(&inp) == EXIT_FAILURE)
                printf(E_QUIT_SERVER);

        while (process_buffer(buffer, CUR_BUFFER_SZ, &inp) == EXIT_FAILURE) {
            printf("Nao sabes ecrever? vai ler o enunciado! esse comando nao é "
                   "valido :angry face:\n");
        };
    } while (!COND_COMP_STRINGS_1(inp.command, "exit"));

    R_EXIT_IF(EXIT_FAILURE, command_exit(&inp) == EXIT_FAILURE, E_QUIT_SERVER);

    exit(EXIT_SUCCESS);
#undef CUR_BUFFER_SZ
}