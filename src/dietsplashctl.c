#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>

#define CMDS_SOCKET_NAME "/dietsplash"
#define MAX_CMD_LEN 63

/**
 * Setup our private socket to communicate boot progress
 *
 * @return file descriptor of new socket created
 */
static inline int __socket_setup(struct sockaddr_un *addr, size_t *addrsize)
{
    int fd;
    size_t len;

    len = strlen(CMDS_SOCKET_NAME);
    assert(len && len < sizeof(addr->sun_path));

    fd = socket(PF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd == -1) {
        perror("creating socket - %m");
        goto out;
    }

    addr->sun_family = AF_UNIX;

    // abstract socket, meaning the path is not created
    addr->sun_path[0] = '\0';
    memcpy(addr->sun_path + 1, CMDS_SOCKET_NAME, len);

    // size is +1 because of the initial NUL char
    *addrsize = len + offsetof(struct sockaddr_un, sun_path) + 1;

out:
    return fd;
}

static int setup_socket(void)
{
    struct sockaddr_un addr;
    size_t addrsize;
    int sfd;

    sfd = __socket_setup(&addr, &addrsize);
    if (sfd == -1)
        goto out;

    if (connect(sfd, (struct sockaddr *) &addr, addrsize) == -1)
        goto close_and_exit;

    return sfd;

close_and_exit:
    close(sfd);
    sfd = -1;

out:
    return sfd;
}

static void usage(void) {
    fprintf(stderr, "USAGE: dietsplashctl percentage [message]\n\n"
                    "EXAMPLE\n\t"
                        "dietsplashctl 49 \"loading ssh\"\n\n");
}

int main(int argc, char *argv[])
{
    int sfd;
    long perc;
    char buf[MAX_CMD_LEN + 1];
    size_t len;
    const char *msg;

    if (argc == 3) {
        msg = argv[2];
        len = strlen(msg);
    } else if (argc == 2) {
        msg = NULL;
        len = 0;
    } else {
        usage();

        return 1;
    }

    errno = 0;
    perc = strtol(argv[1], NULL, 10);
    if (errno == EINVAL || errno == ERANGE || perc < 0 || perc > 100) {
        fprintf(stderr, "Invalid value. Percentage is a value"
                        "between 0 and 100\n");

        return 1;
    }

    buf[0] = (char)(unsigned char) perc;

    if (len > MAX_CMD_LEN - 1) {
        fprintf(stderr, "Max length is %u. Message will be truncated",
                                                            MAX_CMD_LEN - 1);
        len = MAX_CMD_LEN - 1;
    }

    if (msg)
        memcpy(&buf[1], msg, len);

    buf[len + 1] = '\0';

    sfd = setup_socket();
    if (sfd == -1) {
        fprintf(stderr, "Could not connect to dietsplash on private socket\n");

        return 1;
    }

    write(sfd, buf, len + 2);
    close(sfd);

    return 0;
}
