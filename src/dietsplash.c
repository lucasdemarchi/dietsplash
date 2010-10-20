#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <errno.h>
#include "util.h"
#include "pnmtologo.h"

#ifdef LOGOFILE
static const char *default_logo = LOGOFILE;
#else
#include "logo.h"
#endif

static void draw_logo(char *fb_data, struct fb_fix_screeninfo *finfo,
               struct fb_var_screeninfo *vinfo)
{
    unsigned long i, j;
    long cy_offset, cx_offset;
    struct image *logo;

#ifdef LOGOFILE
    logo = ds_read_image(default_logo);
#else
    logo = &dietsplash_staticlogo;
#endif

    cx_offset = (vinfo->xres - logo->width) / 2;
    cy_offset = (vinfo->yres - logo->height) / 2;
    
    for (j = 0; j < logo->height; j++) {
        for (i = 0; i < logo->width; i++) {
            long location = (i + vinfo->xoffset + cx_offset) *
                            (vinfo->bits_per_pixel / 8) +
                            (j + vinfo->yoffset + cy_offset) *
                            finfo->line_length;

            *(fb_data + location)     = logo->pixels[j * logo->width + i].blue;
            *(fb_data + location + 1) = logo->pixels[j * logo->width + i].green;
            *(fb_data + location + 2) = logo->pixels[j * logo->width + i].red;
            *(fb_data + location + 3) = 0;
        }
    }
#ifdef LOGOFILE
    free(logo);
#endif
}

int main(int argc, char *argv[])
{
    int fd;
    void *fb_data;
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    long screen_size;

    fd = open("/dev/fb0", O_RDWR);
    if (fd < 0) {
        perror("open failed");
        return 1;
    }

    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fb fix info");
        return 1;
    }

    if (finfo.type != FB_TYPE_PACKED_PIXELS) {
        fprintf(stderr, "Don't know how to deal with fb type %d", finfo.type);
        return 1;
    }

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading fb var info");
        return 1;
    }

    printf("FB: %s\n", finfo.id);
    printf("FB: %dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    screen_size = finfo.line_length * vinfo.yres;

    fb_data = mmap(0, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (fb_data == MAP_FAILED) {
        perror("Error mmapping fb");
        return 1;
    }

    draw_logo(fb_data, &finfo, &vinfo);

    munmap(fb_data, screen_size);
    close(fd);
    return 0;
}
