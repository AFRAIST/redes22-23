#pragma once
#include "rcwg.h"

extern int socket_udp_fd;

Result udp_sender_try_init();
Result udp_sender_send(const u8 *data, size_t sz);
Result udp_sender_recv(const u8 *data, size_t sz);
void udp_sender_fini();
