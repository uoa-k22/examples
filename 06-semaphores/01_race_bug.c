/*
 * 01_race_bug.c - two processes update a shared counter without coordination.
 *
 * Run:   ./01_race_bug               each process adds 1, 1000000 times
 *        ./01_race_bug 100           try a small count too
 *        The fix is in 01_race_fixed.c. To see what changed:
 *        diff 01_race_bug.c 01_race_fixed.c
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
 *        overlap, so the bug hides.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define NPROC 2

struct shared {
    /* volatile: make the compiler really load and store memory every time. */
    volatile long counter;
};

int main(int argc, char *argv[])
{
    long count = argc > 1 ? atol(argv[1]) : 1000000;

    struct shared *s = mmap(NULL, sizeof(*s), PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (s == MAP_FAILED)
        die("mmap");
    s->counter = 0;

    for (int p = 0; p < NPROC; p++) {
        pid_t pid = fork();
        if (pid < 0)
            die("fork");
        if (pid == 0) {
            for (long i = 0; i < count; i++)
                s->counter++;   /* BUG: a critical section, unprotected */
            _exit(0);
        }
    }
    while (wait(NULL) > 0)
        ;

    long expected = NPROC * count;
    printf("expected %ld, got %ld (%ld updates lost)\n",
           expected, s->counter, expected - s->counter);

    munmap(s, sizeof(*s));
    return 0;
}
