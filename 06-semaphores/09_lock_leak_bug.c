/*
 * 09_lock_leak_bug.c - a code path that forgets to unlock.
 *
 * Run:   ./09_lock_leak_bug
 *        The fix is in 09_lock_leak_fixed.c. To see what changed:
 *        diff 09_lock_leak_bug.c 09_lock_leak_fixed.c
 *
 * Look:  update() locks, checks its argument, and returns early if the
 *        argument is bad. The early return skips sem_post(), so the lock is
 *        never released. The first child passes a bad value and exits
 *        normally. The second child passes a good value, but the lock is
 *        gone and it waits forever. alarm(3) ends the demo, and the parent
 *        reports it.
 *
 *        The failure is far from its cause: the process that leaked the lock
 *        is long gone, and the one that hangs did nothing wrong. Every path
 *        out of a critical section, including every error return, must
 *        release the lock. 09_lock_leak_fixed.c leaves through a single
 *        exit point, which makes that easy to check.
 *
 *        (With a System V semaphore and SEM_UNDO the kernel would release the
 *        lock when the child exited: see 07-sysv-ipc/04_owner_died_fixed.c.)
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
    if (sem_wait(&s->lock) < 0)
        die("sem_wait");

    if (value < 0) {
        printf("  update(%d): bad value\n", value);
        return -1;              /* BUG: returns while still holding the lock */
    }
    s->total += value;
    printf("  update(%d): total is now %ld\n", value, s->total);

    if (sem_post(&s->lock) < 0)
        die("sem_post");
    return 0;
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);

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
