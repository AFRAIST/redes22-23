#include "udp_sender.h"
#include "rcwg.h"

int socket_udp_fd = -1;
struct addrinfo *serv_data;

extern char *GSip;
extern char *GSport;

Result udp_sender_try_init() {
    /* Lazily init the UDP socket only when needed. */
    if (BRANCH_LIKELY(socket_udp_fd == -1))
        return EXIT_SUCCESS;

    socket_udp_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_udp_fd == -1)
        return EXIT_FAILURE;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket

    if (getaddrinfo(GSip, GSport, &hints, &serv_data) != 0)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

Result udp_sender_send(const u8 *data, size_t sz) {
    const Result res = sendto(socket_udp_fd, data, sz, 0, serv_data->ai_addr,
                              serv_data->ai_addrlen);

    return res;
}

Result udp_sender_recv(const u8 *data, size_t sz) {
    const Result res = sendto(socket_udp_fd, data, sz, 0, serv_data->ai_addr,
                              serv_data->ai_addrlen);

    return res;
}

void udp_sender_fini() {
    socket_udp_fd = -1;
    freeaddrinfo(serv_data);
}
