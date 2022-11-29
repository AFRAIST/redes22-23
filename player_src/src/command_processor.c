#include "command_processor.h"
#include "command.h"
#include "udp_sender.h"

static Result process_buffer(char buffer[], struct input *inp) {
#define MAX_CMD_OP1_SZ 10
#define MAX_CMD_OP2_SZ 30

    inp->command = buffer;
    buffer = StrNSplitSpaceNext(buffer, MAX_CMD_OP1_SZ);

    if (buffer == NULL)
        return EXIT_FAILURE;

    inp->appendix = buffer;
    buffer = StrNSplitSpaceNext(buffer, MAX_CMD_OP2_SZ);

    return EXIT_SUCCESS;

#undef MAX_CMD_OP1_SZ
#undef MAX_CMD_OP2_SZ
}

void command_reader() {
#define CUR_BUFFER_SZ 41

    INIT_INPUT(inp);

    char buffer[CUR_BUFFER_SZ];

    if (fgets(buffer, CUR_BUFFER_SZ, stdin) == NULL) {
        printf("Como é que conseguiste fazer porcaria");
    };

    if (process_buffer(buffer, &inp) == EXIT_FAILURE) {
        printf("Nao sabes ecrever? vai lere o enunciado! esse comando nao é "
               "valido :angry face:\n");
    };

    while (!COND_COMP_STRINGS_1(inp.command, "quit")) {
        if (COND_COMP_STRINGS_2(inp.command, "start", "sg")) {
            if (command_start(&inp) == EXIT_FAILURE)
                printf("Input Invalido :c");
        } else if (COND_COMP_STRINGS_2(inp.command, "play", "pl"))
            printf("Sucess! pl\n");
        else if (COND_COMP_STRINGS_2(inp.command, "guess", "gw"))
            printf("Sucess! gw\n");
        else if (COND_COMP_STRINGS_2(inp.command, "scoreboard", "sb"))
            printf("Sucess! sb\n");
        else if (COND_COMP_STRINGS_2(inp.command, "hint", "h"))
            printf("Sucess! h\n");
        else if (COND_COMP_STRINGS_2(inp.command, "state", "st"))
            printf("Sucess! st\n");
        if (fgets(buffer, CUR_BUFFER_SZ, stdin) == NULL) {
            printf("Como é que conseguiste fazer porcaria");
        };
        if (process_buffer(buffer, &inp) == EXIT_FAILURE) {
            printf("Nao sabes ecrever? vai ler o enunciado! esse comando nao é "
                   "valido :angry face:\n");
        };
    }

#undef CUR_BUFFER_SZ
}