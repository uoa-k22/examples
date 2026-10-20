/*
 * 05_shared_arg_fixed.c - every thread gets a pointer to its own variable.
 *
 * Run:   ./05_shared_arg_fixed
 *
 * Look:  the bug was in 05_shared_arg_bug.c: all threads got a pointer to
 *        one variable id, which main() kept changing. Now id is an array,
 *        and each thread gets a pointer to its own element, which nobody
 *        changes. The threads print 0 1 2 3, in some order.
 *
 *        The array lives until main() has joined every thread. A pointer
 *        given to a thread must stay valid for as long as the thread uses
 *        it.
 */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "errexit.h"

#define NTHREADS 4

static void *worker(void *arg)
{
    usleep(100000);                     /* start a little slowly */
    printf("thread: my number is %d\n", *(int *) arg);
    return NULL;
}

int main(void)
{
    pthread_t t[NTHREADS];
    int id[NTHREADS];           /* FIX: one variable for each thread */

    for (int i = 0; i < NTHREADS; i++) {
        id[i] = i;
        if ((errno = pthread_create(&t[i], NULL, worker, &id[i])) != 0)
            die("pthread_create");
    }

    for (int i = 0; i < NTHREADS; i++)
        if ((errno = pthread_join(t[i], NULL)) != 0)
            die("pthread_join");
    return 0;
}
