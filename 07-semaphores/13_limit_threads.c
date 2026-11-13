/*
 * 13_limit_threads.c - a counting semaphore lets at most N threads in.
 *
 * Run:   ./13_limit_threads
 *
 * Look:  THREADS threads each want to use a resource that only SLOTS can use
 *        at the same time, such as a few database connections. The semaphore
 *        "slots" starts at SLOTS, not at 1. Each sem_wait() takes a slot,
 *        and when none is left the next thread waits until one is posted
 *        back. So "[n inside]" goes up to SLOTS but never above it, and the
 *        threads that arrive later wait their turn.
 *
 *        A second semaphore, "mutex", starts at 1 and protects the count
 *        "inside", which is only there to be printed.
 */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include "errexit.h"

#define THREADS 8
#define SLOTS   3

static sem_t slots;         /* free slots */
static sem_t mutex;         /* protects inside */
static int   inside;        /* threads using a slot, only for printing */

static void *worker(void *arg)
{
    long id = (long) arg;

    if (sem_wait(&slots) < 0)
        die("sem_wait");
    if (sem_wait(&mutex) < 0)
        die("sem_wait");
    inside++;
    printf("thread %ld: in    [%d inside]\n", id, inside);
    if (sem_post(&mutex) < 0)
        die("sem_post");

    usleep(200000);                         /* use the resource */

    if (sem_wait(&mutex) < 0)
        die("sem_wait");
    inside--;
    printf("thread %ld: out   [%d inside]\n", id, inside);
    if (sem_post(&mutex) < 0)
        die("sem_post");
    if (sem_post(&slots) < 0)
        die("sem_post");
    return NULL;
}

int main(void)
{
    setvbuf(stdout, NULL, _IOLBF, 0);

    if (sem_init(&slots, 0, SLOTS) < 0 || sem_init(&mutex, 0, 1) < 0)
        die("sem_init");

    pthread_t t[THREADS];
    for (long i = 0; i < THREADS; i++)
        if ((errno = pthread_create(&t[i], NULL, worker, (void *) i)) != 0)
            die("pthread_create");
    for (int i = 0; i < THREADS; i++)
        if ((errno = pthread_join(t[i], NULL)) != 0)
            die("pthread_join");

    sem_destroy(&slots);
    sem_destroy(&mutex);
    return 0;
}
