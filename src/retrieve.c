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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>

size_t
hm_retrieve_data(struct hm_cfg *cfg, int **data)
{
    static size_t allocated = 0;
    size_t len = 0;
    int fd = open(cfg->path, O_RDONLY);
    if (fd == -1) return 0;

    /* Initial memory allocation */
    if (allocated == 0) allocated = cfg->width;
    *data = malloc(allocated * sizeof(int));
    if (*data == NULL) goto error;

    while (1) {
        /* Allocate more memory if needed */
        if (allocated == len) {
            allocated += cfg->width;
            int *new = realloc(*data, allocated * sizeof(int));
            if (new == NULL) goto error;
            *data = new;
        }

        /* Read a 16-bit int */
        int16_t blob;
        ssize_t ret;
        ret = read(fd, &blob, sizeof(int16_t));
        if (ret == 0) {
            close(fd);
            return len;
        }
        if (ret == -1 && errno == EINTR) continue;
        if (ret != 2) goto error;

        /* Store it */
        (*data)[len] = blob;
        if (cfg->auto_min && blob < cfg->min) cfg->min = blob;
        if (cfg->auto_max && blob > cfg->max) cfg->max = blob;
        len += 1;
    }

error:
    close(fd);
    free(*data);
    *data = NULL;
    return 0;
}
