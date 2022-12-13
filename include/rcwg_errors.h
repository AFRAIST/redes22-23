#pragma once

#define E_SIGPIPE_SOCKET ("[ERR] Peer stopped listening (SIGPIPE)\n")
#define E_SENDTO_UDP_SOCKET ("[ERR] Failed sendto socket call\n")
#define E_RECVFROM_UDP_SOCKET ("[ERR] Failed recvfrom socket call\n")
#define E_NOTCONN_UDP_SOCKET                                                   \
    ("[ERR] The UDP socket is in connection mode, not connected though\n")
#define E_CONNRST_UDP_SOCKET                                                   \
    ("[ERR] The UDP socket connection was forcibly closed by a peer\n")
#define E_TIMEOUT_UDP_SOCKET ("[ERR] The UDP socket connection timed out\n")

#define E_CONNRST_TCP_SOCKET                                                   \
    ("[ERR] The TCP socket connection was forcibly closed by a peer\n")
#define E_WRITE_TCP_SOCKET ("[ERR] Failed write socket call\n")
#define E_READ_TCP_SOCKET ("[ERR] Failed read socket call\n")
#define E_TIMEOUT_TCP_SOCKET ("[ERR] The TCP socket connection timed out\n")
#define E_NOTCONN_TCP_SOCKET                                                   \
    ("[ERR] The TCP socket is in connection mode, not connected though\n")

#define E_INVALID_SERVER_REPLY ("[ERROR] Invalid server reply.\n")
#define E_INVALID_NUMBER_REPLY ("[ERROR] Invalid number in server reply.\n")
#define E_INVALID_TRIAL_REPLY ("[ERROR] Invalid trial in server reply.\n")
#define E_INVALID_COMMAND ("[ERROR] Invalid command specification.\n")
#define E_TRIAL_MISMATCH ("[ERROR] The trial value mismatches between peers.\n")

#define E_QUIT_SERVER ("[ERROR] Could not quit the server.\n")
#define E_START_GAME ("[ERROR] Could not start the game.\n")

#define E_FAILED_SOCKET ("[ERROR] Failed to init socket.\n")
#define E_FAILED_REPLY ("[ERROR] Failed to reply.\n")
#define E_FAILED_RECEIVE ("[ERROR] Failed to receive.\n")

#define E_SERVER_ERROR ("[ERROR] Server error.")

#define E_ACQUIRE_ERROR ("[ERROR] Failed to acquire file lock.")
