/*
 * 06_sem_not_shared_fixed.c - the fix for 06_sem_not_shared_bug.c.
 *
 * Run:   ./06_sem_not_shared_fixed
 *        diff 06_sem_not_shared_bug.c 06_sem_not_shared_fixed.c
 *
 * Look:  the semaphore moves into the shared memory, next to the counter.
 *        Now both children use the same semaphore, only one at a time can
 *        update the counter, and no update is lost. This is the program
 *        01_race_fixed.c.
 *
 * Rule:  a semaphore shared by processes must itself be in shared memory
 *        (mmap MAP_SHARED, shm_open, shmget), or be a named semaphore.
 */
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define NPROC 2

struct shared {
    sem_t lock;         /* FIX: the semaphore is in shared memory too */
    long  counter;
};

int main(int argc, char *argv[])
{
    long count = argc > 1 ? atol(argv[1]) : 1000000;

    struct shared *s = mmap(NULL, sizeof(*s), PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (s == MAP_FAILED)
        die("mmap");
    s->counter = 0;

    sem_t *lock = &s->lock;
    if (sem_init(lock, 1, 1) < 0)
        die("sem_init");
    printf("semaphore at %p, in shared memory\n", (void *) lock);
    fflush(stdout);

    for (int p = 0; p < NPROC; p++) {
        pid_t pid = fork();
        if (pid < 0)
            die("fork");
        if (pid == 0) {
            for (long i = 0; i < count; i++) {
                if (sem_wait(lock) < 0)
                    die("sem_wait");
                s->counter++;
                if (sem_post(lock) < 0)
                    die("sem_post");
            }
            _exit(0);
        }
    }
    while (wait(NULL) > 0)
        ;

    long expected = NPROC * count;
    printf("expected %ld, got %ld (%ld updates lost)\n",
           expected, s->counter, expected - s->counter);
    sem_destroy(lock);
    return 0;
}
