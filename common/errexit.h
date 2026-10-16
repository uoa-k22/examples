/*
 * errexit.h - one-line error handling for the examples.
 *
 * Every system call in these examples is checked. To keep the checks from
 * hiding the idea being shown, a failed call is handled with
 *
 *     if (fork() < 0)
 *         die("fork");
 *
 * die() prints the message followed by the reason (from errno), e.g.
 * "fork: Resource temporarily unavailable", and exits with status 1.
 */
#ifndef ERREXIT_H
#define ERREXIT_H

#include <stdio.h>
#include <stdlib.h>

#define die(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

#endif
