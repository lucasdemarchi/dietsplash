/*
 *
 * dietsplash
 *
 * Copyright (C) 2014 O.S. Systems Software Ltda.
 *
 * animation.c - animation
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

#include "animation.h"
#include "util.h"
#include "pnmtologo.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <glob.h>
#include <pthread.h>
#include <sys/param.h>

struct animation {
    int fps;
    struct ds_fb *fb;
    struct animation_part *first_part;
};

struct animation_frame {
    char *path;
    struct image *image;
    struct animation_part *part;
    struct animation_frame *next;
};

struct animation_part {
    int count;
    int pause;
    char *path;
    int play_until_complete;
    struct animation_frame *first_frame;
    struct animation_part *next;
};

static void _animation_read_desc(struct animation *anim) {
    FILE *file;
    char *line;
    size_t line_len;
    struct animation_part *last_part = NULL;

    last_part = NULL;
    file = fopen(ANIMATION_PATH "/desc.txt", "r");
    line = NULL;
    line_len = 0;

    while (getline(&line, &line_len, file) != -1) {
        int width;
        int height;
        char type;
        int count;
        int pause;

        char path[PATH_MAX];

        if (anim->fps == 0) {
            sscanf(line, "%d %d %d", &width, &height, &anim->fps);
        } else if (sscanf(line, "%c %d %d %s", &type, &count, &pause, path) == 4) {
            struct animation_part *part = malloc(sizeof(struct animation_part));

            part->count = count;
            part->pause = pause;
            part->path = strdup(path);
            part->play_until_complete = (type == 'c');
            part->first_frame = NULL;
            part->next = NULL;

            if (last_part == NULL) {
                anim->first_part = part;
                last_part = part;
            } else {
                last_part->next = part;
                last_part = part;
            }
        }
    }

    fclose(file);
}

static void _animation_read_frames(struct animation *anim)
{
    struct animation_part *part;

    for (part = anim->first_part; part != NULL; part = part->next) {
        struct animation_frame *last_frame;
        glob_t files_glob;
        char path[255];
        int i;

        last_frame = NULL;

        path[0] = '\0';

        strcat(path, ANIMATION_PATH);
        strcat(path, part->path);
        strcat(path, "/*.ppm");

        glob(path, GLOB_NOCHECK, NULL, &files_glob);

        for (i = 0; i < (int) files_glob.gl_pathc; i++) {
            struct animation_frame *frame = malloc(sizeof(struct animation_frame));
            frame->path = strdup(files_glob.gl_pathv[i]);
            frame->image = NULL;
            frame->part = part;
            frame->next = NULL;

            if (last_frame == NULL) {
                part->first_frame = frame;
                last_frame = frame;
            } else {
                last_frame->next = frame;
                last_frame = frame;
            }
        }

        globfree(&files_glob);
    }
}

static void *animation_mainloop(void *args)
{
    struct animation *anim;
    struct animation_part *part;
    long frame_duration;
    time_t now;
    time_t past;

    anim = args;

    frame_duration = (1 * 1000000) / anim->fps;

    time(&past);

    for (part = anim->first_part; part != NULL; part = part->next) {
        int i;

        for (i = 0; !part->count || i < part->count; i++) {
            struct animation_frame *frame;

            if (anim->fb->exit_pending && !part->play_until_complete)
                break;

            for (frame = part->first_frame; frame != NULL && (!anim->fb->exit_pending || part->play_until_complete); frame = frame->next) {
                time(&now);

                if (frame->image == NULL)
                    frame->image = ds_read_image(frame->path);

                ds_fb_draw_region(anim->fb, frame->image, 0.5, 0.5);

                time(&past);

                usleep(frame_duration - (now - past));
            }

            if (anim->fb->exit_pending && !part->count)
                break;
        }

        if (part->count != 1) {
            struct animation_frame *frame;

            for (frame = part->first_frame; frame != NULL; frame = frame->next) {
                free(frame->path);
                free(frame->image);
            }
        }
    }

    return NULL;
}

void animation_init(struct ds_fb *fb)
{
    struct animation *anim;

    anim = malloc(sizeof(struct animation));
    anim->fb = fb;
    anim->fb->exit_pending = 0;

    _animation_read_desc(anim);
    _animation_read_frames(anim);

    pthread_create(&fb->anim_thread, NULL, animation_mainloop, (void *)anim);
}
