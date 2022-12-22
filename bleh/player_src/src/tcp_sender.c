#include "tcp_sender.h"

int socket_tcp_fd = -1;
struct addrinfo *tcp_peer_data = NULL;

extern char *GSip;
extern char *GSport;

#define CUR_IP_VAR (GSip)
#define CUR_PORT_VAR (GSport)

static int tcp_sender_has_data() {
    if (socket_tcp_fd != -1) {
        Result rc;

        /* We can yield a little. */
        struct timeval tv;
        memset(&tv, 0, sizeof(tv));
        tv.tv_usec = 1;
        rc = setsockopt(socket_tcp_fd, SOL_SOCKET, SO_RCVTIMEO, (const void*)&tv, sizeof tv);
        
        if (rc == -1) {
            perror("Setsockopt.\n");
            return -1;
        }

        fd_set set;
        FD_ZERO(&set);
        FD_SET(socket_tcp_fd, &set);

        u8 dummy;
        rc = recv(socket_tcp_fd, &dummy, sizeof(dummy), MSG_PEEK);
        if (rc == -1) {
            if (errno == EWOULDBLOCK) {
                rc = 0;
                errno = 0;
            }
        }

        memset(&tv, 0, sizeof(tv));
        if (setsockopt(socket_tcp_fd, SOL_SOCKET, SO_RCVTIMEO, (const void*)&tv, sizeof tv) == -1) {
            perror("Setsockopt.\n");
            return -1;
        }

        return (int)rc;
    }

    return -1;
}

int tcp_sender_delay() {
    if (socket_tcp_fd != -1) {
        Result rc;

        /* We wait 2 seconds. */
        struct timeval tv;
        memset(&tv, 0, sizeof(tv));
        tv.tv_sec = TIMEOUT_VALUE_TCP;
        rc = setsockopt(socket_tcp_fd, SOL_SOCKET, SO_RCVTIMEO, (const void*)&tv, sizeof tv);
        
        if (rc == -1) {
            perror("Setsockopt.\n");
            return -1;
        }

        fd_set set;
        FD_ZERO(&set);
        FD_SET(socket_tcp_fd, &set);

        printf("Connection is bad!\n");
        u8 dummy;
        rc = recv(socket_tcp_fd, &dummy, sizeof(dummy), MSG_PEEK);
        if (rc == -1) {
            if (errno == EWOULDBLOCK) {
                rc = 0;
                errno = 0;
            }
        }

        memset(&tv, 0, sizeof(tv));
        if (setsockopt(socket_tcp_fd, SOL_SOCKET, SO_RCVTIMEO, (const void*)&tv, sizeof tv) == -1) {
            perror("Setsockopt.\n");
            return -1;
        }

        return (int)rc;
    }

    return -1;
}

ssize_t tcp_sender_try_init() {
    if (BRANCH_LIKELY(socket_tcp_fd != -1))
        return EXIT_SUCCESS;

    socket_tcp_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_tcp_fd == -1)
        goto error;
 
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(CUR_IP_VAR, CUR_PORT_VAR, &hints, &tcp_peer_data) != 0)
        goto error;

    return EXIT_SUCCESS;

error:
    if (tcp_sender_fini() == -1) {
        perror(E_CLOSE_SOCKET);
    }

    return -1;
}

int tcp_sender_handshake() {
    printf("Doing HS.\n");
    const int rc = connect(socket_tcp_fd, tcp_peer_data->ai_addr,
                           tcp_peer_data->ai_addrlen);
    printf("HS done. %d\n", rc);

    return rc;
}

ssize_t tcp_sender_recv(u8 *data, size_t sz) {
    const ssize_t res = try_read(socket_tcp_fd, data, sz);

    return res;
}

ssize_t tcp_sender_recv_all(u8 *buf, size_t sz, bool *finished) {
    size_t bytes = 0;

    printf("Receiving data.\n");
    do {
        ssize_t rc = try_read(socket_tcp_fd, (void *)((char *)buf + bytes),
                              sz - (size_t)bytes);

        printf("Recvd %zu %zu.\n", rc, sz);

        if (rc > 0) {
            Result data = tcp_sender_has_data();
            if (data == -1) {
                perror("Data error.\n");
                return -1;
            }

            if (!data) {
                *finished = true;
                return (bytes+rc);
            }
        }

        if (rc == -1) {
            *finished = false;
            printf("Failed.\n");
            return -1;
        }

        bytes += (size_t)rc;
    } while (bytes != sz);

    *finished = false;
    printf("Done.\n");
    return bytes;
}

ssize_t tcp_sender_send(const u8 *data, size_t sz) {
    const ssize_t res = try_write(socket_tcp_fd, data, sz);

    return res;
}

ssize_t tcp_sender_fini() {
    if (tcp_peer_data != NULL) {
        freeaddrinfo(tcp_peer_data);
        tcp_peer_data = NULL;
    }
    const ssize_t rc = try_close(socket_tcp_fd);

    socket_tcp_fd = -1;
    return rc;
}
