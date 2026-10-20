/*
 * 09_join_in_loop_bug.c - joining each thread right after starting it.
 *
 * Run:   ./09_join_in_loop_bug
 *        The fix is in 09_join_in_loop_fixed.c. To see what changed:
 *        diff 09_join_in_loop_bug.c 09_join_in_loop_fixed.c
 *
 * Look:  each of the 4 threads works for 1 second. They could all work at
 *        the same time and finish in 1 second, but this takes 4. main()
 *        joins each thread before it starts the next one, so only one
 *        thread runs at a time. The output is correct, so this mistake is
 *        easy to miss; the program is just slow.
 */
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "errexit.h"

#define NTHREADS 4

static void *worker(void *arg)
{
    sleep(1);                           /* 1 second of "work" */
    printf("thread %ld: done\n", (long) arg);
    return NULL;
}

int main(void)
{
    pthread_t t[NTHREADS];
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (long i = 0; i < NTHREADS; i++) {
        if ((errno = pthread_create(&t[i], NULL, worker, (void *) i)) != 0)
            die("pthread_create");
        /* BUG: wait for this thread before starting the next one */
        if ((errno = pthread_join(t[i], NULL)) != 0)
            die("pthread_join");
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("main: all done after %.1f seconds\n",
           (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9);
    return 0;
}
