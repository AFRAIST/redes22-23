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
    
    struct timeval tmout;
    memset((char *)&tmout,0,sizeof(tmout)); /* Clear time structure. */
    tmout.tv_sec = 2;
    if (setsockopt(socket_tcp_new_fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tmout,sizeof(struct timeval)) == -1) {
        perror("Setsockopt.\n");
        return -1;
    }
    
    const ssize_t res = try_read(socket_tcp_new_fd, data, sz);

    memset((char *)&tmout,0,sizeof(tmout)); /* Clear time structure. */
    tmout.tv_sec = 0;
    if (setsockopt(socket_tcp_new_fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tmout,sizeof(struct timeval)) == -1) {
        perror("Setsockopt.\n");
        return -1;
    }
    
    return res;
}

ssize_t tcp_sender_recv_all(u8 *buf, size_t sz, bool *finished) {
    size_t bytes = 0;

    VerbosePrintF("Recv TCP data. %d\n", socket_tcp_new_fd);
    do {
        struct timeval tmout;
        memset((char *)&tmout,0,sizeof(tmout)); /* Clear time structure. */
        tmout.tv_sec = 2;
        if (setsockopt(socket_tcp_new_fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tmout,sizeof(struct timeval)) == -1) {
            perror("Setsockopt.\n");
            return -1;
        }
        
        ssize_t rc = try_read(socket_tcp_new_fd, (void *)((char *)buf + bytes),
                              sz - (size_t)bytes);

        memset((char *)&tmout,0,sizeof(tmout)); /* Clear time structure. */
        tmout.tv_sec = 0;
        if (setsockopt(socket_tcp_new_fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tmout,sizeof(struct timeval)) == -1) {
            perror("Setsockopt.\n");
            return -1;
        }
    
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
    VerbosePrintF("Sending TCP data. %d\n", socket_tcp_new_fd);

    struct timeval tmout;
    memset((char *)&tmout,0,sizeof(tmout)); /* Clear time structure. */
    tmout.tv_sec = 2;
    if (setsockopt(socket_tcp_new_fd, SOL_SOCKET, SO_SNDTIMEO, (struct timeval *)&tmout,sizeof(struct timeval)) == -1) {
        perror("Setsockopt.\n");
        return -1;
    }
    
    const ssize_t res = try_write(socket_tcp_new_fd, data, sz);

    memset((char *)&tmout,0,sizeof(tmout)); /* Clear time structure. */
    tmout.tv_sec = 0;
    if (setsockopt(socket_tcp_new_fd, SOL_SOCKET, SO_SNDTIMEO, (struct timeval *)&tmout,sizeof(struct timeval))) {
        perror("Setsockopt.\n");
        return -1;
    }
    
    return res;
}

ssize_t tcp_sender_fini() {
    VerbosePrintF("Closing. %d\n", socket_tcp_new_fd);
    if (tcp_peer_data != NULL) {
        freeaddrinfo(tcp_peer_data);
        tcp_peer_data = NULL;
    }
    
    shutdown(socket_tcp_new_fd, SHUT_RDWR);
    const ssize_t rc = try_close(socket_tcp_new_fd);

    socket_tcp_new_fd = -1;
    return rc;
}

ssize_t tcp_sender_fini_global() {
    if (tcp_peer_data != NULL) {
        freeaddrinfo(tcp_peer_data);
        tcp_peer_data = NULL;
    }
    const ssize_t rc = try_close(socket_tcp_fd);

    socket_tcp_fd = -1;
    return rc;
}
