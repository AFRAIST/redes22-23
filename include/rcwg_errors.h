#pragma once

#define E_SIGPIPE_SOCKET ("[ERR] Peer stopped listening (SIGPIPE)")
#define E_SENDTO_UDP_SOCKET ("[ERR] Failed sendto socket call")
#define E_RECVFROM_UDP_SOCKET ("[ERR] Failed recvfrom socket call")
#define E_NOTCONN_UDP_SOCKET                                                   \
    ("[ERR] The UDP socket is in connection mode, not connected though")
#define E_CONNRST_UDP_SOCKET                                                   \
    ("[ERR] The UDP socket connection was forcibly closed by a peer")
#define E_TIMEOUT_UDP_SOCKET ("[ERR] The UDP socket connection timed out")

#define E_CONNRST_TCP_SOCKET                                                   \
    ("[ERR] The TCP socket connection was forcibly closed by a peer")
#define E_WRITE_TCP_SOCKET ("[ERR] Failed write socket call")
#define E_READ_TCP_SOCKET ("[ERR] Failed read socket call")
#define E_TIMEOUT_TCP_SOCKET ("[ERR] The TCP socket connection timed out")
#define E_NOTCONN_TCP_SOCKET                                                   \
    ("[ERR] The TCP socket is in connection mode, not connected though")

#define E_INVALID_SERVER_REPLY ("[ERROR] Invalid server reply.")
#define E_INVALID_NUMBER_REPLY ("[ERROR] Invalid number in server reply.")
