#include "rcwg.h"

char GSport_GSip_reader(int argc, char **argv, char **GSip, char **GSport) {

    int option_val;

    while ((option_val = getopt(argc, argv, "n:p:")) != -1) {
        switch (option_val) {
        case 'n':
            (*GSip) = strdup(optarg);
            break;

        case 'p':
            (*GSport) = strdup(optarg);
            break;
        }
    }
    if ((*GSip) == NULL) {
        (*GSip) = strdup(DEFAULT_IP);
    }
    if ((*GSport) == NULL) {
        (*GSport) = strdup(DEFAULT_PORT);
    }
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    char *GSip = NULL;
    char *GSport = NULL;

    if (GSport_GSip_reader(argc, argv, &GSip, &GSport) == EXIT_FAILURE) {
        printf("ABORT");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", GSip);
    printf("%s\n", GSport);
}
