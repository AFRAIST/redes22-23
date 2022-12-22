#include "rcwg.h"

static ALWAYS_INLINE void ignore_signal_sigpipe() {
    struct sigaction act;

    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    act.sa_flags = SA_RESTART;

    sigaction(SIGPIPE, &act, NULL);
}

static ALWAYS_INLINE void ignore_signal_sigchld() {
    struct sigaction act;

    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_DFL;
    act.sa_flags = SA_RESTART;

    sigaction(SIGCHLD, &act, NULL);
}


void ignore_signals() {
    ignore_signal_sigpipe();
    ignore_signal_sigchld();
}
