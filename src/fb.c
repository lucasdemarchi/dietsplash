/*
 *
 * dietsplash
 *
 * Copyright (C) 2010 ProFUSION embedded systems
 *
 * fb.c - framebuffer rendering
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

#include "log.h"
#include "fb.h"
#include "pnmtologo.h"
#include "util.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#ifndef ENABLE_ANIMATION
#ifdef BACKGROUND_FILE
static const char *background_filename = BACKGROUND_FILE;
#else
#include "background.h"
#endif
#else
#include "animation.h"
#endif

void ds_fb_draw_region(struct ds_fb *fb, const struct image *region,
                       float xalign, float yalign)
{
    long i, j, xoffset, yoffset;
    long w = region->width;
    long h = region->height;

    assert(fb);
    assert(region);

    if (fb->xres < w) {
        wrn("fb xres (%u) is less than region size (%u)", fb->xres, w);
        w = fb->xres;
    }

    if (fb->yres < h) {
        wrn("fb yres (%u) is less than region size (%u)", fb->yres, h);
        h = fb->yres;
    }

    xoffset = (long)((fb->xres - w) * xalign);
    yoffset = (long)((fb->yres - h) * yalign);

    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            long location = ((i + fb->xoffset + xoffset) * (fb->bits_per_pixel / 8)) +
                            (j + fb->yoffset + yoffset) * fb->stride;

            long pixel = (region->pixels[j * w + i].blue >> (8-fb->blue_length)) | 
                         ((region->pixels[j * w + i].green >> (8-fb->green_length)) << fb->green_offset) | 
                         ((region->pixels[j * w + i].red >> (8-fb->red_length)) << fb->red_offset);

            int k;
            for(k = 0; k < fb->bits_per_pixel/8; k++) {
                *(fb->data + location + k ) = pixel >> k*8;
            }
        }
    }
}

#ifndef ENABLE_ANIMATION
static void _fb_draw_bg(struct ds_fb *fb)
{
    struct image *bg;

#ifdef BACKGROUND_FILE
    bg = ds_read_image(background_filename);
#else
    bg = &dietsplash_static_background;
#endif

    ds_fb_draw_region(fb, bg, 0.5, 0.5);

#ifdef BACKGROUND_FILE
    free(bg);
#endif
}
#endif

int ds_fb_init(struct ds_fb *ds_fb)
{
    int ret = 0, fd;
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;

    ret = ds_fs_setup("/dev/fb0");
    if (ret < 0)
        goto ret_on_err;

    fd = open("/dev/fb0", O_RDWR);
    if (fd < 0) {
        crit("open failed -- %m");
        ret = -errno;
        goto ret_on_err;
    }

    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        crit("reading fb fix info -- %m");
        ret = -errno;
        goto close_on_err;
    }

    if (finfo.type != FB_TYPE_PACKED_PIXELS) {
        crit("don't know how to deal with fb type %d", finfo.type);
        ret = 1;
        goto close_on_err;
    }

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        crit("reading fb var info -- %m");
        ret = -errno;
        goto close_on_err;
    }

    /* dual monitor does not plays well with framebuffer. Logo will be
     * centralized with regard to visible resolution, so it might not be
     * centralized in all monitors
     */
    if (vinfo.xres != vinfo.xres_virtual || vinfo.yres != vinfo.yres_virtual)
        wrn("Virtual resolution is not the same of visible one. Logo will be " \
            "centralized with regard to visible resolution");

    ds_fb->xres = vinfo.xres;
    ds_fb->yres = vinfo.yres;
    ds_fb->xres_virtual = vinfo.xres_virtual;
    ds_fb->yres_virtual = vinfo.yres_virtual;
    ds_fb->xoffset = vinfo.xoffset;
    ds_fb->yoffset = vinfo.yoffset;
    ds_fb->type = finfo.type;
    ds_fb->stride = finfo.line_length;
    ds_fb->screen_size = ds_fb->stride * ds_fb->yres;
    ds_fb->red_length = vinfo.red.length;
    ds_fb->red_offset = vinfo.red.offset;
    ds_fb->green_length = vinfo.green.length;
    ds_fb->green_offset = vinfo.green.offset;
    ds_fb->blue_length = vinfo.blue.length;
    ds_fb->blue_offset = vinfo.blue.offset;
    ds_fb->bits_per_pixel = vinfo.bits_per_pixel;

    inf("FB %s", finfo.id);
    inf("FB %dx%d, %dbpp", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);
    inf("FB %dx%d, virtual", vinfo.xres_virtual, vinfo.yres_virtual);

    ds_fb->data = mmap(0, ds_fb->screen_size,
                       PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (ds_fb->data == MAP_FAILED) {
        crit("fb mmapping -- %m");
        ret = -errno;
        goto close_on_err;
    }

    if (close(fd) == -1)
        err("fb closing fd -- %m");

#ifndef ENABLE_ANIMATION
    _fb_draw_bg(ds_fb);
#else
    animation_init(ds_fb);
#endif

    return 0;

close_on_err:
    close(fd);
ret_on_err:
    return ret;
}

int ds_fb_shutdown(struct ds_fb *ds_fb)
{
    int ret = 0;
    assert(ds_fb);
    assert(ds_fb->data);

    // we unmap, and log in case of error, but continue shutting down
    if(munmap(ds_fb->data, ds_fb->screen_size) == -1) {
        err("fb munmap -- %m");
        ret = -1;
    }

    ds_fb->data = NULL;
    ds_fb->screen_size = 0;

    if (ds_fs_shutdown() < 0)
        ret = -1;

    return ret;
}
