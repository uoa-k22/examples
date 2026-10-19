/*
 * 10_wrong_init_value_bug.c - a lock that starts at 0 can never be taken.
 *
 * Run:   ./10_wrong_init_value_bug
 *        The fix is in 10_wrong_init_value_fixed.c. To see what changed:
 *        diff 10_wrong_init_value_bug.c 10_wrong_init_value_fixed.c
 *
 * Look:  a semaphore used as a lock must start at 1: one process may enter.
 *        This program starts it at 0, so the very first sem_wait()
 *        blocks, and since nobody is inside to call sem_post(), it blocks
 *        forever. Nothing reports an error; the program just hangs. Each
 *        child calls alarm(3) so the demo ends by itself, and the parent
 *        reports that SIGALRM killed it while it was still waiting.
 *
 *        The same hang happens if sem_init() is left out altogether. The
 *        value of a sem_t that was never initialised is undefined (often 0,
 *        in fresh memory from mmap()).
 *
 *        Remember: start at 1 for mutual exclusion, at 0 for signalling
 *        (see 04_pingpong.c). Starting a lock at 0 turns it into a signal
 *        nobody sends.
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
    /* BUG: a lock must start at 1, not 0. */
    if (sem_init(&s->lock, 1, 0) < 0)
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
