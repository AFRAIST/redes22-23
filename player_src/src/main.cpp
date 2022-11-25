#include "content.hpp"
#include <arpa/inet.h>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>



char GSport_GSip_reader(int argc, char **argv, char** GSip, char** GSport){

    int option_val;

    
    while ((option_val = getopt(argc, argv, "n:p:")) != -1)
    {
        switch (option_val)
        {
        case 'n':
            (*GSip) = strdup(optarg);
            break;
        
        case 'p':
            (*GSport) = strdup(optarg);
            break;
        }
    }
    if((*GSip) == NULL){
        (*GSip) = strdup(DEFAULT_IP);
    }
    if((*GSport) == NULL){
        (*GSport) = strdup(DEFAULT_PORT);
    }
    return EXIT_SUCCESS;
}
    
namespace rcwg {
    ALWAYS_INLINE void Main(int argc, char *argv[]) {
        RCWG_UNUSED(argc, argv);
        util::io::PrintF("Hello World from client!\n");
        util::io::SetVerbose();
        util::io::VerbosePrintF("aaaa\n");
        util::io::PrintF("bbba\n");

        char* GSip = NULL;
        char* GSport = NULL;

        if(GSport_GSip_reader(argc, argv, &GSip, &GSport) == EXIT_FAILURE){
            printf("ABORT");
            exit(EXIT_FAILURE);
        }

        printf("%s\n", GSip);
        printf("%s\n", GSport);
    }
}

int main(int argc, char *argv[]) {
    rcwg::Main(argc, argv);
}

