/*
 * 07_mutex.c - the race from 06_race.c, fixed with a mutex.
 *
 * Run:   ./07_mutex
 *        ./07_mutex 100
 *
 * Look:  a mutex is a lock for the threads of one process. Only one thread
 *        at a time can be between pthread_mutex_lock() and
 *        pthread_mutex_unlock(); another thread that calls lock waits until
 *        it is unlocked. So no update is lost and the total is always exact.
 *        The price is speed: compare  time ./06_race  with  time ./07_mutex
 *
 *        It does the job of the semaphore that starts at 1 in
 *        07-semaphores/02_unnamed_sem.c. Unlike a semaphore, a mutex has an
 *        owner: only the thread that locked it may unlock it.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "errexit.h"

#define NTHREADS 2

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
long counter = 0;
long count;

static void *add(void *arg)
{
    (void) arg;
    for (long i = 0; i < count; i++) {
        if ((errno = pthread_mutex_lock(&lock)) != 0)
            die("pthread_mutex_lock");
        counter++;                      /* critical section */
        if ((errno = pthread_mutex_unlock(&lock)) != 0)
            die("pthread_mutex_unlock");
    }
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
