#include "udp_sender.h"

int socket_fds[PLAYER_AMOUNT];
int socket_udp_fd = -1;
struct addrinfo *peer_data;

extern char *player_ip;
extern char *player_port;
extern char *GSport;

#define CUR_IP_VAR (player_ip)
#define CUR_PORT_VAR (player_port)

struct addrinfo *own_data;
socklen_t addrlen;
struct sockaddr_in addr;

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
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, GSport, &hints, &own_data) != 0)
        return -1;

    ssize_t n = bind(socket_udp_fd, own_data->ai_addr, own_data->ai_addrlen);

    if (n == -1)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

ssize_t udp_sender_recv(u8 *data, size_t sz) {
    addrlen = sizeof(addr);
    const ssize_t res = try_recvfrom(socket_udp_fd, data, sz, 0,
                                     (struct sockaddr *)&addr, &addrlen);

    return res;
}

ssize_t udp_sender_send(const u8 *data, size_t sz) {
    const ssize_t res = try_sendto(socket_udp_fd, data, sz, 0,
                                   (struct sockaddr *)&addr, addrlen);

    return res;
}

void udp_sender_fini() {
    if (socket_udp_fd != -1) {
        socket_udp_fd = -1;
        freeaddrinfo(own_data);
    }
}
