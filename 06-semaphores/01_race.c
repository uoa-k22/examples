/*
 * 01_race.c - two processes update a shared counter without coordination.
 *
 * Run:   ./01_race               each process adds 1, 1000000 times
 *        ./01_race 100           try a small count too
 *
 * Look:  the expected total is 2 * count, but the result is usually less,
 *        and different on every run. counter++ is not one step. It is
 *        load, add, store, and the two processes interleave:
 *
 *            A: load 41            B: load 41
 *            A: store 42           B: store 42     <- one increment lost
 *
 *        This is a race condition, and the code that updates the counter is
 *        a critical section. With a small count the processes rarely
 *        overlap, so the bug hides. 02_unnamed_sem.c fixes it.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define NPROC 2

int main(int argc, char *argv[])
{
    long count = argc > 1 ? atol(argv[1]) : 1000000;

    /* volatile: make the compiler really load and store memory every time. */
    volatile long *counter = mmap(NULL, sizeof(long), PROT_READ | PROT_WRITE,
                                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (counter == MAP_FAILED)
        die("mmap");
    *counter = 0;

    for (int p = 0; p < NPROC; p++) {
        pid_t pid = fork();
        if (pid < 0)
            die("fork");
        if (pid == 0) {
            for (long i = 0; i < count; i++)
                (*counter)++;
            _exit(0);
        }
    }
    while (wait(NULL) > 0)
        ;

    long expected = NPROC * count;
    printf("expected %ld, got %ld (%ld updates lost)\n",
           expected, *counter, expected - *counter);
    return 0;
}
