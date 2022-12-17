#include "command_processor.h"
#include "proc.h"
#include "serial.h"
#include "udp_sender.h"

static void handle_udp_impl() {
    const size_t recv_buf_sz = COMMAND_BUF_SZ;
    char recv_buf[COMMAND_BUF_SZ] = "";
    char *command;
    char *plid_s;
    struct output outp;

    ssize_t sz;
    if ((sz = udp_sender_recv((u8 *)recv_buf, recv_buf_sz)) == EXIT_FAILURE) {
        perror(E_FAILED_RECEIVE);
        return;
    }

#define ERROR_RETURN()                                                         \
    ({                                                                         \
        udp_sender_send((u8 *)"ERR\n", 4);                                     \
        return;                                                                \
    })

    if (BufNotContainsInvalidNull(recv_buf, sz) == EXIT_FAILURE) {
        ERROR_RETURN();
    }

    /* Isto está errado. */
    outp.buff = recv_buf;
    command = outp.buff;
    outp.buff = StrNSplitSpaceNext(outp.buff, 3);
    plid_s = outp.buff;
    outp.buff = StrNSplitSpaceNext(outp.buff, 6);

    if (strtoul_check((ssize_t *)&outp.plid, plid_s) == EXIT_FAILURE) {
        ERROR_RETURN();
    }

    acquire_player_file(outp.plid);

    if (COND_COMP_STRINGS_1("SNG", command))
        command_start(&outp);
    else if (COND_COMP_STRINGS_1("PLG", command)) {
        command_play(&outp);
    } else if (COND_COMP_STRINGS_1("PWG", command)) {
        command_guess(&outp);
    } else {
        perror("No command.\n");
    }

    release_player_file();
#undef ERROR_RETURN
}

void command_reader() {
    const pid_t udp_pid = fork();
    if (udp_pid == 0) {
        /* Listen for UDP in parent. */
        udp_sender_try_init();

        fd_set set;
        while (true) {
            FD_ZERO(&set);\
            FD_SET(socket_udp_fd, &set);

            int rc = try_select(socket_udp_fd + 1, &set, NULL, NULL, NULL);

            if (rc == -1)
                continue;

            /* Spin the seeds. */
            rand();
            
            const pid_t h_udp = fork();
            if (h_udp == 0) {
                handle_udp_impl();
                exit(EXIT_SUCCESS);
            }
            /* Return the seeds. */
        }

        exit(EXIT_SUCCESS);
    } else {
        const pid_t tcp_pid = fork();

        if (tcp_pid == 0) {
            puts("TCP not yet implemented!");
            exit(EXIT_SUCCESS);
        }

        /* Hunt the zombies! */
        proc_start_zombie_hunter();
    }

    udp_sender_fini();
}
