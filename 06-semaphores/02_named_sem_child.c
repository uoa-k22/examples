/*
 * 02_named_sem_child.c - started by 02_named_sem_parent; not run directly.
 *
 * Opens the existing semaphore by name (no O_CREAT) and enters a critical
 * section ROUNDS times. Each time it prints when it enters and when it
 * leaves, so you can check that the sections never overlap.
 */
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include "errexit.h"
#include "named_sem.h"

int main(int argc, char *argv[])
{
    const char *id = argc > 1 ? argv[1] : "?";

    sem_t *sem = sem_open(SEM_NAME, 0);
    if (sem == SEM_FAILED)
        die("sem_open " SEM_NAME);

    for (int r = 0; r < ROUNDS; r++) {
        if (sem_wait(sem) < 0)
            die("sem_wait");
        printf("child %s (pid %d): enter\n", id, (int) getpid());
        fflush(stdout);
        usleep(200000);                     /* "work" in the critical section */
        printf("child %s (pid %d): leave\n", id, (int) getpid());
        fflush(stdout);
        if (sem_post(sem) < 0)
            die("sem_post");
        usleep(100000);                     /* work outside it */
    }

    sem_close(sem);
    return 0;
}
