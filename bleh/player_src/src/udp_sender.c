#include "udp_sender.h"

int socket_udp_fd = -1;
struct addrinfo *peer_data;

extern char *GSip;
extern char *GSport;

#define CUR_IP_VAR (GSip)
#define CUR_PORT_VAR (GSport)

ssize_t udp_sender_try_init() {
    if (BRANCH_LIKELY(socket_udp_fd != -1))
        return EXIT_SUCCESS;

    socket_udp_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_udp_fd == -1)
        return -1;
 
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket

    if (getaddrinfo(CUR_IP_VAR, CUR_PORT_VAR, &hints, &peer_data) != 0)
        return -1;

    return EXIT_SUCCESS;
}

ssize_t udp_sender_send(const u8 *data, size_t sz) {
    struct timeval tmout;
    memset((char *)&tmout,0,sizeof(tmout)); /* Clear time structure. */
    tmout.tv_sec = TIMEOUT_VALUE_UDP;
    if (setsockopt(socket_udp_fd, SOL_SOCKET, SO_SNDTIMEO, (struct timeval *)&tmout,sizeof(struct timeval)) == -1) {
        perror("Setsockopt.\n");
        return -1;
    }
  
    const Result res = try_sendto(socket_udp_fd, data, sz, 0,
                        peer_data->ai_addr, peer_data->ai_addrlen);

    memset((char *)&tmout,0,sizeof(tmout)); /* Clear time structure. */
    tmout.tv_sec = 0;
    if (setsockopt(socket_udp_fd, SOL_SOCKET, SO_SNDTIMEO, (struct timeval *)&tmout,sizeof(struct timeval))) {
        perror("Setsockopt.\n");
        return -1;
    }

    return res;
}

ssize_t udp_sender_recv(u8 *data, size_t sz) {
    struct timeval tmout;
    memset((char *)&tmout,0,sizeof(tmout)); /* Clear time structure. */
    tmout.tv_sec = TIMEOUT_VALUE_UDP;
    if (setsockopt(socket_udp_fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tmout,sizeof(struct timeval)) == -1) {
        perror("Setsockopt.\n");
        return -1;
    }
    
    const Result res = try_recvfrom(socket_udp_fd, data, sz, 0,
                                    peer_data->ai_addr, &peer_data->ai_addrlen);

    memset((char *)&tmout,0,sizeof(tmout)); /* Clear time structure. */
    tmout.tv_sec = 0;
    if (setsockopt(socket_udp_fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tmout,sizeof(struct timeval)) == -1) {
        perror("Setsockopt.\n");
        return -1;
    }
    
    return res;
}

void udp_sender_fini() {
    socket_udp_fd = -1;
    freeaddrinfo(peer_data);
}
