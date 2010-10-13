/*
 *  Convert a logo in ASCII PNM format to C source suitable for inclusion in
 *  dietsplash. Based on source file for doing this conversion for the Linux
 *  Kernel.
 *
 *  Copyright (C) 2001-2003 Geert Uytterhoeven <geert@linux-m68k.org>
 *  Copyright (C) 2010 ProFUSION embedded systems <lucas.demarchi@profusion.mobi>
 */
#include "pnmtologo.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "util.h"

static unsigned int get_number(FILE *fp)
{
    int c, val;

    /* Skip leading whitespace */
    do {
	c = fgetc(fp);
	if (c == EOF)
	    die("end of file\n");
	if (c == '#') {
	    /* Ignore comments 'till end of line */
	    do {
		c = fgetc(fp);
		if (c == EOF)
		    die("end of file\n");
	    } while (c != '\n');
	}
    } while (isspace(c));

    /* Parse decimal number */
    val = 0;
    while (isdigit(c)) {
	val = 10*val+c-'0';
	c = fgetc(fp);
	if (c == EOF)
	    die("end of file\n");
    }
    return val;
}

static unsigned int get_number255(FILE *fp, unsigned int maxval)
{
    unsigned int val = get_number(fp);
    return (255*val+maxval/2)/maxval;
}

struct image *read_image(const char *filename)
{
    FILE *fp;
    unsigned int i, j;
    int magic;
    unsigned int maxval, logo_width, logo_height;
    struct image *logo, *tmp;

    /* open image file */
    fp = fopen(filename, "r");
    if (!fp)
	die("Cannot open file %s: %s\n", filename, strerror(errno));

    /* check file type and read file header */
    magic = fgetc(fp);
    if (magic != 'P')
	die("%s is not a PNM file\n", filename);
    magic = fgetc(fp);
    switch (magic) {
	case '1':
	case '2':
	case '3':
	    /* Plain PBM/PGM/PPM */
	    break;

	case '4':
	case '5':
	case '6':
	    /* Binary PBM/PGM/PPM */
	    die("%s: Binary PNM is not supported\n"
		"Use pnmnoraw(1) to convert it to ASCII PNM\n", filename);

	default:
	    die("%s is not a PNM file\n", filename);
    }
    
    logo = malloc(sizeof(*logo));
    if (!logo)
        die("%s\n", strerror(errno));

    logo->width = get_number(fp);
    logo->height = get_number(fp);

    /* allocate image data */
    tmp = realloc(logo, sizeof(*logo) + logo->height * logo->width *
                            sizeof(struct color));
    if (!tmp) {
        free(logo);
	die("%s\n", strerror(errno));
    }
    logo = tmp;
    
    /* read image data */
    switch (magic) {
	case '1':
	    /* Plain PBM */
            for (i = 0; i < logo->height * logo->width; i++)
                logo->pixels[i].red = logo->pixels[i].green =
			logo->pixels[i].blue = 255*(1-get_number(fp));
	    break;

	case '2':
	    /* Plain PGM */
	    maxval = get_number(fp);
	    for (i = 0; i < logo->height * logo->width; i++)
                logo->pixels[i].red = logo->pixels[i].green =
			logo->pixels[i].blue = get_number255(fp, maxval);
	    break;

	case '3':
	    /* Plain PPM */
	    maxval = get_number(fp);
	    for (i = 0; i < logo->height * logo->width; i++) {
		    logo->pixels[i].red = get_number255(fp, maxval);
		    logo->pixels[i].green = get_number255(fp, maxval);
		    logo->pixels[i].blue = get_number255(fp, maxval);
            }
	    break;
    }

    /* close file */
    fclose(fp);

    return logo;
}

