/*
 * 08_no_join_bug.c - main() returns without waiting for its threads.
 *
 * Run:   ./08_no_join_bug
 *        The fix is in 08_no_join_fixed.c. To see what changed:
 *        diff 08_no_join_bug.c 08_no_join_fixed.c
 *
 * Look:  main() starts 4 threads and returns. Returning from main() calls
 *        exit(), which ends the whole process, with all its threads. The
 *        threads are still in usleep(), so none of them prints "done".
 *
 *        A process ends when any of its threads calls exit() or main()
 *        returns, not when its last thread ends. Waiting with sleep()
 *        instead of joining only hides the bug: a thread that needs longer
 *        is still killed.
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

    /* BUG: no pthread_join(), so the threads die with the process */
    printf("main: returning\n");
    return 0;
}
