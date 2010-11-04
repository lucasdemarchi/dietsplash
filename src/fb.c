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

#ifdef LOGOFILE
static const char *logo_filename = LOGOFILE;
#else
#include "logo.h"
#endif

void ds_fb_draw_logo(struct ds_fb *ds_fb)
{
    unsigned long i, j;
    long cy_offset, cx_offset;
    struct image *logo;

#ifdef LOGOFILE
    logo = ds_read_image(logo_filename);
#else
    logo = &dietsplash_staticlogo;
#endif

    cx_offset = (ds_fb->xres - logo->width) / 2;
    cy_offset = (ds_fb->yres - logo->height) / 2;

    for (j = 0; j < logo->height; j++) {
        for (i = 0; i < logo->width; i++) {
            long location = ((i + ds_fb->xoffset + cx_offset) << 2) +
                            (j + ds_fb->yoffset + cy_offset) * ds_fb->stride;

            *(ds_fb->data + location)     = logo->pixels[j * logo->width + i].blue;
            *(ds_fb->data + location + 1) = logo->pixels[j * logo->width + i].green;
            *(ds_fb->data + location + 2) = logo->pixels[j * logo->width + i].red;
            *(ds_fb->data + location + 3) = 0;
        }
    }
#ifdef LOGOFILE
    free(logo);
#endif
}

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

    if (!(vinfo.bits_per_pixel == 32 &&
                vinfo.blue.offset == 0 && vinfo.blue.length == 8 &&
                vinfo.green.offset == 8 && vinfo.green.length == 8 &&
                vinfo.red.offset == 16 && vinfo.red.length == 8)) {
        crit("Only BGRA8888 is implemented, your fb is:\n" \
                "\tb_offset: %d\tb_length: %d\n" \
                "\tg_offset: %d\tg_length: %d\n" \
                "\tr_offset: %d\tr_length: %d", \
                vinfo.blue.offset, vinfo.blue.length,
                vinfo.green.offset, vinfo.green.length,
                vinfo.red.offset, vinfo.red.length);
        ret = 1;
        goto close_on_err;
    }

    /* dual monitor does not plays well with framebuffer. Logo will be
     * centralized with regard to visible resolution, so it might not be
     * centralized in all monitors
     */
    if (vinfo.xres != vinfo.xres_virtual || vinfo.yres != vinfo.yres_virtual)
        inf("Virtual resolution is not the same of visible one. Logo will be " \
            "centralized with regard to visible resolution");

    ds_fb->image_format = BGRA8888;
    ds_fb->xres = vinfo.xres;
    ds_fb->yres = vinfo.yres;
    ds_fb->xres_virtual = vinfo.xres_virtual;
    ds_fb->yres_virtual = vinfo.yres_virtual;
    ds_fb->xoffset = vinfo.xoffset;
    ds_fb->yoffset = vinfo.yoffset;
    ds_fb->type = finfo.type;
    ds_fb->stride = finfo.line_length;
    ds_fb->screen_size = ds_fb->stride * ds_fb->yres;

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
