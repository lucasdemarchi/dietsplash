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
