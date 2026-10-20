/*
 * 05_shared_arg_bug.c - every thread gets a pointer to the same variable.
 *
 * Run:   ./05_shared_arg_bug
 *        The fix is in 05_shared_arg_fixed.c. To see what changed:
 *        diff 05_shared_arg_bug.c 05_shared_arg_fixed.c
 *
 * Look:  main() puts each thread's number in the variable id and passes
 *        &id. But there is only one id, and main() changes it for the next
 *        thread before the last one has read it. Every thread prints the
 *        number of the last one: 3 3 3 3 instead of 0 1 2 3.
 *
 *        The usleep() in the thread makes the bad timing happen every time.
 *        Without it, threads would read id sooner or later, and the numbers
 *        would come out wrong only now and then.
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
    int id;                     /* BUG: one variable shared by all threads */

    for (int i = 0; i < NTHREADS; i++) {
        id = i;
        if ((errno = pthread_create(&t[i], NULL, worker, &id)) != 0)
            die("pthread_create");
    }

    for (int i = 0; i < NTHREADS; i++)
        if ((errno = pthread_join(t[i], NULL)) != 0)
            die("pthread_join");
    return 0;
}
