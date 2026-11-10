/*
 * sysv_common.h - what the System V writer and reader agree on: the path
 * and project id that ftok() turns into a key, and the layout of the data
 * stored in the segment.
 */
#ifndef SYSV_COMMON_H
#define SYSV_COMMON_H

#include <sys/types.h>

/* ftok() needs a file that exists; this header is one. Both programs run
   from this directory, so they name the same file. */
#define KEY_PATH "sysv_common.h"
#define KEY_ID   'k'

struct message {
    pid_t writer;       /* who wrote the message */
    char text[256];
};

#endif
