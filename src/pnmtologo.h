#ifndef __DIETSPLASH_PNMTOLOGO_H
#define __DIETSPLASH_PNMTOLOGO_H

struct color {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
};

struct image {
    unsigned int width;
    unsigned int height;
    struct color pixels[];
};

struct image *ds_read_image(const char *filename);

#endif
