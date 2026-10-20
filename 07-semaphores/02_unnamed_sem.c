/*
 * 02_unnamed_sem.c - the race from 01_race.c, fixed with a semaphore.
 *
 * Run:   ./02_unnamed_sem
 *        ./02_unnamed_sem 100
 *
 * Look:  the semaphore starts at 1, so it works as a lock (a mutex).
 *        sem_wait() decrements it, blocking while it is 0, and sem_post()
 *        increments it. Only one process at a time can be between the two
 *        calls, so no update is lost and the total is always exact. The
 *        price is speed: compare  time ./01_race  with  time ./02_unnamed_sem
 *
 *        An unnamed semaphore is just a sem_t in memory. To be shared by
 *        processes it must live in shared memory, and sem_init() must be
 *        called with pshared = 1.
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
    sem_t lock;
    long  counter;
};

int main(int argc, char *argv[])
{
    long count = argc > 1 ? atol(argv[1]) : 1000000;

    struct shared *s = mmap(NULL, sizeof(*s), PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (s == MAP_FAILED)
        die("mmap");
    if (sem_init(&s->lock, 1, 1) < 0)      /* pshared = 1, initial value = 1 */
        die("sem_init");
    s->counter = 0;

    for (int p = 0; p < NPROC; p++) {
        pid_t pid = fork();
        if (pid < 0)
            die("fork");
        if (pid == 0) {
            for (long i = 0; i < count; i++) {
                if (sem_wait(&s->lock) < 0)
                    die("sem_wait");
                s->counter++;                   /* critical section */
                if (sem_post(&s->lock) < 0)
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

    sem_destroy(&s->lock);
    munmap(s, sizeof(*s));
    return 0;
}
