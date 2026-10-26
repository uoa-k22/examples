/*
 * 03_shared_globals.c - threads share one address space.
 *
 * Run:   ./03_shared_globals
 *
 * Look:  the thread changes a global variable and, through the pointer it
 *        gets as its argument, a local variable of main(). After joining,
 *        main() sees both changes. Compare 01-fork/02_address_space.c: there
 *        the child changes the same variables, and the parent does not see
 *        it, because each process has its own copy.
 *
 *        All threads of a process share its global variables, its heap and
 *        its open files. Each thread has only its own stack and registers.
 */
#include <pthread.h>
#include <stdio.h>
#include "errexit.h"

int global = 100;

static void *change(void *arg)
{
    int *local_p = arg;

    global++;
    (*local_p)++;
    printf("thread: global = %d (at %p), local = %d (at %p)\n",
           global, (void *) &global, *local_p, (void *) local_p);
    return NULL;
}

int main(void)
{
    int local = 200;
    pthread_t t;

    printf("before: global = %d (at %p), local = %d (at %p)\n",
           global, (void *) &global, local, (void *) &local);

    if ((errno = pthread_create(&t, NULL, change, &local)) != 0)
        die("pthread_create");
    if ((errno = pthread_join(t, NULL)) != 0)
        die("pthread_join");

    printf("main:   global = %d (at %p), local = %d (at %p)\n",
           global, (void *) &global, local, (void *) &local);
    return 0;
}
