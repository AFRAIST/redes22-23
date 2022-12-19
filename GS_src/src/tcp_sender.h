#pragma once
#include "rcwg.h"

extern int socket_tcp_fd;

ssize_t tcp_sender_try_init();
ssize_t tcp_sender_handshake();
ssize_t tcp_sender_recv(u8 *data, size_t sz);
ssize_t tcp_sender_recv_all(u8 *buf, size_t sz, bool *finished);
ssize_t tcp_sender_send(const u8 *data, size_t sz);
ssize_t tcp_sender_fini();
