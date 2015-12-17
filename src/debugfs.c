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

#include "debugfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

#define DEBUGFS_ROOT_DIR		"/sys/kernel/debug"
#define HEATMAP_DIR_PREFIX		"heatmap-"

#define DEBUGFS_NAME_LEN		50
#define DEBUGFS_FORMAT_LEN		7
#define DEBUGFS_PATH_LEN		128


static struct hm_cfg *
init_debugfs_config(struct hm_cfg *cfg)
{
    /* allocate memory for cfg */
    cfg->name = calloc(DEBUGFS_NAME_LEN, sizeof(char));
    if (!cfg->name)
        return 0;

    cfg->input_name = calloc(DEBUGFS_NAME_LEN, sizeof(char));
    if (!cfg->input_name)
        return 0;

    cfg->format = calloc(DEBUGFS_FORMAT_LEN, sizeof(char));
    if (!cfg->format)
        return 0;

    cfg->path = calloc(DEBUGFS_PATH_LEN, sizeof(char));
    if (!cfg->path)
        return 0;

    return cfg;
}

static char *
file_get_contents(char *dirname, char *filename, char *buf, unsigned int count)
{
    char path[DEBUGFS_PATH_LEN];
    char *ret;
    FILE *fp;

    snprintf(path, DEBUGFS_PATH_LEN, "%s/%s", dirname, filename);

    fp = fopen(path, "r");
    ret = fgets(buf, count, fp);
    if (!ret)
        fprintf(stderr, "Unable to read contents from <%s>\n", path);

    fclose(fp);

    return ret;
}

static void
debugfs_scan_config(struct hm_cfg cfgs[], int *idx, char *dirname)
{
    char tmp[DEBUGFS_PATH_LEN];
    char buf[20];
    struct dirent *ep;
    DIR *sub_dp, *dp = opendir(dirname);

    /* scan for data sources in sub-directories */
    while ((ep = readdir(dp))) {
        /* ensure valid directory */
        if (strchr(ep->d_name, '.'))
            continue;

        sprintf(tmp, "%s/%s", dirname, ep->d_name);
        sub_dp = opendir(tmp);
        if (!sub_dp)
            continue;

        struct hm_cfg *cfg = init_debugfs_config(&cfgs[(*idx)++]);
        if (!cfg) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            return;
        }

        /* get configuration from files */
        if (file_get_contents(tmp, "width", buf, sizeof(buf)))
            cfg->width = atoi(buf);

        if (file_get_contents(tmp, "height", buf, sizeof(buf)))
            cfg->height = atoi(buf);

        file_get_contents(tmp, "name", cfg->name, DEBUGFS_NAME_LEN);

        file_get_contents(tmp, "input_name", cfg->input_name, DEBUGFS_NAME_LEN);

        file_get_contents(tmp, "format", cfg->format, DEBUGFS_FORMAT_LEN);

        /* set path */
        snprintf(cfg->path, DEBUGFS_PATH_LEN, "%s/%s", tmp, "data");

        closedir(sub_dp);

    }
    closedir(dp);
}

static void
free_debugfs_config(struct hm_cfg *cfg)
{
    if (cfg->path)
        free(cfg->path);

    if (cfg->name)
        free(cfg->name);

    if (cfg->input_name)
        free(cfg->input_name);

    if (cfg->format)
        free(cfg->format);
}

int
debugfs_get_config(struct hm_cfg cfgs[])
{
    int i = 0;
    char path[DEBUGFS_PATH_LEN];
    DIR *dp;
    struct dirent *ep;

    dp = opendir(DEBUGFS_ROOT_DIR);
    if (dp) {
        while ((ep = readdir(dp))) {
            if (!strncmp(ep->d_name, HEATMAP_DIR_PREFIX, strlen(HEATMAP_DIR_PREFIX))) {

                /* configure base dir */
                snprintf(path, DEBUGFS_PATH_LEN, "%s/%s", DEBUGFS_ROOT_DIR, ep->d_name);

                debugfs_scan_config(cfgs, &i, path);

                if (i >= MAX_DEBUGFS_CONFIGS)
                    break;
            }
        }
        closedir(dp);
    } else
        fprintf(stderr, "Couldn't open the debugfs directory.\n");

    return i;
}

void
print_debugfs_devices(struct hm_cfg cfg[], unsigned int count)
{
    unsigned int i = 0;

    fprintf(stdout, "\nFound %d devices:\n\n", count);

    if (count > 0) {
        while (i != count) {
            fprintf(stdout, "No: %d\n"
                    "Name: %s\n"
                    "Input Name: %s\n"
                    "Path: %s\n"
                    "Width: %d\n"
                    "Height: %d\n"
                    "Format %s\n\n",
                    i, cfg[i].name, cfg[i].input_name,
                    cfg[i].path, cfg[i].width, cfg[i].height,
                    cfg[i].format);
            i++;
        }

        fprintf(stdout,
                "Use -f <No> to select one of the above devices\n");
    }
}

void
free_debugfs_configs(struct hm_cfg cfgs[], unsigned int count)
{
    unsigned int i;
    for (i = 0; i <= count ; i++) {
        free_debugfs_config(&cfgs[i]);
    }
}
