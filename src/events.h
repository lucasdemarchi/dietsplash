/*
 *
 * dietsplash
 *
 * Copyright (C) 2010 ProFUSION embedded systems
 *
 * fb.h - framebuffer rendering
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

#ifndef __DIETSPLASH_EVENTS_H
#define __DIETSPLASH_EVENTS_H

#include <stdbool.h>
#include <time.h>

enum timers {
    TIMERS_QUIT = 0,
    TIMERS_NR
};

int ds_events_init(void);
int ds_events_shutdown(void);
int ds_events_run(void);
void ds_events_stop(void);

int ds_events_timer_add(int idx, time_t tv_sec, long tv_nsec, bool oneshot);

#endif
