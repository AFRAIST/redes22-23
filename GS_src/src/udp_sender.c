#include "udp_sender.h"

int socket_fds[PLAYER_AMOUNT];
int socket_udp_fd = -1;
struct addrinfo *serv_data;

extern char *player_ip;
extern char *player_port;

#define CUR_IP_VAR (player_ip)
#define CUR_PORT_VAR (player_port)

void udp_transition_socket(size_t id) {
    const Player *const cur_player = &g_players[id];
    socket_udp_fd = cur_player->sock_fd;
    player_ip = cur_player->player_ip;
    player_port = cur_player->player_port;
}

#include "udp_instance.h"
