#pragma once
#include "../content.h"
int try_select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds,
               fd_set *restrict exceptfds, struct timeval *restrict timeout);

ssize_t try_write(int fd, const void *buf, size_t count);

ssize_t try_read(int fd, void *buf, size_t sz);
int try_close(int fd);

ssize_t try_sendto(int socket, const void *message, size_t length, int flags,
                   const struct sockaddr *dest_addr, socklen_t dest_len);

ssize_t try_recvfrom(int socket, void *restrict buffer, size_t length,
                     int flags, struct sockaddr *restrict address,
                     socklen_t *restrict address_len);
