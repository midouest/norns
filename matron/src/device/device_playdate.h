#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <termios.h>

#include "device_common.h"
#include <libevdev/libevdev.h>

#define PLAYDATE_BAUDRATE B115200

struct dev_playdate {
    struct dev_common base;
    int fd;
    struct termios oldtio, newtio;
    char line[255];
};

extern int dev_playdate_init(void *self);
extern void *dev_playdate_start(void *self);
extern void dev_playdate_deinit(void *self);

extern void dev_playdate_run(void *self, const char *path);
extern void dev_playdate_send(void *self, const char *line);
extern void dev_playdate_controller_start(void *self);
extern void dev_playdate_controller_stop(void *self);
