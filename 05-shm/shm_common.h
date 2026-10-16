/*
 * shm_common.h - what the POSIX shared memory writer and reader agree on:
 * the object's name and the layout of the data stored in it.
 */
#ifndef SHM_COMMON_H
#define SHM_COMMON_H

#include <sys/types.h>

/* POSIX shm names start with '/'. On Linux the object appears as
   /dev/shm/k22_shm_demo while it exists. */
#define SHM_NAME "/k22_shm_demo"

struct message {
    pid_t writer;       /* who wrote the message */
    char  text[256];
};

#endif
