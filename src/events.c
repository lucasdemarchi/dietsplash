#include "log.h"
#include "events.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/un.h>

struct cb {
    int fd;
    void (*func)(int fd);
};

static bool _mainloop_quit;
static enum mainloop_status _mainloop_status;

static int epollfd = -1;

/* callbacks */
static void on_quit(int fd);
static void on_connection_request(int fd);
static void on_command(int fd);

static struct cb _timers[TIMERS_NR] = {
    {
       .fd = -1,
       .func = on_quit
    }
};

static struct cmds {
    struct cb conn;
    struct cb read;
    struct {
        unsigned char perc;
        char msg[MAX_CMD_LEN];
    } boot_status;
} _cmds = {
    .conn = { .fd = -1, .func = on_connection_request },
    .read = { .fd = -1, .func = on_command },
};

#define MAX_EPOLL_EVENTS 5
#define MAX_CMDS_EVENTS 5
#define CMDS_SOCKET_NAME "/dietsplash"

int ds_events_shutdown(void)
{
    int i, r = 0;

    for (i = 0; i < TIMERS_NR; i++) {
        inf("closing timer %d", _timers[i].fd);
        if (_timers[i].fd != -1 && (r |= close(_timers[i].fd)) == -1)
            err("shutdown timer %d - %m", i);
    }

    if (_cmds.conn.fd != -1 && (r |= close(_cmds.conn.fd)) == -1)
        err("close cmds connection sock - %m");

    if (_cmds.read.fd != -1 && (r |= close(_cmds.read.fd)) == -1)
        err("close cmds sock - %m");

    if((r |= close(epollfd)) == -1)
        err("close epoll - %m");

    return r;
}

static int _watch_fd(int fd, struct cb *cb)
{
    struct epoll_event ev;

    inf("_watch_fd %d", fd);

    ev.events = EPOLLIN;
    ev.data.ptr = cb;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        err("epoll_ctl: fd - %m");
        close(fd);
        cb->fd = -1;
        return -1;
    }

    return fd;
}

static void on_command(int fd) {
    char buf[MAX_CMD_LEN + 1];
    int n;

    buf[MAX_CMD_LEN] = '\0';

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        _cmds.boot_status.perc = (unsigned char) buf[0];

        if (n < 2 || (unsigned int)(n - 2) != strlen(&buf[1]))
            wrn("Command received is truncated %d %d", n, strlen(&buf[1]));

        inf("Command received: perc: %u%% message: %s",
                                                _cmds.boot_status.perc, &buf[1]);
    }

    /* save the latest boot status */
    memcpy(&_cmds.boot_status.msg, &(buf[1]), MAX_CMD_LEN);

    if (_cmds.boot_status.perc == 100)
        ds_events_stop(MAINLOOP_STATUS_EXIT_SUCCESS);

    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return;

    /* Client closed the connection */
    close(fd);
    _cmds.read.fd = -1;
}

static void on_quit(int fd)
{
    uint64_t buf;

    while (read(fd, &buf, sizeof(buf)) > 0)
        ;

    if (errno != EAGAIN && errno != EWOULDBLOCK)
        err("read quit timer");

    ds_events_stop(MAINLOOP_STATUS_EXIT_FAILURE);
}

static void on_connection_request(int fd)
{
    int s;
    uint64_t buf;

    /*
     * Ignore new connections since there's a client
     * already
     */
    if (_cmds.read.fd != -1) {
        while (read(fd, &buf, sizeof(buf)) > 0)
            ;
    }

    while ((s = accept4(fd, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC)) > 0) {
        inf("connection request received");
        _cmds.read.fd = s;
        _watch_fd(_cmds.read.fd, &_cmds.read);
    }
}

/**
 * Setup our private socket to communicate boot progress
 *
 * @return file descriptor of new socket created
 */
static inline int __events_socket_setup(struct sockaddr_un *addr, size_t *addrsize)
{
    int fd;
    size_t len;

    len = strlen(CMDS_SOCKET_NAME);
    assert(len && len < sizeof(addr->sun_path));

    fd = socket(PF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd == -1) {
        crit("creating socket - %m");
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

static int _events_cmds_listen(void)
{
    struct sockaddr_un addr;
    size_t addrsize;

    assert(_cmds.conn.fd == -1);

    _cmds.conn.fd = __events_socket_setup(&addr, &addrsize);
    if (_cmds.conn.fd == -1)
       goto exit_err;

    if (bind(_cmds.conn.fd, (struct sockaddr *) &addr, addrsize) == -1) {
        crit("binding to cmd socket - %m");
        goto close_and_exit_err;
    }

    if (listen(_cmds.conn.fd, MAX_CMDS_EVENTS) == -1) {
        crit("listening socket - %m");
        goto close_and_exit_err;
    }

    return _watch_fd(_cmds.conn.fd, &_cmds.conn);

close_and_exit_err:
    close(_cmds.conn.fd);
exit_err:
    return -1;
}

static void _process_events(struct epoll_event *ev)
{
    struct cb *cb = ev->data.ptr;

    inf("processing events for fd %d", cb->fd);

    cb->func(cb->fd);
}

int ds_events_timer_add(int idx, time_t tv_sec, long tv_nsec, bool oneshot)
{
    struct itimerspec tm = { { 0 }, { 0 } };

    assert(idx < TIMERS_NR && _timers[idx].fd == -1);

    _timers[idx].fd = timerfd_create(CLOCK_MONOTONIC,
                                     TFD_NONBLOCK | TFD_CLOEXEC);
    if (_timers[idx].fd == -1) {
        err("timerfd_create - %m");
        return -1;
    }

    tm.it_value.tv_sec = tv_sec;
    tm.it_value.tv_nsec = tv_nsec;

    if (!oneshot) {
        tm.it_interval.tv_sec = tv_sec;
        tm.it_interval.tv_nsec = tv_nsec;
    }

    if (timerfd_settime(_timers[idx].fd, 0, &tm, NULL) == -1) {
        err("timerfd_settime - %m");
        close(_timers[idx].fd);
        return -1;
    }

    return _watch_fd(_timers[idx].fd, &_timers[idx]);
}

/**
 * Get the status of the mainloop
 *
 * @return Status of mainloop.
 */
enum mainloop_status ds_events_status_get(void)
{
    return _mainloop_status;
}

void ds_events_stop(enum mainloop_status status)
{
    _mainloop_quit = 1;
    _mainloop_status = status;
}

int ds_events_run(void)
{
    inf("entering mainloop");

    _mainloop_status = MAINLOOP_STATUS_RUNNING;

    while(!_mainloop_quit) {
        struct epoll_event events[MAX_EPOLL_EVENTS];
        int nfds, i;

        nfds = epoll_wait(epollfd, events, MAX_EPOLL_EVENTS, -1);

        inf("mainloop - iterate - nfds=%d ", nfds);
        if (nfds == -1) {
            crit("epoll wait - %m");
            return -1;
        }

        for (i = 0; i < nfds; i++) {
            inf("mainloop - i=%d", i);
            _process_events(&events[i]);
        }
    }

    inf("exiting mainloop");

    return 0;
}

int ds_events_init(void)
{
    epollfd = epoll_create(MAX_EPOLL_EVENTS);
    if (epollfd == -1) {
        err("epoll_create - %m");
        return -1;
    }

    _events_cmds_listen();

    return 0;
}

