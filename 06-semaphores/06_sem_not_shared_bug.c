/*
 * 06_sem_not_shared_bug.c - a semaphore that is not in shared memory.
 *
 * Run:   ./06_sem_not_shared_bug
 *        The fix is in 06_sem_not_shared_fixed.c. To see what changed:
 *        diff 06_sem_not_shared_bug.c 06_sem_not_shared_fixed.c
 *
 * Look:  the counter is in shared memory, but the semaphore that should
 *        protect it is an ordinary global variable. fork() gives each child
 *        its own copy of it, each starting at 1. Each child locks and
 *        unlocks only its own copy, so neither ever waits for the other, and
 *        updates are lost just as in 01_race.c.
 *
 *        Nothing reports an error. sem_init() with pshared = 1 cannot check
 *        where the semaphore lives, and every sem_wait() and sem_post()
 *        succeeds. The only symptom is a wrong result, now and then.
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
    long counter;
};

/* BUG: a global variable: fork() gives each process its own copy. */
static sem_t private_lock;

int main(int argc, char *argv[])
{
    long count = argc > 1 ? atol(argv[1]) : 1000000;

    struct shared *s = mmap(NULL, sizeof(*s), PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (s == MAP_FAILED)
        die("mmap");
    s->counter = 0;

    sem_t *lock = &private_lock;
    if (sem_init(lock, 1, 1) < 0)
        die("sem_init");
    printf("semaphore at %p, in a global variable\n", (void *) lock);
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
            return 0;
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
