#include "udp_sender.h"

int socket_udp_fd = -1;
struct addrinfo *serv_data;

extern char *GSip;
extern char *GSport;

#define CUR_IP_VAR (GSip)
#define CUR_PORT_VAR (GSport)

#include "udp_instance.h"