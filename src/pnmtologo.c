/*
 *
 * dietsplash
 *
 * Copyright (C) 2001-2003 Geert Uytterhoeven <geert@linux-m68k.org>
 * Copyright (C) 2010 ProFUSION embedded systems <lucas.demarchi@profusion.mobi>
 *
 * pnmtologo.c - Convert a logo in ASCII PNM format to C source suitable for
 * inclusion in dietsplash. Based on source file for doing this conversion for
 * the Linux Kernel.
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

#include "pnmtologo.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	val = 10 * val + c - '0';
	c = fgetc(fp);
	if (c == EOF)
	    die("end of file\n");
    }
    return val;
}

static unsigned int get_number255(FILE *fp, unsigned int maxval)
{
    unsigned int val = get_number(fp);
    return (255 * val + (maxval/2)) / maxval;
}

static unsigned int get_byte(FILE *fp)
{
    int c;

    c = fgetc(fp);
    if (c == EOF) {
	die("end of file\n");
    }

    return (unsigned int)c;
}

static unsigned int get_byte255(FILE *fp, unsigned int maxval)
{
    unsigned int val = get_byte(fp);
    return (255 * val + (maxval/2)) / maxval;
}

struct image *ds_read_image(const char *filename)
{
    FILE *fp;
    unsigned int i, j;
    int magic;
    unsigned int maxval;
    struct image *logo, *tmp;

    /* open image file */
    fp = fopen(filename, "r");
    if (!fp)
	die("Cannot open file %s: %m\n", filename);

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
	    break;

	default:
	    die("%s is not a PNM file\n", filename);
    }

    logo = malloc(sizeof(*logo));
    if (!logo)
        die("%m\n");

    logo->width = get_number(fp);
    logo->height = get_number(fp);

    /* allocate image data */
    tmp = realloc(logo, sizeof(*logo) + logo->height * logo->width *
                            sizeof(struct color));
    if (!tmp) {
        free(logo);
	die("%m\n");
    }
    logo = tmp;

    /* read image data */
    switch (magic) {
	case '1':
	    /* Plain PBM */
            for (i = 0; i < logo->height * logo->width; i++)
                logo->pixels[i].red = logo->pixels[i].green =
			logo->pixels[i].blue = 255 * (1 - get_number(fp));
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

	case '4':
	    /* Binary PBM */
	    {
	    struct color * pDst = &logo->pixels[0];
	    for (i = 0; i < logo->height; ++i) {
		    for (j = 0; j < (logo->width/8); ++j) {
			    unsigned int val = get_byte(fp);
			    pDst->red = pDst->green = pDst->blue = ((val & 0x80) ? 0 : 255); ++pDst;
			    pDst->red = pDst->green = pDst->blue = ((val & 0x40) ? 0 : 255); ++pDst;
			    pDst->red = pDst->green = pDst->blue = ((val & 0x20) ? 0 : 255); ++pDst;
			    pDst->red = pDst->green = pDst->blue = ((val & 0x10) ? 0 : 255); ++pDst;
			    pDst->red = pDst->green = pDst->blue = ((val & 0x08) ? 0 : 255); ++pDst;
			    pDst->red = pDst->green = pDst->blue = ((val & 0x04) ? 0 : 255); ++pDst;
			    pDst->red = pDst->green = pDst->blue = ((val & 0x02) ? 0 : 255); ++pDst;
			    pDst->red = pDst->green = pDst->blue = ((val & 0x01) ? 0 : 255); ++pDst;
		    }
		    if (logo->width % 8 != 0) {
			unsigned int val = get_byte(fp);
			unsigned int msk = 0x80;
			for (j = 0; j < (logo->width % 8); ++j) {
			    pDst->red = pDst->green = pDst->blue = ((val & msk) ? 0 : 255);
			    ++pDst;
			    msk >>= 1;
			}
		    }
	    }
	    }
	    break;

	case '5':
	    /* Binary PGM */
	    maxval = get_number(fp);
	    for (i = 0; i < logo->height * logo->width; i++)
                logo->pixels[i].red = logo->pixels[i].green =
			logo->pixels[i].blue = get_byte255(fp, maxval);
	    break;

	case '6':
	    /* Binary PPM */
	    maxval = get_number(fp);
	    for (i = 0; i < logo->height * logo->width; i++) {
		    logo->pixels[i].red = get_byte255(fp, maxval);
		    logo->pixels[i].green = get_byte255(fp, maxval);
		    logo->pixels[i].blue = get_byte255(fp, maxval);
            }
	    break;
    }

    /* close file */
    fclose(fp);

    return logo;
}

