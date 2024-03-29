#include "Dictionary.h"
#include "command_processor.h"
#include "rcwg.h"

char *player_ip = NULL;
char *player_port = NULL;
char *GSport = NULL;

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


FILE *word_file = NULL;
static void sig_exit_parent() { 
    if (GSport != NULL)
        free(GSport);
    FiniDictionary(&dict_instance);

    if (word_file == NULL)
        fclose(word_file);

    #ifdef FOR_TEST
    extern int *shmptr;
    extern int g_shmid;
    extern int g_shm_fd;

    // Detach and destroy the shared memory segment
    shmdt(shmptr);
    
    shmctl(g_shmid, IPC_RMID, NULL);
    
    // Close the file descriptor
    handle_fd_close(g_shm_fd);
    #endif

    fflush(stderr);
    fflush(stdout);

    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    ignore_signals();

    if (GSport_GSip_reader(argc, argv, &word_file, &GSport) == EXIT_FAILURE) {
        printf("ABORT\n");
        exit(EXIT_FAILURE);
    }

    /* Init randomizer. */
    srand(time(NULL));

    signal(SIGINT, sig_exit_parent);
    
    InitDictionary(&dict_instance, word_file);

    VerbosePrintF("%s\n", GSport);

    command_reader();

    free(GSport);
    FiniDictionary(&dict_instance);
}