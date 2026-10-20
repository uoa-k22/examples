/*
 * 06_race.c - two threads update a shared counter without coordination.
 *
 * Run:   ./06_race               each thread adds 1, 1000000 times
 *        ./06_race 100           try a small count too
 *
 * Look:  the expected total is 2 * count, but the result is usually less,
 *        and different on every run. counter++ is not one step. It is
 *        load, add, store, and the two threads interleave:
 *
 *            A: load 41            B: load 41
 *            A: store 42           B: store 42     <- one increment lost
 *
 *        This is a race condition, and the code that updates the counter is
 *        a critical section. With a small count the threads rarely
 *        overlap, so the bug hides. 07_mutex.c fixes it.
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
