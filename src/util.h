/*
 *
 * dietsplash
 *
 * Copyright (C) 2010 ProFUSION embedded systems <lucas.demarchi@profusion.mobi>
 *
 * util.h - utility stuff that doesn't deserve a file on its own
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

#ifndef __DIETSPLASH_UTIL_H
#define __DIETSPLASH_UTIL_H

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define DIE_PREFIX "[" PACKAGE_NAME "] ERR: "
#define LOG_SUFFIX "\n"

void _die(const char *fmt, ...)  __attribute__((noreturn)) __attribute__((format (printf, 1, 2)));
#define die(x, ...) \
    _die(DIE_PREFIX x LOG_SUFFIX, ## __VA_ARGS__)

int ds_fs_setup(const char *dev);
int ds_fs_shutdown(void);

int ds_console_setup(void);
int ds_console_restore(void);

#endif
