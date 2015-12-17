/*
 * Copyright (c) 2014 Zodiac Inflight Innovations
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "heatmap.h"

#define MAXGRAY 24
#define MAXCOLOR 216

struct color {
    short red;
    short green;
    short blue;
};

void
hm_display_init(struct hm_cfg *cfg)
{
    if (has_colors() == FALSE) {
        endwin();
        fatalx("heatmap", "terminal doesn't support colors");
    }
    start_color();
    if (COLORS < 256) {
        endwin();
        fatalx("heatmap", "terminal doesn't support 256 colors");
    }
    if (!can_change_color()) cfg->gray = true;

    if (cfg->gray) {
        for (size_t i = 0; i < MAXGRAY; i++) {
            init_pair(i, 255-i, 232+i);
        }
    } else {
        init_color(0, 0, 0, 0);
        for (size_t i = 0; i < MAXCOLOR; i++) {
            struct color colors[] = {
                {0,    0,    1000},
                {0,    1000, 1000},
                {0,    1000, 0},
                {1000, 1000, 0},
                {1000, 0,    0}
            };
            struct color color;
            size_t j, k;
            if (i < MAXCOLOR/4) {
                j = i;
                k = 1;
            } else if (i < MAXCOLOR/2) {
                j = i - MAXCOLOR/4;
                k = 2;
            } else if (i < MAXCOLOR*3/4) {
                j = i - MAXCOLOR/2;
                k = 3;
            } else {
                j = i - MAXCOLOR*3/4;
                k = 4;
            }
            color.red = 4 * j * (colors[k].red - colors[k-1].red) / MAXCOLOR + colors[k-1].red;
            color.green = 4 * j * (colors[k].green - colors[k-1].green) / MAXCOLOR + colors[k-1].green;
            color.blue = 4 * j * (colors[k].blue - colors[k-1].blue) / MAXCOLOR + colors[k-1].blue;
            if (color.red < 0) color.red = 0;
            else if (color.red > 1000) color.red = 1000;
            if (color.green < 0) color.green = 0;
            else if (color.green > 1000) color.green = 1000;
            if (color.blue < 0) color.blue = 0;
            else if (color.blue > 1000) color.blue = 1000;

            init_color(i+16, color.red, color.green, color.blue);
            init_pair(i, 0, i+16);
        }
    }
}

void
hm_display_data(struct hm_cfg *cfg, int *data, size_t len)
{
    size_t columns = cfg->width;   /* Number of columns */
    size_t lines = len / columns; /* Number of lines */
    int sheight, swidth;	      /* Screen height and width */
    getmaxyx(stdscr, sheight, swidth);

    /* Compute height and width of one cell */
    size_t cwidth = swidth / columns;
    size_t cheight = sheight / lines;
    if (cwidth == 0) cwidth = 1;
    if (cheight == 0) cheight = 1;

    /* Manage centering */
    ssize_t offsetx = (swidth - cwidth * columns) / 2;
    ssize_t offsety = (sheight - cheight * lines) / 2;
    if (offsetx < 0) offsetx = 0;
    if (offsety < 0) offsety = 0;

    for (size_t i = 0; i < len; i++) {
        short max;
        if (cfg->gray)
            max = MAXGRAY;
        else
            max = MAXCOLOR;
        ssize_t gray = (data[i] - cfg->min) * max / (cfg->max - cfg->min);
        if (gray >= max) gray = max - 1;
        if (gray < 0) gray = 0;
        attrset(COLOR_PAIR(gray));
        for (size_t j = 0; j < cheight; j++) {
            move(offsety + (i / cfg->width) * cheight + j,
                 offsetx + (i % cfg->width) * cwidth);
            if (cfg->values && j == cheight / 2 && cwidth > 3) {
                printw("% *d", cwidth, data[i]);
            } else {
                printw("% *s", cwidth, " ");
            }
        }
    }
    refresh();
}
