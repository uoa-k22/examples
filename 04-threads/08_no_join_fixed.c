/*
 * 08_no_join_fixed.c - main() waits for its threads before returning.
 *
 * Run:   ./08_no_join_fixed
 *
 * Look:  the bug was in 08_no_join_bug.c: main() returned at once, and the
 *        process ended with all its threads. Now main() joins every thread
 *        first, so all 4 print "done" before main() returns.
 */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "errexit.h"

#define NTHREADS 4

static void *worker(void *arg)
{
    usleep(100000);                     /* some work */
    printf("thread %ld: done\n", (long) arg);
    return NULL;
}

int main(void)
{
    pthread_t t[NTHREADS];

    for (long i = 0; i < NTHREADS; i++)
        if ((errno = pthread_create(&t[i], NULL, worker, (void *) i)) != 0)
            die("pthread_create");

    /* FIX: wait for every thread before the process ends */
    for (int i = 0; i < NTHREADS; i++)
        if ((errno = pthread_join(t[i], NULL)) != 0)
            die("pthread_join");
    printf("main: returning\n");
    return 0;
}
