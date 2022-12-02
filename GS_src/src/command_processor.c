#include "command_processor.h"
#include "rcwg.h"

int init_server(struct addrinfo *hints, struct addrinfo **res, char* GSport){
    int errcode, fd, n;
    fd=socket(AF_INET,SOCK_DGRAM,0); //UDP socket
    if(fd==-1) /*error*/exit(1);

    memset(hints,0,sizeof(*hints));
    hints->ai_family=AF_INET; // IPv4
    hints->ai_socktype=SOCK_DGRAM; // UDP socket
    hints->ai_flags=AI_PASSIVE;

    errcode=getaddrinfo(NULL,GSport,hints,res);
    if(errcode!=0) /*error*/ exit(1);
    n=bind(fd,(*res)->ai_addr, (*res)->ai_addrlen);
    if(n==-1) /*error*/ exit(1);

    return fd;
}


void command_reader(char* GSport) { 

    int udp_fd, n, i = 0;
    socklen_t addrlen;
    struct addrinfo hints,*res;
    struct sockaddr_in addr;
    char buffer[128];
    char* command;
    struct output outp;

    udp_fd = init_server(&hints, &res, GSport);

    while (1){
        outp.buff = buffer;
        memset(buffer, 'a', 128);
        addrlen=sizeof(addr);
        printf("AAAAAAAAA\n");
        n=recvfrom(udp_fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen);
        if(n==-1)/*error*/exit(1);

        command = outp.buff;
        outp.buff = StrNSplitSpaceNext(outp.buff, 3);
        if(COND_COMP_STRINGS_1("SNG", command))
            command_start(&outp);
        else if(COND_COMP_STRINGS_1("PLG", command)){
            command_play(&outp);
        }
        else if(COND_COMP_STRINGS_1("PWG", command)){
            command_guess(&outp);
        }
        n=sendto(udp_fd,buffer,n,0,
        (struct sockaddr*)&addr,addrlen);
        if(n==-1)/*error*/exit(1);
    }
}