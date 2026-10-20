/*
 * 09_lock_leak_fixed.c - the fix for 09_lock_leak_bug.c.
 *
 * Run:   ./09_lock_leak_fixed
 *        diff 09_lock_leak_bug.c 09_lock_leak_fixed.c
 *
 * Look:  update() has a single way out: the error path jumps to the label
 *        "out", where the lock is released, instead of returning directly.
 *        The first child still reports its bad value, but it releases the
 *        lock, and the second child gets it and finishes.
 *
 *        Whatever the style (a goto to the end, or a wrapper function that
 *        locks, calls, and unlocks), the rule is the same: every path out of
 *        a critical section must release the lock.
 */
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

struct shared {
    sem_t lock;
    long  total;
};

static struct shared *s;

/* Add value to the total. Negative values are not allowed. */
static int update(int value)
{
    int result = 0;

    if (sem_wait(&s->lock) < 0)
        die("sem_wait");

    if (value < 0) {
        printf("  update(%d): bad value\n", value);
        result = -1;
        goto out;               /* FIX: leave through the unlock below */
    }
    s->total += value;
    printf("  update(%d): total is now %ld\n", value, s->total);
out:
    if (sem_post(&s->lock) < 0)
        die("sem_post");
    return result;
}

int main(void)
{
    setvbuf(stdout, NULL, _IOLBF, 0);

    s = mmap(NULL, sizeof(*s), PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (s == MAP_FAILED)
        die("mmap");
    if (sem_init(&s->lock, 1, 1) < 0)
        die("sem_init");

    /* The first child's bad value takes the error path; the second child's
       good value needs the lock. One child at a time. */
    const int values[] = { -1, 7 };
    for (int i = 0; i < 2; i++) {
        pid_t pid = fork();
        if (pid < 0)
            die("fork");
        if (pid == 0) {
            alarm(3);           /* give up after 3 seconds */
            printf("child %d: calling update(%d)\n",
                   (int) getpid(), values[i]);
            update(values[i]);
            return 0;
        }

        int status;
        if (waitpid(pid, &status, 0) < 0)
            die("waitpid");
        if (WIFSIGNALED(status))
            printf("parent: child %d was killed by %s: "
                   "it was stuck waiting for the lock\n",
                   (int) pid, strsignal(WTERMSIG(status)));
        else
            printf("parent: child %d finished\n", (int) pid);
    }

    sem_destroy(&s->lock);
    return 0;
}
