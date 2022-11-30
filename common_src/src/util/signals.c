#include "rcwg.h"

static ALWAYS_INLINE void ignore_signal_impl(int signum) {
    struct sigaction act;

    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    act.sa_flags = SA_RESTART;

    sigaction(signum, &act, NULL);
}

void ignore_signal_sigpipe() { ignore_signal_impl(SIGPIPE); }

void ignore_signal_sigchld() { ignore_signal_impl(SIGCHLD); }

void ignore_signals() {
    ignore_signal_sigpipe();
    ignore_signal_sigchld();
}
