
#include "command.h"
#include "udp_sender.h"

static Result get_plid(struct input *inp) {
    char *ap = inp->appendix;
    if (*ap == '\0')
        return EXIT_FAILURE;

    int appendix_size = strlen(inp->appendix);
    int i = 0;

    if (appendix_size > 6 || appendix_size <= 0)
        return EXIT_FAILURE;

    for (i = 0; i < PLID_MAX_SIZE; i++) {
        /* No need to zero pad when we have fmt strings and ints.
        if(appendix_size - PLID_MAX_SIZE + i < 0){
            inp->plid[i] = '0';
            continue;
        }
        */
        if(inp->appendix[i] == '\0')
            break;

        if (inp->appendix[i] < '0' ||
            inp->appendix[i] > '9') {
            return EXIT_FAILURE;
        }
    }
    /*inp->plid[i] = '\0';*/

    inp->plid = strtol(ap, NULL, 10);

    if (inp->plid > 999999)
        return EXIT_FAILURE;

    /* Invalid number. */
    if (errno != 0) {
        errno = 0;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

Result command_start(struct input *inp) {
    if (inp->plid_exists == false) {
        if (get_plid(inp) == EXIT_FAILURE) {
            printf("Nada fixe :c\n");
            return EXIT_FAILURE;
        } else {
            printf("%06zu c:\n", inp->plid);
            inp->plid_exists = true;
        }
    }

    else {
        printf("[ERR] Jogador ja existente\n");
        return EXIT_FAILURE;
    }

    const size_t buf_sz =
        STR_SIZEOF("SNG ") + sizeof('0') * 6 + sizeof('\n') + sizeof('\0');
    char buf[buf_sz];

    size_t sz = (size_t)sprintf(buf, "SNG %zu\n", inp->plid);

    R_FAIL_RETURN(EXIT_FAILURE, udp_sender_try_init() == EXIT_FAILURE,
                  "[ERROR] Could not initialize socket.\n");

    R_FAIL_RETURN(EXIT_FAILURE, udp_sender_send((u8 *)buf, sz) == EXIT_FAILURE,
                  "[ERROR] Could not send player id to server.\n");

    // falta pedir request para ter os max errors
    return EXIT_SUCCESS;
}

Result command_play(struct input *inp) {
    (void)(inp);
    R_NOT_IMPLEMENTED();
    return EXIT_SUCCESS;
}

Result command_guess(struct input *inp) {
    (void)(inp);
    R_NOT_IMPLEMENTED();
    return EXIT_SUCCESS;
}

Result command_scoreboard(struct input *inp) {
    (void)(inp);
    R_NOT_IMPLEMENTED();
    return EXIT_SUCCESS;
}

Result command_hint(struct input *inp) {
    (void)(inp);
    R_NOT_IMPLEMENTED();
    return EXIT_SUCCESS;
}

Result command_state(struct input *inp) {
    (void)(inp);
    R_NOT_IMPLEMENTED();
    return EXIT_SUCCESS;
}

Result command_exit(struct input *inp) {
    (void)(inp);
    R_NOT_IMPLEMENTED();
    return EXIT_SUCCESS;
}
