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

struct image *read_image(const char *filename);

