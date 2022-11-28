#include "rcwg.h"

char get_plid(struct input *inp){

    int appendix_size = strlen(inp->appendix);
    int i = 0;

    if(appendix_size > 6 || appendix_size <= 0) return EXIT_FAILURE;

    for(i = 0; i < PLID_MAX_SIZE; i++){
        if(appendix_size - PLID_MAX_SIZE + i < 0){
            inp->plid[i] = '0';
        }
        else{
            if(inp->appendix[appendix_size - PLID_MAX_SIZE + i] >= '0' 
            && inp->appendix[appendix_size - PLID_MAX_SIZE + i] <= '9'){
                inp->plid[i] = inp->appendix[appendix_size - PLID_MAX_SIZE + i];
            }else{
                inp->plid[0] = '\0';
                return EXIT_FAILURE;
            }
        }
    }
    inp->plid[i] = '\0';
    return EXIT_SUCCESS;
}

char start_command(struct input *inp){
    if(inp->plid_exists == false){
        if(get_plid(inp) == EXIT_FAILURE){
            printf("Nada fixe :c");
            return EXIT_FAILURE;
        } else {
            printf("%s c:\n", inp->plid);
            inp->plid_exists = true;

        }
    }
    else{
        printf("Jogador ja existente");
        return EXIT_FAILURE;
    }

    //falta pedir request para ter os max errors
    return EXIT_SUCCESS;
}