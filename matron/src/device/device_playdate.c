#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "device_playdate.h"
#include "events.h"

#define PLAYDATE_RETRIES 10

int dev_playdate_init(void *self) {
    struct dev_playdate *d = (struct dev_playdate *)self;
    struct dev_common *base = (struct dev_common *)self;

    d->fd = open(d->base.path, O_RDWR | O_NOCTTY | O_SYNC);
    if (d->fd < 0) {
        fprintf(stderr, "failed to open playdate device: %s\n", d->base.path);
        return -1;
    }

    tcgetattr(d->fd, &d->oldtio);
    d->newtio.c_cflag = PLAYDATE_BAUDRATE | CS8 | CLOCAL | CREAD;
    d->newtio.c_iflag &= ~(IXON | IXOFF | IXANY);
    d->newtio.c_oflag = 0;
    d->newtio.c_lflag = 0;
    d->newtio.c_cc[VMIN] = 0;
    d->newtio.c_cc[VTIME] = 5;
    tcflush(d->fd, TCIFLUSH);
    tcsetattr(d->fd, TCSANOW, &d->newtio);

    char s[512];
    write(d->fd, "echo off\n", 9);
    read(d->fd, s, 511); // clear buffer
    write(d->fd, "version\n", 8);
    usleep(1000);
    int retry = 0;
    while (retry < PLAYDATE_RETRIES) {
        retry++;
        memset(s, 0, sizeof(s));
        read(d->fd, s, 511);
        // fprintf(stderr, "playdate init> %i %s", sizeof(s), s);
        if (strstr(s, "pdxversion=") != NULL)
            break;
    }
    if (retry >= PLAYDATE_RETRIES) {
        fprintf(stderr, ">> playdate failed version\n");
        return -1;
    }

    base->start = &dev_playdate_start;
    base->deinit = &dev_playdate_deinit;

    return 0;
}

#define PLAYDATE_BUTTON_A 0
#define PLAYDATE_BUTTON_B 1
#define PLAYDATE_BUTTON_U 2
#define PLAYDATE_BUTTON_D 3
#define PLAYDATE_BUTTON_L 4
#define PLAYDATE_BUTTON_R 5
#define PLAYDATE_BUTTON_M 6

static void handle_event(void *dev, uint32_t id) {
    union event_data *ev = event_data_new(EVENT_PLAYDATE_EVENT);
    ev->playdate_event.dev = dev;
    ev->playdate_event.id = id;
    event_post(ev);
}

void *dev_playdate_start(void *self) {
    struct dev_playdate *di = (struct dev_playdate *)self;
    struct dev_common *base = (struct dev_common *)self;

    uint8_t len;

    while (1) {
        len = read(di->fd, di->line, 255);
        if (len > 0) {
            di->line[len] = 0; // add null to end of string
            if (len > 1) {
                // fprintf(stderr, "playdate> %s", di->line);
                handle_event(self, base->id);
            }
            len = 0;
        }
        usleep(1000); // 1ms
    }
    return NULL;
}

void dev_playdate_deinit(void *self) {
    struct dev_playdate *di = (struct dev_playdate *)self;
    write(di->fd, "controller stop\n", 16);
    tcsetattr(di->fd, TCSANOW, &di->oldtio);
}

void dev_playdate_run(void *self, const char *path) {
    struct dev_playdate *d = (struct dev_playdate *)self;
    write(d->fd, "run ", 4);
    write(d->fd, path, strlen(path));
    write(d->fd, "\n", 1);
}

void dev_playdate_send(void *self, const char *line) {
    struct dev_playdate *d = (struct dev_playdate *)self;
    write(d->fd, "msg ", 4);
    write(d->fd, line, strlen(line));
    write(d->fd, "\n", 1);
}

void dev_playdate_controller_start(void *self) {
    struct dev_playdate *d = (struct dev_playdate *)self;
    write(d->fd, "controller start\n", 17);
}

void dev_playdate_controller_stop(void *self) {
    struct dev_playdate *d = (struct dev_playdate *)self;
    write(d->fd, "controller stop\n", 16);
}
