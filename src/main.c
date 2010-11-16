/*
 *
 * dietsplash
 *
 * Copyright (C) 2010 ProFUSION embedded systems
 *
 * main.c - dietsplash daemon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 */

#include "events.h"
#include "fb.h"
#include "log.h"
#include "util.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct ds_info {
    struct ds_fb fb;
    bool testing;
};
static struct ds_info ds_info;


int main(int argc, char *argv[])
{
    pid_t pid;
    ds_log_init(argv[0]);

    ds_info.testing = (getpid() != 1);

    inf("running mode -- %s", ds_info.testing ? "testing":"system");

    if (!ds_info.testing) {
        pid = fork();
        if (pid) {
            char *argv0 = argv[0];
            /* first, change our argv[0], then exec */
            argv[0] = basename(REAL_INIT);
            execv(REAL_INIT, argv);

            argv[0] = argv0;
            err("Failed to exec %s", REAL_INIT);
            exit(1);
        }
    }

    if (ds_events_init() == -1)
        goto err_on_events;

    if (ds_fb_init(&ds_info.fb))
        goto err_on_fb;

    ds_console_setup();
    ds_fb_draw_logo(&ds_info.fb);

    ds_events_cmds_listen();

    if (ds_info.testing) {
        ds_events_timer_add(TIMERS_QUIT, 5, 0, true);
        ds_events_run();
        ds_console_restore();
    }

    ds_events_shutdown();
    ds_fb_shutdown(&ds_info.fb);
    ds_log_shutdown();

    return 0;

err_on_events:
err_on_fb:
    ds_log_shutdown();

    return 1;
}
