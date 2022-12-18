#include "command_processor.h"
#include "proc.h"
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
        if (udp_sender_send((u8 *)"ERR\n", 4) != 4) {                            \
            perror(E_FAILED_REPLY);                                            \
            exit(EXIT_FAILURE); \
        } \
    })

    if (BufNotContainsInvalidNull(recv_buf, sz) == EXIT_FAILURE) {
        /* That's what Tejo said, lol. */
        ERROR_RETURN();
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
    } else {
        outp.err = false;
    }

    tok[6] = back;

    if (COND_COMP_STRINGS_1("SNG", cmd))
        command_start(&outp);
    else if (COND_COMP_STRINGS_1("PLG", cmd)) {
        command_play(&outp);
    } else if (COND_COMP_STRINGS_1("PWG", cmd)) {
        command_guess(&outp);
    } else {
        perror("No command.\n");
    }
    exit(EXIT_SUCCESS); 
}

void command_reader() {
    const pid_t udp_pid = fork();
    if (udp_pid == 0) {
        /* Listen for UDP in parent. */
        udp_sender_try_init();

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
                /* Will exit inside. */
                handle_udp_impl();
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
#undef ERROR_RETURN
}
