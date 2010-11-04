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

#ifndef __DIETSPLASH_FB_H
#define __DIETSPLASH_FB_H

#include <stdbool.h>

enum ds_image_format {
    BGRA8888
};

struct ds_fb {
    long screen_size;
    long stride;
    int xres;
    int yres;
    int xres_virtual;
    int yres_virtual;
    int xoffset;
    int yoffset;
    int type;
    char *data;
    enum ds_image_format image_format;
};

void ds_fb_draw_logo(struct ds_fb *ds_fb);
int ds_fb_init(struct ds_fb *ds_fb);
int ds_fb_shutdown(struct ds_fb *ds_fb);

#endif
