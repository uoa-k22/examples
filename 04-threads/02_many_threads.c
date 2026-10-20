/*
 * 02_many_threads.c - start several threads, each with its own argument.
 *
 * Run:   ./02_many_threads
 *        ./02_many_threads 8         try another number of threads
 *
 * Look:  the last argument of pthread_create() is passed to the thread's
 *        function; here it is the thread's number, put into the pointer
 *        itself. The threads print in a different order on every run.
 *        They all show the same pid, but each has its own tid: the kernel
 *        schedules every thread on its own.
 *
 *        main() keeps each pthread_t in an array, so that it can join them
 *        all after starting them all.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "errexit.h"

static void *worker(void *arg)
{
    long id = (long) arg;

    printf("thread %ld: pid %d, tid %ld\n",
           id, (int) getpid(), (long) syscall(SYS_gettid));
    return NULL;
}

int main(int argc, char *argv[])
{
    long n = argc > 1 ? atol(argv[1]) : 4;
    pthread_t *t = malloc(n * sizeof(*t));
    if (t == NULL)
        die("malloc");

    for (long i = 0; i < n; i++)
        if ((errno = pthread_create(&t[i], NULL, worker, (void *) i)) != 0)
            die("pthread_create");

    for (long i = 0; i < n; i++)
        if ((errno = pthread_join(t[i], NULL)) != 0)
            die("pthread_join");
    printf("main: all %ld threads have finished\n", n);

    free(t);
    return 0;
}
