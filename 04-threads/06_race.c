/*
 * 06_race.c - two threads update a shared counter without coordination.
 *
 * Run:   ./06_race               each thread adds 1, 1000000 times
 *        ./06_race 100           try a small count too
 *
 * Look:  the same race as in 07-semaphores/01_race.c, with threads instead
 *        of processes. The expected total is 2 * count, but the result is
 *        usually less, and different on every run, because counter++ is
 *        load, add, store, and the threads interleave. Threads need no
 *        mmap() to share the counter: a global variable is already shared.
 *        07_mutex.c fixes it.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "errexit.h"

#define NTHREADS 2

/* volatile: make the compiler really load and store memory every time. */
volatile long counter = 0;
long count;

static void *add(void *arg)
{
    (void) arg;
    for (long i = 0; i < count; i++)
        counter++;
    return NULL;
}

int main(int argc, char *argv[])
{
    count = argc > 1 ? atol(argv[1]) : 1000000;
    pthread_t t[NTHREADS];

    for (int i = 0; i < NTHREADS; i++)
        if ((errno = pthread_create(&t[i], NULL, add, NULL)) != 0)
            die("pthread_create");
    for (int i = 0; i < NTHREADS; i++)
        if ((errno = pthread_join(t[i], NULL)) != 0)
            die("pthread_join");

    long expected = NTHREADS * count;
    printf("expected %ld, got %ld (%ld updates lost)\n",
           expected, counter, expected - counter);
    return 0;
}
