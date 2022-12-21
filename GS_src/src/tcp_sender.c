#include "tcp_sender.h"

int socket_tcp_fd = -1;
struct addrinfo *tcp_peer_data = NULL;

extern char *GSip;
extern char *GSport;

#define CUR_IP_VAR (GSip)
#define CUR_PORT_VAR (GSport)

static struct addrinfo *own_data;
static socklen_t addrlen;
static struct sockaddr_in addr;

ssize_t tcp_sender_try_init() {
    if (BRANCH_LIKELY(socket_tcp_fd != -1))
        return EXIT_SUCCESS;

    socket_tcp_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_tcp_fd == -1)
        goto error;

    int dummy = 1;
    if (setsockopt(socket_tcp_fd, SOL_SOCKET, SO_REUSEADDR, &dummy, sizeof(int)) == -1) {
        perror("[ERR] Setsockopt.\n");
        return -1;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, GSport, &hints, &own_data) != 0)
        goto error;

    if (bind(socket_tcp_fd, own_data->ai_addr, own_data->ai_addrlen) == -1)
        goto error;

    if (listen(socket_tcp_fd, 5) == -1)
        goto error;

    return EXIT_SUCCESS;

error:
    if (tcp_sender_fini() == -1) {
        perror(E_CLOSE_SOCKET);
    }

    return -1;
}

ssize_t tcp_sender_handshake() {
    addrlen = sizeof(addr);
    socket_tcp_fd = accept(socket_tcp_fd, (struct sockaddr *)&addr, &addrlen); 

    char* client_ip = inet_ntoa(addr.sin_addr);
    int client_port = ntohs(addr.sin_port);

    if (socket_tcp_fd != -1) {
        VerbosePrintF("Received TCP from %s:%d.\n", client_ip, client_port);
    }

    return socket_tcp_fd;
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

        if (rc > 0) {
            /* We can't check for 0 now because we are the ones replying and that would
            mean a broken pipe. */
            if(*((char *)buf + bytes + rc - 1) == '\n') {
                *finished = true;                
                return bytes + rc;
            }
        }

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
    if (tcp_peer_data != NULL) {
        freeaddrinfo(tcp_peer_data);
        tcp_peer_data = NULL;
    }
    const ssize_t rc = try_close(socket_tcp_fd);

    socket_tcp_fd = -1;
    return rc;
}
