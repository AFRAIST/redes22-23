#include "rcwg.h"

char get_plid(char* appendix, char plid[7]){

    int appendix_size = strlen(appendix);
    int i = 0;

    if(appendix_size > 6 || appendix_size <= 0) return EXIT_FAILURE;

    for(i = 0; i < PLID_MAX_SIZE; i++){
        if(appendix_size - PLID_MAX_SIZE + i < 0){
            plid[i] = '0';
        }
        else{
            if(appendix[appendix_size - PLID_MAX_SIZE + i] >= '0' && appendix[appendix_size - PLID_MAX_SIZE + i] <= '9'){
                plid[i] = appendix[appendix_size - PLID_MAX_SIZE + i];
            }else{
                plid[0] = '\0';
                return EXIT_FAILURE;
            }
        }
    }
    plid[i] = '\0';
    return EXIT_SUCCESS;
}