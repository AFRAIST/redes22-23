#include "rcwg.h"

int try_select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds,
               fd_set *restrict exceptfds, struct timeval *restrict timeout) {
    int rc = 0;

    do {
        errno = 0;
        rc = select(nfds, readfds, writefds, exceptfds, timeout);
    } while (rc == -1 && errno == EINTR);

    return rc;
}

ssize_t try_write(int fd, const void *buf, size_t count) {
    ssize_t rc = 0;

    do {
        errno = 0;
        rc = write(fd, buf, count);
    } while (rc == -1 && errno == EINTR);

    if (errno == 0)
        return rc;

    if (errno == EPIPE)
        perror(E_SIGPIPE_SOCKET);

    else if (errno == ECONNRESET)
        perror(E_CONNRST_TCP_SOCKET);

    else
        perror(E_WRITE_TCP_SOCKET);

    return -1;
}

ssize_t try_read(int fd, void *buf, size_t sz) {
    ssize_t rc = 0;

    do {
        errno = 0;
        rc = read(fd, buf, sz);
    } while (rc == -1 && errno == EINTR);

    if (errno == 0)
        return rc;

    if (errno == ETIMEDOUT)
        perror(E_TIMEOUT_TCP_SOCKET);

    else if (errno == ECONNRESET)
        perror(E_CONNRST_TCP_SOCKET);

    else if (errno == ENOTCONN)
        perror(E_NOTCONN_TCP_SOCKET);

    else
        perror(E_READ_TCP_SOCKET);

    return -1;
}

int try_close(int fd) {
    int rc = 0;

    do {
        errno = 0;
        rc = close(fd);
    } while (rc == -1 && errno == EINTR);

    if (errno == 0)
        return 0;

    return -1;
}

ssize_t try_recvfrom(int socket, void *restrict buffer, size_t length,
                     int flags, struct sockaddr *restrict address,
                     socklen_t *restrict address_len) {
    Result rc = 0;

    do {
        errno = 0;
        rc = recvfrom(socket, buffer, length, flags, address, address_len);
    } while (rc == -1 && errno == EINTR);

    if (errno == 0)
        return rc;

    if (errno == ETIMEDOUT)
        perror(E_TIMEOUT_UDP_SOCKET);

    else if (errno == ECONNRESET)
        perror(E_CONNRST_UDP_SOCKET);

    else if (errno == ENOTCONN)
        perror(E_NOTCONN_UDP_SOCKET);

    else
        perror(E_RECVFROM_UDP_SOCKET);

    return -1;
}

ssize_t try_sendto(int socket, const void *message, size_t length, int flags,
                   const struct sockaddr *dest_addr, socklen_t dest_len) {
    Result rc = 0;

    do {
        errno = 0;
        rc = sendto(socket, message, length, flags, dest_addr, dest_len);
    } while (rc == -1 && errno == EINTR);

    if (errno == 0)
        return rc;

    if (errno == EPIPE)
        perror(E_SIGPIPE_SOCKET);

    else if (errno == ECONNRESET)
        perror(E_CONNRST_UDP_SOCKET);

    else if (errno == ENOTCONN)
        perror(E_NOTCONN_UDP_SOCKET);

    else
        perror(E_SENDTO_UDP_SOCKET);

    return -1;
}
