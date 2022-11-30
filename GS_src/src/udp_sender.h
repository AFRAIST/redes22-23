#pragma once
#include "player.h"
#include "rcwg.h"

/* Array of sockets. */
extern int socket_fds[PLAYER_AMOUNT];

/* Current socket. */
extern int socket_udp_fd;

Result udp_sender_try_init();
Result udp_sender_send(const u8 *data, size_t sz);
Result udp_sender_recv(u8 *data, size_t sz);
void udp_sender_fini();
void udp_transition_socket(size_t id);
