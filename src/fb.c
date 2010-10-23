#include "log.h"
#include "fb.h"
#include "pnmtologo.h"
#include "util.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
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
    int ret = 0;
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;

    ds_fb->need_fs_setup = ds_fs_setup("/dev/fb0");
    if (ds_fb->need_fs_setup < 0) {
        ret = 1;
        goto ret_on_err;
    }

    ds_fb->fd = open("/dev/fb0", O_RDWR);
    if (ds_fb->fd < 0) {
        crit("open failed -- %m");
        ret = -errno;
        goto ret_on_err;
    }

    if (ioctl(ds_fb->fd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        crit("reading fb fix info -- %m");
        ret = -errno;
        goto close_on_err;
    }

    if (finfo.type != FB_TYPE_PACKED_PIXELS) {
        crit("don't know how to deal with fb type %d", finfo.type);
        ret = 1;
        goto close_on_err;
    }

    if (ioctl(ds_fb->fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
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

    /* when using dual monitor, set up xoffset and yoffset, so the image is
     * centralized on both monitors
     */
    if (vinfo.xres != vinfo.xres_virtual || vinfo.yres != vinfo.yres_virtual) {
        vinfo.xoffset = (vinfo.xres_virtual - vinfo.xres) / 2;
        vinfo.yoffset = (vinfo.yres_virtual - vinfo.yres) / 2;

        vinfo.activate |= FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;

        if (ioctl(ds_fb->fd, FBIOPUT_VSCREENINFO, &vinfo) < 0) {
            err("setting resolution for dual monitor");
            ret = 1;
            goto close_on_err;
        }
        else {
            inf("FB xoffset x yoffset, %d x %d", vinfo.xoffset, vinfo.yoffset);
        }

        ioctl(ds_fb->fd, FBIOGET_VSCREENINFO, &vinfo);
    }

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
                       PROT_READ | PROT_WRITE, MAP_SHARED, ds_fb->fd, 0);

    if (ds_fb->data == MAP_FAILED) {
        crit("fb mmapping -- %m");
        ret = -errno;
        goto close_on_err;
    }

    return 0;

close_on_err:
    close(ds_fb->fd);
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
        ret = errno;
    }

    ds_fb->data = NULL;
    ds_fb->screen_size = 0;

    if (close(ds_fb->fd) == -1) {
        err("fb closing fd -- %m");
        ret |= errno;
    }

    ret |= -ds_fs_shutdown();
    return -ret;
}

static const char *tty_path = "/dev/tty0";

/**
 * Let console in graphics mode, effectivelly disabling fbcon. This is
 * accomplished by calling ioctl(fd, KDSETMODE, KD_GRAPHICS)
 *
 * @param ds_fb the framebuffer identifier in dietsplash
 */
int ds_fb_console_off(struct ds_fb *ds_fb)
{
    int ret;

    ds_fb->console_dev = open(tty_path, O_WRONLY);
    if (ds_fb->console_dev < 0) {
        ret = errno;
        goto exit_on_err;
    }

    if (ioctl(ds_fb->console_dev, KDSETMODE, KD_GRAPHICS) == -1)
        goto close_on_err;

    return 0;

close_on_err:
    ret = errno;
    close(ds_fb->console_dev);

exit_on_err:
    err("Failed to go in graphics mode -- %m");
    return -ret;
}

int ds_fb_console_on(struct ds_fb *ds_fb)
{
    int ret;
    assert(ds_fb->console_dev);

    if (ioctl(ds_fb->console_dev, KDSETMODE, KD_TEXT) == -1)
        goto close_on_err;

    return 0;

close_on_err:
    ret = errno;
    close(ds_fb->console_dev);

    err("Failed to return to text mode -- %m");
    return -ret;
}
