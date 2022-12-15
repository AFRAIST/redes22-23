#include "tcp_sender.h"

int socket_tcp_fd = -1;
struct addrinfo *tcp_peer_data;

extern char *GSip;
extern char *GSport;

#define CUR_IP_VAR (GSip)
#define CUR_PORT_VAR (GSport)

ssize_t tcp_sender_try_init() {
    if (BRANCH_LIKELY(socket_tcp_fd != -1))
        return EXIT_SUCCESS;

    socket_tcp_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_tcp_fd == -1)
        return -1;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(CUR_IP_VAR, CUR_PORT_VAR, &hints, &tcp_peer_data) != 0)
        return -1;

    return EXIT_SUCCESS;
}

int tcp_sender_handshake() {
    const int rc = connect(socket_tcp_fd, tcp_peer_data->ai_addr,
                           tcp_peer_data->ai_addrlen);

    return rc;
}

ssize_t tcp_sender_recv(u8 *data, size_t sz) {
    const ssize_t res = try_read(socket_tcp_fd, data, sz);

    return res;
}

ssize_t tcp_sender_recv_all(u8 *buf, size_t sz, bool *finished) {
    size_t bytes = 0;

    do {
        ssize_t rc = try_read(socket_tcp_fd, (void *)((char *)buf + bytes),
                              sz - (size_t)bytes);

        /* EOF... */
        if (rc == 0) {
            *finished = true;
            return bytes;
        }

        if (rc == -1) {
            *finished = false;
            return -1;
        }

        bytes += (size_t)rc;
    } while (bytes != sz);

    *finished = false;
    return bytes;
}

ssize_t tcp_sender_send(const u8 *data, size_t sz) {
    const ssize_t res = try_write(socket_tcp_fd, data, sz);

    return res;
}

ssize_t tcp_sender_fini() {
    freeaddrinfo(tcp_peer_data);
    const ssize_t rc = try_close(socket_tcp_fd);
    socket_tcp_fd = -1;
    return rc;
}
