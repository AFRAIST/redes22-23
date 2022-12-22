#include "rcwg.h"

Result get_plid(char *ap, size_t *plid) {
    if (*ap == '\x00')
        return EXIT_FAILURE;

    int appendix_size = strlen(ap);

    if (appendix_size > 6 || appendix_size <= 0)
        return EXIT_FAILURE;

    if (strtoul_check((ssize_t *)plid, ap) == EXIT_FAILURE)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int handle_fd_close(int fd) {
    if (fd != -1) {
        flock(fd, LOCK_UN);
        int res = close(fd);
        fd = -1;
        return res;
    }
    
    return -1;
}
