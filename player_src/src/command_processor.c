#include "command_processor.h"
#include "command.h"
#include "udp_sender.h"

static Result process_buffer(char buffer[], size_t sz, struct input *inp) {
#define MAX_CMD_OP1_SZ 10
#define MAX_CMD_OP2_SZ 30

    /* Reset the buffer. */
    memset(buffer, 0, sz);

    /* Parse from stdin. Will not trivially fail. */
    fgets(buffer, sz, stdin);

    /* Fail if first char is null. */
    if(*buffer == '\0')
        return EXIT_FAILURE;

    /* Check if input contains nulls because fgets won't stop at them. */
    if(BufNotContainsInvalidNull(buffer, sz) == EXIT_FAILURE)
        return EXIT_FAILURE;

    /* Parse command logic. */ 
    inp->command = buffer;
    buffer = StrNSplitSpaceNext(buffer, MAX_CMD_OP1_SZ);

    /* Invalid. */
    if (buffer == NULL)
        return EXIT_FAILURE;

    /* 1 op command. */
    if(*buffer == '\0')
        return EXIT_SUCCESS;

    /* Next. */
    inp->appendix = buffer;
    buffer = StrNSplitSpaceNext(buffer, MAX_CMD_OP2_SZ);

    /* Invalid. */
    if(buffer == NULL)
        return EXIT_FAILURE;

    /* There are no commands with 3 ops. */
    if(*buffer != '\0')
        return EXIT_FAILURE;

    /* 2 op command. */
    return EXIT_SUCCESS;

#undef MAX_CMD_OP1_SZ
#undef MAX_CMD_OP2_SZ
}

void command_reader() {
#define CUR_BUFFER_SZ 41

    INIT_INPUT(inp);

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
                printf("Input Invalido :c\n");
        } else if (COND_COMP_STRINGS_2(inp.command, "play", "pl")){
            if (command_play(&inp) == EXIT_FAILURE)
                printf("Input Invalido :c\n");
        }
        else if (COND_COMP_STRINGS_2(inp.command, "guess", "gw")){
            if (command_guess(&inp) == EXIT_FAILURE)
                printf("Input Invalido :c\n");
        }
        else if (COND_COMP_STRINGS_2(inp.command, "scoreboard", "sb"))
            printf("Sucess! sb\n");
        else if (COND_COMP_STRINGS_2(inp.command, "hint", "h"))
            printf("Sucess! h\n");
        else if (COND_COMP_STRINGS_2(inp.command, "state", "st"))
            printf("Sucess! st\n");
        
        while (process_buffer(buffer, CUR_BUFFER_SZ, &inp) == EXIT_FAILURE) {
            printf("Nao sabes ecrever? vai ler o enunciado! esse comando nao é "
                   "valido :angry face:\n");
        };
    } while (!COND_COMP_STRINGS_1(inp.command, "quit"));

#undef CUR_BUFFER_SZ
}