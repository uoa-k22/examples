/*
 * 01_create_join.c - start a thread and wait for it to finish.
 *
 * Run:   ./01_create_join
 *
 * Look:  pthread_create() starts a new thread that runs hello() while main()
 *        goes on; the two run at the same time, so their lines can come in
 *        either order. pthread_join() waits for the thread to end, as wait()
 *        waits for a child process. Both lines show the same pid: a thread
 *        is not a new process, but a second flow of control inside the same
 *        one.
 */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "errexit.h"

static void *hello(void *arg)
{
    (void) arg;                         /* not used */
    printf("thread: hello (pid %d)\n", (int) getpid());
    return NULL;
}

int main(void)
{
    pthread_t t;

    if ((errno = pthread_create(&t, NULL, hello, NULL)) != 0)
        die("pthread_create");
    printf("main:   started a thread (pid %d)\n", (int) getpid());

    if ((errno = pthread_join(t, NULL)) != 0)
        die("pthread_join");
    printf("main:   the thread has finished\n");
    return 0;
}
