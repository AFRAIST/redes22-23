#include "rcwg.h"

#include "Dictionary.h"

char GSport_GSip_reader(int argc, char **argv, FILE** word_file, char** GSport){

    int option_val;

    if(argc >= 2){ 
        (*word_file) = fopen(argv[1], "rb");
        if((*word_file) == NULL) return EXIT_FAILURE;
    }else return EXIT_FAILURE;

    while ((option_val = getopt(argc, argv, "p:v")) != -1){
        switch (option_val){
            case 'p':
            (*GSport) = strdup(optarg);
            break;
            case 'v':
            SetVerbose();
            break;
        }
    }
    if((*GSport) == NULL){
        (*GSport) = strdup(DEFAULT_PORT);
    }
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    FILE* word_file = NULL;
    char* GSport = NULL;

    if(GSport_GSip_reader(argc, argv, &word_file, &GSport) == EXIT_FAILURE){
        printf("ABORT");
        exit(EXIT_FAILURE);
    }
    

    InitDictionary(&dict_instance, word_file);

    VerbosePrintF("%s\n", GSport);

    FiniDictionary(&dict_instance);
}

