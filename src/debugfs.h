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

#ifndef _DEBUGFS_H
#define _DEBUGFS_H

#include <sys/types.h>
#include <stdbool.h>

#define MAX_DEBUGFS_CONFIGS		10

struct hm_cfg {
    char *name;		/* Data type name */
    char *input_name;	/* Input device name */
    char *path;		/* Path to data file */
    char *format;		/* Data format */
    unsigned int rate;	/* Refresh rate */
    unsigned int width;	/* Touchscreen width */
    unsigned int height;	/* Touchscreen height */
    int min;		/* Minimal pressure value */
    int max;		/* Maximumal pressure value */
    bool auto_min;
    bool auto_max;
    bool values;		/* Display pressure values */
    bool gray;		/* Use grayscale */
};

/* debugfs */
int debugfs_get_config(struct hm_cfg cfgs[]);
void print_debugfs_devices(struct hm_cfg cfg[], unsigned int count);
void free_debugfs_configs(struct hm_cfg cfgs[], unsigned int count);
#endif
