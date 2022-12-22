#pragma once
#include "rcwg.h"

extern int socket_udp_fd;

ssize_t udp_sender_try_init();
ssize_t udp_sender_send(const u8 *data, size_t sz);
ssize_t udp_sender_recv(u8 *data, size_t sz);
void udp_sender_fini();
