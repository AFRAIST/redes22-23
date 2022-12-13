#include "proc.h"

void proc_start_zombie_hunter() {
    pid_t rc;
    int status;
    errno = 0;

    do {
        rc = waitpid(-1, &status, 0);
    } while (rc != -1);
}
