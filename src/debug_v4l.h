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

#ifndef _DEBUG_V4L_H
#define _DEBUG_V4L_H

#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <linux/videodev2.h>
#include <linux/limits.h>

struct hm_cfg {
  int fd;       /* open device file descriptor */
  struct v4l2_format fmt;    /* v4l2 device format */
  void *buffer; /* Buffer used to transfer data */
  struct v4l2_buffer bufferinfo; /* v4l2 buffer info */
  struct v4l2_capability cap;   /* v4l2 capability */
  struct v4l2_input in;   /* v4l2 input */
  char path[PATH_MAX];		/* Path to data file */
  unsigned int rate;	/* Refresh rate */
  int width;	/* Touchscreen width */
  int height;	/* Touchscreen height */
  uint32_t pixfmt; /* Pixel format */
  int min;		/* Minimal pressure value */
  int max;		/* Maximumal pressure value */
  bool auto_min;
  bool auto_max;
  bool values;		/* Display pressure values */
  bool gray;		/* Use grayscale */
};

/* debug_v4l */
void print_debug_v4l_devices();
void hm_v4l_init(struct hm_cfg *cfg, int input);
int hm_v4l_get_frame(struct hm_cfg *cfg);
void hm_v4l_close(struct hm_cfg *cfg);
int hm_v4l_get_value(struct hm_cfg *cfg, size_t index);
#endif
