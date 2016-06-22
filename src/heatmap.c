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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>

extern const char *__progname;

static void
usage(void)
{
    fprintf(stderr, "Usage: %s [OPTIONS]\n",
            __progname);
    fprintf(stderr, "Version: %s\n", PACKAGE_STRING);
    fprintf(stderr, "\n");
    fprintf(stderr, "-D, --debug      Be more verbose.\n");
    fprintf(stderr, "-d n, --devno    v4l device number.\n");
    fprintf(stderr, "-i n, --input    v4l input number.\n");
    fprintf(stderr, "-r R, --rate R   Refresh rate (default: %s).\n", HM_DEFAULT_RATE);
    fprintf(stderr, "-w W, --width W  Touchscreen width (default: %s).\n", HM_DEFAULT_WIDTH);
    fprintf(stderr, "-m M, --min M    Minimum heatmap value (default: %s).\n", HM_DEFAULT_MIN);
    fprintf(stderr, "-M M, --max M    Maximum heatmap value (default: %s).\n", HM_DEFAULT_MAX);
    fprintf(stderr, "-V, --values     Display heatmap values.\n");
    fprintf(stderr, "-s, --scan-v4l   Display scan results showing debug v4l devices.\n");
    fprintf(stderr, "-g, --gray       Use grayscale.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "see manual page " PACKAGE "(8) for more information\n");
}

static bool stop = false;
static void
hm_terminate()
{
    stop = true;
}

static bool resize = false;
static void
hm_resize()
{
    resize = true;
}

static int
hm_minmax_value(const char *value, int auto_value)
{
    if (!strcmp(value, "auto"))
        return auto_value;

    long val;
    char *end;
    errno = 0;
    val = strtol(value, &end, 10);
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) ||
        (errno != 0 && val == 0) || *end != '\0') {
        fprintf(stderr, "invalid min/max value should be an integer, not `%s'\n",
                value);
        usage();
        exit(1);
    }
    return val;
}

static int
hm_min_value(const char *value)
{
    return hm_minmax_value(value, INT_MAX);
}

static int
hm_max_value(const char *value)
{
    return hm_minmax_value(value, INT_MIN);
}

int
main(int argc, char *argv[])
{
    int debug = 1;
    int devno = 0;
    int input = 0;
    int ch;

    struct hm_cfg cfg = {
        .rate = atoi(HM_DEFAULT_RATE),
        .width = 0,
        .min = hm_min_value(HM_DEFAULT_MIN),
        .max = hm_max_value(HM_DEFAULT_MAX)
    };

    static struct option long_options[] = {
        { "debug", no_argument, 0, 'D' },
        { "help",  no_argument, 0, 'h' },
        { "version", no_argument, 0, 'v' },
        { "devno", required_argument, 0, 'd' },
        { "rate", required_argument, 0, 'r' },
        { "width", required_argument, 0, 'w' },
        { "min", required_argument, 0, 'm' },
        { "max", required_argument, 0, 'M' },
        { "values", no_argument, 0, 'V' },
        { "scan-v4l", no_argument, 0, 's' },
        { "gray", no_argument, 0, 'g' },
        { 0 }
    };

    int index_option;
    unsigned long uval;
    char *end;
    while ((ch = getopt_long(argc, argv, "ghvD:i:d:p:r:w:m:M:Vs",
                             long_options, &index_option)) != -1) {
        switch (ch) {
        case 'h':
            usage();
            exit(0);
            break;
        case 'v':
            fprintf(stdout, "%s\n", PACKAGE_VERSION);
            exit(0);
            break;
        case 'D':
            debug++;
            break;
        case 'd':
            devno = atoi(optarg);
            break;
        case 'i':
            input = atoi(optarg);
            break;
        case 'r':
            errno = 0;
            uval = strtoul(optarg, &end, 10);
            if ((errno == ERANGE && (uval == ULONG_MAX || uval == 0)) ||
                (errno != 0 && uval == 0) || *end != '\0') {
                fprintf(stderr, "rate should be an unsigned integer, not `%s'\n",
                        optarg);
                usage();
                exit(1);
            }
            cfg.rate = uval;
            break;
        case 'w':
            errno = 0;
            uval = strtoul(optarg, &end, 10);
            if ((errno == ERANGE && (uval == ULONG_MAX || uval == 0)) ||
                (errno != 0 && uval == 0) || *end != '\0') {
                fprintf(stderr, "width should be an unsigned integer, not `%s'\n",
                        optarg);
                usage();
                exit(1);
            }
            cfg.width = uval;
            break;
        case 'm':
            cfg.min = hm_min_value(optarg);
            break;
        case 'M':
            cfg.max = hm_max_value(optarg);
            break;
        case 'V':
            cfg.values = true;
            break;
        case 's':
            print_debug_v4l_devices();
            exit(0);
            break;
        case 'g':
            cfg.gray = true;
            break;
        default:
            usage();
            exit(1);
        }
    }

    cfg.auto_min = (cfg.min == INT_MAX);
    cfg.auto_max = (cfg.max == INT_MIN);

    log_init(debug, __progname);

    snprintf(cfg.path, sizeof(cfg.path), "/dev/v4l-touch%d", devno);

    if (hm_v4l_init(&cfg, input) < 0)
        fatal("heatmap", "unable to init V4L");;

    /* Setup signals */
    struct sigaction actterm;
    sigemptyset(&actterm.sa_mask);
    actterm.sa_flags = 0;
    actterm.sa_handler = hm_terminate;
    if (sigaction(SIGTERM, &actterm, NULL) < 0)
        fatal("heatmap", "unable to register SIGTERM");
    struct sigaction actwinch;
    sigemptyset(&actwinch.sa_mask);
    actwinch.sa_flags = 0;
    actwinch.sa_handler = hm_resize;
    if (sigaction(SIGWINCH, &actwinch, NULL) < 0)
        fatal("heatmap", "unable to register SIGWINCH");
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    hm_display_init(&cfg);

    int err = 0;
    do {
        if (cfg.rate > 0) {
            const struct timespec ts = {
                .tv_sec = 1 / cfg.rate,
                .tv_nsec = ((cfg.rate > 1)?(1000 * 1000 * 1000 / cfg.rate):0)
            };
            nanosleep(&ts, NULL);
        }

        size_t len;
        len = hm_v4l_get_frame(&cfg);

        if (len == 0) {
            if (err++ > 5) {
                endwin();
                fatalx("heatmap", "unable to retrieve data");
            }
            continue;
        }
        err = 0;
        if (resize) {
            resize = false;
            endwin();
            refresh();
            clear();
        }

        for (size_t i = 0; i < len; i++) {
          int blob = hm_v4l_get_value(&cfg, i);
          if (cfg.auto_min && blob < cfg.min) cfg.min = blob;
          if (cfg.auto_max && blob > cfg.max) cfg.max = blob;
        }

        hm_display_data(&cfg, len);
    } while (!stop);

    endwin();

    hm_v4l_close(&cfg);

    return EXIT_SUCCESS;
}

