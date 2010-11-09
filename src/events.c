#include "log.h"
#include "events.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>

static bool _mainloop_quit;

static int epollfd = -1;

static int _timers[TIMERS_NR] = { -1 };

#define MAX_EPOLL_EVENTS 5


int ds_events_shutdown(void)
{
    int i, r = 0;

    for (i = 0; i < TIMERS_NR; i++) {
        if (_timers[i] != -1 && (r |= close(_timers[i])) == -1)
            err("shutdown timer %d - %m", i);
    }

    if((r |= close(epollfd)) == -1)
        err("close epoll - %m");

    return r;
}

static void _process_events(struct epoll_event *ev)
{
    uint64_t buf;

    inf("processing events for fd %d", ev->data.fd);

    if (ev->data.fd == _timers[TIMERS_QUIT]) {
        while (read(ev->data.fd, &buf, sizeof(buf)) > 0)
            ;

        if (errno != EAGAIN)
            err("read quit timer");

        _mainloop_quit = 1;
    }
}

static int _watch_fd(int fd)
{
    struct epoll_event ev;

    inf("_watch_fd %d", fd);

    ev.events = EPOLLIN;
    ev.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        err("epoll_ctl: timer fd - %m");
        close(fd);
        return -1;
    }

    return fd;
}

int ds_events_timer_add(int idx, time_t tv_sec, long tv_nsec, bool oneshot)
{
    struct itimerspec tm = { { 0 }, { 0 } };

    assert(idx < TIMERS_NR && _timers[idx] == -1);

    _timers[idx] = timerfd_create(CLOCK_MONOTONIC,
                                  TFD_NONBLOCK | TFD_CLOEXEC);
    if (_timers[idx] == -1) {
        err("timerfd_create - %m");
        return -1;
    }

    tm.it_value.tv_sec = tv_sec;
    tm.it_value.tv_nsec = tv_nsec;

    if (!oneshot) {
        tm.it_interval.tv_sec = tv_sec;
        tm.it_interval.tv_nsec = tv_nsec;
    }

    if (timerfd_settime(_timers[idx], 0, &tm, NULL) == -1) {
        err("timerfd_settime - %m");
        close(_timers[idx]);
        return -1;
    }

    return _watch_fd(_timers[idx]);
}

int ds_events_run(void)
{
    inf("entering mainloop");
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

    return 0;
}

