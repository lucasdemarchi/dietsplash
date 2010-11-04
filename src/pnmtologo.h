/*
 *
 * dietsplash
 *
 * Copyright (C) 2010 ProFUSION embedded systems <lucas.demarchi@profusion.mobi>
 *
 * pnmtologo.h - Convert a logo in ASCII PNM format to C source suitable for
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
