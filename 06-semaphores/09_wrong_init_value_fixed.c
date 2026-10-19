/*
 * 09_wrong_init_value_fixed.c - the fix for 09_wrong_init_value_bug.c.
 *
 * Run:   ./09_wrong_init_value_fixed
 *        diff 09_wrong_init_value_bug.c 09_wrong_init_value_fixed.c
 *
 * Look:  the only change is the initial value: 1 instead of 0. The first
 *        child to arrive takes the lock, the other waits for it, both get
 *        into the critical section one after the other, and the counter
 *        ends at 2. Nobody is killed by the alarm.
 */
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define NPROC 2

struct shared {
    sem_t lock;
    long  counter;
};

int main(void)
{
    struct shared *s = mmap(NULL, sizeof(*s), PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (s == MAP_FAILED)
        die("mmap");
    /* FIX: a lock starts at 1: one process may enter. */
    if (sem_init(&s->lock, 1, 1) < 0)
        die("sem_init");
    setvbuf(stdout, NULL, _IONBF, 0);

    for (int p = 0; p < NPROC; p++) {
        pid_t pid = fork();
        if (pid < 0)
            die("fork");
        if (pid == 0) {
            alarm(3);       /* give up after 3 seconds */
            printf("child %d: waiting for the lock\n", p);
            if (sem_wait(&s->lock) < 0)
                die("sem_wait");
            s->counter++;
            printf("child %d: in the critical section\n", p);
            if (sem_post(&s->lock) < 0)
                die("sem_post");
            _exit(0);
        }
    }

    int status;
    pid_t pid;
    while ((pid = wait(&status)) > 0) {
        if (WIFSIGNALED(status))
            printf("parent: child %d was killed by %s: it never got the lock\n",
                   (int) pid, strsignal(WTERMSIG(status)));
        else
            printf("parent: child %d finished\n", (int) pid);
    }
    printf("parent: counter = %ld, expected %d\n", s->counter, NPROC);
    sem_destroy(&s->lock);
    return 0;
}
