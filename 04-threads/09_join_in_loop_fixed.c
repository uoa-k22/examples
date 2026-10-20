/*
 * 09_join_in_loop_fixed.c - start every thread first, then join them.
 *
 * Run:   ./09_join_in_loop_fixed
 *
 * Look:  the bug was in 09_join_in_loop_bug.c: main() joined each thread
 *        right after starting it, so the threads ran one at a time and the
 *        program took 4 seconds. Now main() starts all 4 threads in one
 *        loop and joins them in a second loop. The threads work at the same
 *        time, and the program takes 1 second.
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

    for (long i = 0; i < NTHREADS; i++)
        if ((errno = pthread_create(&t[i], NULL, worker, (void *) i)) != 0)
            die("pthread_create");
    /* FIX: join only after every thread has started */
    for (int i = 0; i < NTHREADS; i++)
        if ((errno = pthread_join(t[i], NULL)) != 0)
            die("pthread_join");

    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("main: all done after %.1f seconds\n",
           (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9);
    return 0;
}
