#include "rcwg.hpp"
#include <arpa/inet.h>
#include <cstdio>

namespace rcwg {

    char GSport_GSip_reader(int argc, char **argv, FILE** word_file, char** GSport){

        int option_val;

        if(argc >= 2){ 
            (*word_file) = fopen(argv[1], "r");
            if((*word_file) == NULL) return EXIT_FAILURE;
        }else return EXIT_FAILURE;

        while ((option_val = getopt(argc, argv, "p:v")) != -1){
            switch (option_val){
                case 'p':
                (*GSport) = strdup(optarg);
                break;
                case 'v':
                util::io::SetVerbose();
                break;
            }
        }
        if((*GSport) == NULL){
            (*GSport) = strdup(DEFAULT_PORT);
        }
        return EXIT_SUCCESS;
    }

    ALWAYS_INLINE void Main(int argc, char *argv[]) {
        RCWG_UNUSED(argc, argv);
        FILE* word_file = NULL;
        char* GSport = NULL;

        if(GSport_GSip_reader(argc, argv, &word_file, &GSport) == EXIT_FAILURE){
            printf("ABORT");
            exit(EXIT_FAILURE);
        }

        util::io::VerbosePrintF("%s\n", GSport);
        fclose(word_file);
    }
}
} // namespace rcwg

int main(int argc, char *argv[]) {
    rcwg::Main(argc, argv);
}

