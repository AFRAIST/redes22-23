#include "rcwg.h"
#include "command_processor.h"

char GSport_GSip_reader(int argc, char **argv, char **GSip, char **GSport) {

    int option_val;
    char validation = true;

    while ((option_val = getopt(argc, argv, "n:p:")) != -1) {
        switch (option_val) {
        case 'n':
            if((*GSip) == NULL)
                (*GSip) = strdup(optarg);
            else validation = false;
            break;

        case 'p':
            if((*GSport) == NULL)
                (*GSport) = strdup(optarg);
            else validation = false;
            break;
        }
    }
    if(validation == false){
        printf("Please Enter only 1 IP or Port\n");
        return EXIT_FAILURE;
    }
    if ((*GSip) == NULL) {
        (*GSip) = DEFAULT_IP;
    }
    if ((*GSport) == NULL) {
        (*GSport) = DEFAULT_PORT;
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

    //validate_ip();
    //validate_port();

    command_reader();

    free(GSip);
    free(GSport);

}
