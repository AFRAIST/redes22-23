#include "rcwg.h"
#include "udp_sender.h"

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
        (*GSip) = strdup(DEFAULT_IP);
    }
    if ((*GSport) == NULL) {
        (*GSport) = strdup(DEFAULT_PORT);
    }
    return EXIT_SUCCESS;
}


char process_buffer(char buffer[],struct input *inp){

    int i = 0;
    int prev_i = 0;
    for(i = 0; i <= 10 && buffer[i] != ' ' && buffer[i] != '\n'; i++){
        if(i == 10)
            return EXIT_FAILURE;
        inp->command[i] = buffer[i];
    }
    inp->command[i] = '\0';
    prev_i = i + 1;
    if(buffer[i] == ' '){
        for(i=0;i <= 30 && buffer[prev_i + i] != ' ' && buffer[prev_i + i] != '\n'; i++){
            if(i == 30)
                return EXIT_FAILURE;
            inp->appendix[i] = buffer[prev_i + i];
        }
        inp->appendix[i] = '\0';
    }
    return EXIT_SUCCESS;
}

char command_reader(){
    
    INIT_INPUT(inp);

    char buffer[41];

    if(fgets(buffer, 41 , stdin) == NULL){
        printf("Como é que conseguiste fazer porcaria");
    };

    
    if(process_buffer(buffer, &inp) == EXIT_FAILURE){
        printf("Nao sabes ecrever? vai lere o enunciado! esse comando nao é valido :angry face:\n");
    };

    while(!COND_COMP_STRINGS_1(inp.command, "quit")){
        if(COND_COMP_STRINGS_2(inp.command, "start", "sg")){
            if(start_command(&inp) == EXIT_FAILURE)
                printf("Input Invalido :c");
        }
        else if(COND_COMP_STRINGS_2(inp.command, "play", "pl"))
            printf("Sucess! pl\n");
        else if(COND_COMP_STRINGS_2(inp.command, "guess", "gw"))
            printf("Sucess! gw\n");
        else if(COND_COMP_STRINGS_2(inp.command, "scoreboard", "sb"))
            printf("Sucess! sb\n");
        else if(COND_COMP_STRINGS_2(inp.command, "hint", "h"))
            printf("Sucess! h\n");
        else if(COND_COMP_STRINGS_2(inp.command, "state", "st"))
            printf("Sucess! st\n");
        if(fgets(buffer, 41 , stdin) == NULL){
            printf("Como é que conseguiste fazer porcaria");
        };
        if(process_buffer(buffer, &inp) == EXIT_FAILURE){
            printf("Nao sabes ecrever? vai ler o enunciado! esse comando nao é valido :angry face:\n");
        };
    }

    return 0;
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
