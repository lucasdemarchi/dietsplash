/*
 *
 * dietsplash
 *
 * Copyright (C) 2010 ProFUSION embedded systems
 *
 * log.h - Logging
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

#ifndef __DIETSPLASH_LOG_H_
#define __DIETSPLASH_LOG_H_

#include <stdarg.h>

void ds_log_init(const char *argv0);
void ds_log_shutdown(void);
void ds_log(int level, const char *file, int line, const char *func, const char *fmt, ...);
int ds_log_get_current_level(void);

#define LOG_CRITICAL  0
#define LOG_ERROR     1
#define LOG_WARNING   2
#define LOG_INFO      3
#define LOG_DEBUG     4

#define crit(fmt, ...) ds_log(LOG_CRITICAL, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define err(fmt, ...) ds_log(LOG_ERROR, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define wrn(fmt, ...) ds_log(LOG_WARNING, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define inf(fmt, ...) ds_log(LOG_INFO, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define dbg(fmt, ...) ds_log(LOG_DEBUG, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)

#endif
