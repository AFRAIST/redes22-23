#include "tcp_sender.h"

int socket_tcp_fd = -1;
int socket_tcp_new_fd = -1;
struct addrinfo *tcp_peer_data = NULL;

extern char *GSip;
extern char *GSport;

#define CUR_IP_VAR (GSip)
#define CUR_PORT_VAR (GSport)

static struct addrinfo *own_data;
static socklen_t addrlen;
static struct sockaddr_in addr;

static int tcp_sender_has_data() {
    if (socket_tcp_new_fd != -1) {
        Result rc;

        /* We can yield a little. */
        struct timeval tv;
        memset(&tv, 0, sizeof(tv));
        tv.tv_usec = 1;
        rc = setsockopt(socket_tcp_new_fd, SOL_SOCKET, SO_RCVTIMEO, (const void*)&tv, sizeof tv);
        
        if (rc == -1) {
            perror("Setsockopt.\n");
            return -1;
        }

        fd_set set;
        FD_ZERO(&set);
        FD_SET(socket_tcp_new_fd, &set);

        u8 dummy;
        rc = recv(socket_tcp_new_fd, &dummy, sizeof(dummy), MSG_PEEK);
        if (rc == -1) {
            if (errno == EWOULDBLOCK) {
                rc = 0;
                errno = 0;
            }
        }

        memset(&tv, 0, sizeof(tv));
        if (setsockopt(socket_tcp_new_fd, SOL_SOCKET, SO_RCVTIMEO, (const void*)&tv, sizeof tv) == -1) {
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
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, GSport, &hints, &own_data) != 0) {
        perror("Getaddrinfo.\n");
        goto error;
    }

    int optval = 1;
    if(setsockopt(socket_tcp_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        perror("setsockopt");
        return -1;
    }
    
    if (bind(socket_tcp_fd, own_data->ai_addr, own_data->ai_addrlen) == -1) {
        perror("Bind.\n");
        goto error;
    }

    if (listen(socket_tcp_fd, 5) == -1) {
        perror("Listen.\n");
        goto error;
    }

    return EXIT_SUCCESS;

error:
    if (tcp_sender_fini() == -1) {
        perror(E_CLOSE_SOCKET);
    }

    return -1;
}

ssize_t tcp_sender_handshake() {
    addrlen = sizeof(addr);
    socket_tcp_new_fd = accept(socket_tcp_fd, (struct sockaddr *)&addr, &addrlen); 

    char* client_ip = inet_ntoa(addr.sin_addr);
    int client_port = ntohs(addr.sin_port);

    if (socket_tcp_new_fd != -1) {
        VerbosePrintF("Received TCP from %s:%d.\n", client_ip, client_port);
    }

    return socket_tcp_new_fd;
}

ssize_t tcp_sender_recv(u8 *data, size_t sz) {
    const ssize_t res = try_read(socket_tcp_new_fd, data, sz);
    
    return res;
}

ssize_t tcp_sender_recv_all(u8 *buf, size_t sz, bool *finished) {
    size_t bytes = 0;

    VerbosePrintF("Recv TCP data. %d\n", socket_tcp_new_fd);
    do {
        ssize_t rc = try_read(socket_tcp_new_fd, (void *)((char *)buf + bytes),
                              sz - (size_t)bytes);

    
        if (rc > 0) {
            /* We can't check for 0 now because we are the ones replying and that would
            mean a broken pipe. */

            Result data = tcp_sender_has_data();
            if (data == -1) {
                perror("Data error.\n");
                return -1;
            }

            if (!data) {
                /* We are done, I guess... */
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
    VerbosePrintF("Sending TCP data. %d\n", socket_tcp_new_fd);

    const ssize_t res = try_write(socket_tcp_new_fd, data, sz);

    return res;
}

ssize_t tcp_sender_fini() {
    ssize_t rc = EXIT_SUCCESS;

    if (tcp_peer_data != NULL) {
        freeaddrinfo(tcp_peer_data);
        tcp_peer_data = NULL;
    }
    
    if (socket_tcp_new_fd != -1) {
        VerbosePrintF("Closing. %d\n", socket_tcp_new_fd);
        
        shutdown(socket_tcp_new_fd, SHUT_RDWR);
        rc = try_close(socket_tcp_new_fd);

        socket_tcp_new_fd = -1;
    } else {
        rc = -1;
    }

    return rc;
}

ssize_t tcp_sender_fini_global() {
    ssize_t rc = tcp_sender_fini();

    if (tcp_peer_data != NULL) {
        freeaddrinfo(tcp_peer_data);
        tcp_peer_data = NULL;
    }

    if (socket_tcp_fd != -1) {    
        rc = try_close(socket_tcp_fd);
        socket_tcp_fd = -1;
    }

    return rc;
}
