/*
 * 07_sem_not_shared.c - a semaphore that is not in shared memory.
 *
 * Run:   ./07_sem_not_shared
 *
 * Look:  as written, this is 02_unnamed_sem.c. The counter and the
 *        semaphore are both in shared memory, and no update is lost.
 *
 * Try:   uncomment the line marked below, run make and run again. The
 *        counter is still shared, but the semaphore is now an ordinary
 *        global variable. fork() gives each child its own copy of it, each
 *        starting at 1. Each child locks and unlocks only its own copy, so
 *        neither ever waits for the other, and updates are lost just as in
 *        01_race.c.
 *
 *        Nothing reports an error. sem_init() with pshared = 1 cannot check
 *        where the semaphore lives, and every sem_wait() and sem_post()
 *        succeeds. The only symptom is a wrong result, now and then.
 *
 * Rule:  a semaphore shared by processes must itself be in shared memory
 *        (mmap MAP_SHARED, shm_open, shmget), or be a named semaphore.
 */
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define NPROC 2

struct shared {
    sem_t lock;
    long  counter;
};

static sem_t private_lock;      /* a global: each process gets its own copy */

int main(int argc, char *argv[])
{
    long count = argc > 1 ? atol(argv[1]) : 1000000;

    struct shared *s = mmap(NULL, sizeof(*s), PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (s == MAP_FAILED)
        die("mmap");
    s->counter = 0;

    int lock_in_shared_memory = 1;
    // lock_in_shared_memory = 0;  /* <-- uncomment this line to see the bug */

    sem_t *lock = lock_in_shared_memory ? &s->lock : &private_lock;
    if (sem_init(lock, 1, 1) < 0)
        die("sem_init");
    printf("semaphore at %p, %s\n", (void *) lock,
           lock_in_shared_memory ? "in shared memory" : "in a global variable");
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
