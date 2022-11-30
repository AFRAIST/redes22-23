#include "Dictionary.h"
#include "command_processor.h"
#include "player.h"
#include "rcwg.h"

char *player_ip = NULL;
char *player_port = NULL;

char GSport_GSip_reader(int argc, char **argv, FILE **word_file,
                        char **GSport) {

    int option_val;
    bool validation = true;

    if (argc >= 2) {
        (*word_file) = fopen(argv[1], "rb");
        if ((*word_file) == NULL)
            return EXIT_FAILURE;
    } else
        return EXIT_FAILURE;

    while ((option_val = getopt(argc, argv, "p:v")) != -1) {
        switch (option_val) {
        case 'p':
            if ((*GSport) == NULL)
                (*GSport) = strdup(optarg);
            else
                validation = false;
            break;
        case 'v':
            if (is_verbose == false)
                SetVerbose();
            else
                validation = false;
            break;
        }
    }
    if (validation == false)
        return EXIT_FAILURE;
    if ((*GSport) == NULL) {
        (*GSport) = strdup(DEFAULT_PORT);
    }
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    FILE *word_file = NULL;
    char *GSport = NULL;

    ignore_signals();

    if (GSport_GSip_reader(argc, argv, &word_file, &GSport) == EXIT_FAILURE) {
        printf("ABORT\n");
        exit(EXIT_FAILURE);
    }

    InitDictionary(&dict_instance, word_file);

    VerbosePrintF("%s\n", GSport);

    init_players(g_players, sizeof(g_players) / sizeof(Player));

    command_reader();

    FiniDictionary(&dict_instance);
}
