#include "command_processor.h"
#include "proc.h"
#include "tcp_sender.h"
#include "udp_sender.h"

static __attribute__((noreturn)) void handle_udp_impl() {
    const size_t recv_buf_sz = COMMAND_BUF_SZ;
    char recv_buf[COMMAND_BUF_SZ] = "";
    struct output outp;

    ssize_t sz;
    if ((sz = udp_sender_recv((u8 *)recv_buf, recv_buf_sz)) == EXIT_FAILURE) {
        perror(E_FAILED_RECEIVE);
        exit(EXIT_FAILURE);
    }

#define ERROR_RETURN()                                                         \
    ({                                                                         \
        if (udp_sender_send((u8 *)"ERR\n", 4) != 4) {                          \
            perror(E_FAILED_REPLY);                                            \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
        exit(EXIT_FAILURE);                                                    \
    })

    if (recv_buf[sz - 1] != '\n') {
        ERROR_RETURN();
    }

    outp.err = false;

    if (BufNotContainsInvalidNull(recv_buf, sz) == EXIT_FAILURE) {
        outp.err = true;
    }

    if (BufNotContainsMoreThanOneLF(recv_buf, sz) == EXIT_FAILURE) {
        outp.err = true;
    }

    char *cmd;
    char *tok;

    cmd = BufTokenizeOpt(recv_buf, " ", &outp.next);

    tok = BufTokenizeOpt(outp.next, " ", &outp.next);

    /* Dirty backup, but we have 128 of free space. */
    char back = tok[6];
    tok[6] = 0;
    if (strtoul_check((ssize_t *)&outp.plid, tok) == EXIT_FAILURE) {
        outp.err = true;
    }

    tok[6] = back;

    VerbosePrintF("Command: %s.\n", cmd);
    VerbosePrintF("PLID: %zu.\n", outp.plid);

    if (COND_COMP_STRINGS_1("SNG", cmd))
        command_start(&outp);
    else if (COND_COMP_STRINGS_1("PLG", cmd)) {
        command_play(&outp);
    } else if (COND_COMP_STRINGS_1("PWG", cmd)) {
        command_guess(&outp);
    } else if (COND_COMP_STRINGS_1("QUT", cmd)) {
        command_quit(&outp);
    } else {
        perror("No command.\n");
        ERROR_RETURN();
    }
    exit(EXIT_SUCCESS);
#undef ERROR_RETURN
}

static __attribute__((noreturn)) void handle_tcp_impl() {
    const size_t recv_buf_sz = COMMAND_BUF_SZ;
    char recv_buf[COMMAND_BUF_SZ] = "";
    struct output outp;

#define ERROR_RETURN()                                                         \
    ({                                                                         \
        VerbosePrintF("Data received: %s\n", recv_buf);                        \
        if (tcp_sender_send((u8 *)"ERR\n", 4) != 4) {                          \
            perror(E_FAILED_REPLY);                                            \
            if (tcp_sender_fini() == -1)                                       \
                perror("[ERR] Closing TCP.\n");                                \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
        if (tcp_sender_fini() == -1)                                           \
            perror("[ERR] Closing TCP.\n");                                    \
        exit(EXIT_FAILURE);                                                    \
    })

    u32 sz;
    bool fin;
    if ((s32)(sz = tcp_sender_recv_all((u8 *)recv_buf, recv_buf_sz - 1,
                                       &fin)) == -1)
        ERROR_RETURN();
    VerbosePrintF("Received %u bytes from TCP.\n", sz);

    if (!fin || recv_buf[sz-1] != '\n')
        ERROR_RETURN();

    recv_buf[sz] = '\x00';

    if (BufNotContainsInvalidNull(recv_buf, sz) == EXIT_FAILURE)
        ERROR_RETURN();

    if (BufNotContainsMoreThanOneLF(recv_buf, sz) == EXIT_FAILURE)
        ERROR_RETURN();

    if (!strcmp(recv_buf, "GSB\n")) {
        perror("Not yet implemented!");
        if (tcp_sender_fini())
            perror("[ERR] Closing TCP.\n");
        exit(EXIT_FAILURE);
    }

    char *cmd, *tok;
    cmd = BufTokenizeOpt(recv_buf, " ", &outp.next);
    tok = BufTokenizeOpt(outp.next, " ", &outp.next);

    /* Dirty backup, but we have 128 of free space. */
    char back = tok[6];
    tok[6] = 0;
    if (strtoul_check((ssize_t *)&outp.plid, tok) == EXIT_FAILURE) {
        outp.err = true;
    } else {
        outp.err = false;
    }

    tok[6] = back;

    VerbosePrintF("Command: %s.\n", cmd);
    VerbosePrintF("PLID: %zu.\n", outp.plid);

    if (COND_COMP_STRINGS_1("GHL", cmd)) {
        command_hint(&outp);
    } else if (COND_COMP_STRINGS_1("STA", cmd)) {
        command_state(&outp);
    } else {
        perror("No command.\n");
        ERROR_RETURN();
    }

    if (tcp_sender_fini())
        perror("[ERR] Closing TCP.\n");
    exit(EXIT_SUCCESS);
#undef ERROR_RETURN
}

void command_reader() {
    const pid_t udp_pid = fork();
    if (udp_pid == 0) {
        /* Listen for UDP in parent. */
        R_FAIL_EXIT_IF(udp_sender_try_init() == -1,
                       "[ERROR] Broken UDP sockets.\n");

        fd_set set;
        while (true) {
            FD_ZERO(&set);
            FD_SET(socket_udp_fd, &set);

            int rc = try_select(socket_udp_fd + 1, &set, NULL, NULL, NULL);

            if (rc == -1)
                continue;

            /* Spin the seeds. */
            rand();

            const pid_t h_udp = fork();
            if (h_udp == 0) {
                // VerbosePrintF("UDP connection inited.\n");

                /* Will exit inside. */
                handle_udp_impl();
            }
            /* Return the seeds. */
        }

        exit(EXIT_SUCCESS);
    } else {
        VerbosePrintF("TCP ready.");
        const pid_t tcp_pid = fork();
        VerbosePrintF("TCP %u.\n", tcp_pid);

        if (tcp_pid == 0) {
            fd_set set;
            /* Listen for TCP in parent. */
            VerbosePrintF("TCP in.\n");
            while (tcp_sender_try_init() == -1) {
                perror("Socket init.\n");
            }
            //while(1) {perror("Worked.\n");}
            VerbosePrintF("Inited.\n");
            while (true) {
                
                int rc;
                FD_ZERO(&set);
                FD_SET(socket_tcp_fd, &set);

                VerbosePrintF("Going to SELECT %d.\n", socket_tcp_fd);
                rc = try_select(socket_tcp_fd + 1, &set, NULL, NULL, NULL);
                VerbosePrintF("TCP Select done...\n");
               
                if (rc == -1) {
                    continue;
                }

                if (tcp_sender_handshake() == -1) {
                    perror(E_HANDSHAKE_FAILED);
                    continue;
                }

                /*
                VerbosePrintF("TCP Handshake done...\n");
                FD_ZERO(&set);
                FD_SET(socket_tcp_fd, &set);
                rc = try_select(socket_tcp_fd + 1, &set, NULL, NULL, NULL);
                VerbosePrintF("TCP Post Handshake done...\n");

                if (rc == -1) {
                    tcp_sender_fini();
                    continue;
                }
                */

                const pid_t h_tcp = fork();
                if (h_tcp == 0) {
                    VerbosePrintF("TCP connection inited.\n");
                    /* Will exit inside. */
                    handle_tcp_impl();
                }
            }

            if (tcp_sender_fini_global() == -1) {
                perror("Failed to close global socket.\n");
            }

            exit(EXIT_SUCCESS);
        }

        /* Hunt the zombies! */
        proc_start_zombie_hunter();
    }

    udp_sender_fini();
#undef ERROR_RETURN
}
