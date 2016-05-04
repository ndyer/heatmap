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

#include "debug_v4l.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stropts.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <errno.h>

#ifndef V4L2_PIX_FMT_YS16
#define V4L2_PIX_FMT_YS16    v4l2_fourcc('Y', 'S', '1', '6') /* signed 16-bit Greyscale */
#endif

#define DEBUG_V4L_ROOT_DIR		"/dev"
#define V4L_DEV_PREFIX	     	"v4l-touch"

static int
debug_v4l_open_device(char *dev)
{
  int fd = 0;

  printf("Opening device: %s\n", dev);

  if((fd = open(dev, O_RDWR)) < 0){
    fprintf(stderr, "Error: Failed to open device %s\n", dev);
    return 0;
  }

  return fd;
}

static void
debug_v4l_close_device(struct hm_cfg *cfg)
{
  if (cfg->fd) {
    printf("Closing device");

    close(cfg->fd);
    cfg->fd = 0;
  }
  return;
}

static int
debug_v4l_set_dev_input(int fd, int index)
{
  if (ioctl(fd, VIDIOC_S_INPUT, &index) == -1) {
    fprintf(stderr, "Error: Failed to set device input %d\n", index);
    return -1;
  }
  return 0;
}

static void
debug_v4l_get_cap(int fd, struct v4l2_capability *cap)
{
  struct v4l2_input in;

  int ret = ioctl(fd, VIDIOC_QUERYCAP, cap);
  if (ret == -1)
    return;

  printf("  Driver:\t%s\n", cap->driver);
  printf("  Card:\t\t%s\n", cap->card);
  printf("  Bus Info:\t%s\n", cap->bus_info);

  in.index = 0;

  while (ioctl(fd, VIDIOC_ENUMINPUT, &in) == 0) {
    /* Print input instance */
    printf("  Input:%d\t%s\n", in.index, in.name);
    in.index++;
  }

  printf("\n");
}

static void
debug_v4l_print_dev_info(char *path)
{
  int fd;
  struct v4l2_capability cap;

  fd = debug_v4l_open_device(path);
  if (!fd)
    return;

  debug_v4l_get_cap(fd, &cap);

  close(fd);
}

static int
debug_v4l_get_dev_fmt(struct hm_cfg *cfg)
{
  cfg->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  int ret = ioctl(cfg->fd, VIDIOC_G_FMT, &cfg->fmt);
  if (ret == -1) {
    fprintf(stderr, "Error: Failed to get v4l2 device format\n");
    return -1;
  }

  return 0;
}

void
print_debug_v4l_devices()
{
  char path[PATH_MAX];
  DIR *dp;
  struct dirent *ep;

  dp = opendir(DEBUG_V4L_ROOT_DIR);
  if (dp) {
    while ((ep = readdir(dp))) {
      if (!strncmp(ep->d_name, V4L_DEV_PREFIX, strlen(V4L_DEV_PREFIX))) {

        /* configure base dir */
        snprintf(path, PATH_MAX, "%s/%s", DEBUG_V4L_ROOT_DIR, ep->d_name);
        debug_v4l_print_dev_info(path);
      }
    }
    closedir(dp);
  } else {
    fprintf(stderr, "Couldn't open the debug_v4l directory.\n");
  }
}

int
hm_v4l_init(struct hm_cfg *cfg, int input)
{
  int ret;

  /* open device */
  cfg->fd = debug_v4l_open_device(cfg->path);
  if (!cfg->fd) {
    fprintf(stderr, "Error: Failed to open device\n");
    return -1;
  }

  debug_v4l_get_cap(cfg->fd, &cfg->cap);

  /* set input */
  ret = debug_v4l_set_dev_input(cfg->fd, input);
  if (ret) {
    fprintf(stderr, "input\n");
    return -1;
  }

  cfg->in.index = input;
  ret = ioctl(cfg->fd, VIDIOC_ENUMINPUT, &cfg->in);
  if (ret) {
    fprintf(stderr, "VIDIOC_ENUMINPUT failed");
    return -1;
  }

  /* request buffers */
  struct v4l2_requestbuffers bufrequest = { 0 };
  bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  bufrequest.memory = V4L2_MEMORY_MMAP;
  bufrequest.count = 1;

  if (ioctl(cfg->fd, VIDIOC_REQBUFS, &bufrequest) < 0) {
    fprintf(stderr, "Error from VIDIOC_REQBUFS: %s\n", strerror(errno));
    return -1;
  }

  memset(&cfg->bufferinfo, 0, sizeof(cfg->bufferinfo));

  cfg->bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  cfg->bufferinfo.memory = V4L2_MEMORY_MMAP;
  cfg->bufferinfo.index = 0;

  if(ioctl(cfg->fd, VIDIOC_QUERYBUF, &cfg->bufferinfo) == -1){
    fprintf(stderr, "Query buf\n");
    return -1;
  }

  /* alloc buffers */
  cfg->buffer = mmap(
      NULL,
      cfg->bufferinfo.length,
      PROT_READ | PROT_WRITE,
      MAP_SHARED,
      cfg->fd,
      cfg->bufferinfo.m.offset
      );

  if(cfg->buffer == MAP_FAILED){
    fprintf(stderr, "Error: Memory map failed\n");
    return -1;
  }

  memset(cfg->buffer, 0, cfg->bufferinfo.length);

  /* Query format */
  ret = debug_v4l_get_dev_fmt(cfg);
  if (ret) {
    fprintf(stderr, "Error: Query format failed\n");
    return -1;
  }

  cfg->pixfmt = cfg->fmt.fmt.pix.pixelformat;
  cfg->width = cfg->fmt.fmt.pix.width;
  cfg->height = cfg->fmt.fmt.pix.height;
  printf("cfg->width %d  cfg->height %d\n",  cfg->width, cfg->height);

  printf("cfg->pixfmt %c%c%c%c\n", cfg->pixfmt & 0xff,
         (cfg->pixfmt >> 8) & 0xff, (cfg->pixfmt >> 16) & 0xff, cfg->pixfmt >> 24);

  /* activate streaming */
  int type = cfg->bufferinfo.type;
  if(ioctl(cfg->fd, VIDIOC_STREAMON, &type) < 0){
    fprintf(stderr, "Error: Failed to activate streaming\n");
    return -1;
  }

  return 0;
}

int
hm_v4l_get_frame(struct hm_cfg *cfg)
{
  /* Put the buffer in the incoming queue */
  if(ioctl(cfg->fd, VIDIOC_QBUF, &cfg->bufferinfo) < 0){
    fprintf(stderr, "Error: Failed to queue buffer\n");
    return 0;
  }

  // The buffer's waiting in the outgoing queue.
  if(ioctl(cfg->fd, VIDIOC_DQBUF, &cfg->bufferinfo) < 0){
    fprintf(stderr, "Error: Failed to dequeue buffer\n");
    return 0;
  }

  return (cfg->bufferinfo.length/ sizeof(uint16_t));
}

void
hm_v4l_close(struct hm_cfg *cfg)
{
  /* Deactivate streaming */
  int type = cfg->bufferinfo.type;
  if(ioctl(cfg->fd, VIDIOC_STREAMOFF, &type) < 0){
    fprintf(stderr, "Error: Failed to deactivate streaming\n");
    return;
  }

  munmap(cfg->buffer, cfg->bufferinfo.length);

  debug_v4l_close_device(cfg);
}

int
hm_v4l_get_value(struct hm_cfg *cfg, size_t index)
{
    int val;
    uint16_t *uiPtr;
    int16_t *iPtr;

    switch (cfg->fmt.fmt.pix.pixelformat) {
        case V4L2_PIX_FMT_Y16:
            uiPtr = cfg->buffer;
            val = uiPtr[index];
            break;

        case V4L2_PIX_FMT_YS16:
            iPtr = cfg->buffer;
            val = iPtr[index];
            break;

        default:
            val = 0;
            break;
    }

    return val;
}
