/*
 * 08_deadlock_bug.c - two processes that each wait for the other forever.
 *
 * Run:   ./08_deadlock_bug
 *        The fix is in 08_deadlock_fixed.c. To see what changed:
 *        diff 08_deadlock_bug.c 08_deadlock_fixed.c
 *
 * Look:  parent and child each need both semaphores, A and B, but they take
 *        them in opposite orders. The parent holds A and waits for B; the
 *        child holds B and waits for A. Neither can continue, and neither
 *        will give up what it holds: a deadlock. Each process calls
 *        alarm(5), which kills it after 5 seconds ("Alarm clock"), so the
 *        demo ends by itself; without it, both would hang until killed.
 *
 *        The usleep() between taking the two semaphores makes the bad
 *        timing happen every time. Without it the deadlock would happen
 *        only now and then, which is what makes it so hard to find in real
 *        programs.
 */
#include <semaphore.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

struct shared {
    sem_t a, b;
};

static void P(sem_t *sem) { if (sem_wait(sem) < 0) die("sem_wait"); }
static void V(sem_t *sem) { if (sem_post(sem) < 0) die("sem_post"); }

static void use_both(const char *who, sem_t *first, const char *n1,
                     sem_t *second, const char *n2)
{
    printf("%s: waiting for %s\n", who, n1);
    P(first);
    printf("%s: got %s\n", who, n1);

    usleep(500000);

    printf("%s: waiting for %s\n", who, n2);
    P(second);
    printf("%s: got %s and %s, working\n", who, n1, n2);

    usleep(200000);
    V(second);
    V(first);
    printf("%s: released both\n", who);
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    struct shared *s = mmap(NULL, sizeof(*s), PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (s == MAP_FAILED)
        die("mmap");
    if (sem_init(&s->a, 1, 1) < 0 || sem_init(&s->b, 1, 1) < 0)
        die("sem_init");

    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    alarm(5);       /* both processes: give up after 5 seconds */

    if (pid == 0) {
        /* BUG: the child takes B first, the parent takes A first */
        use_both("    child ", &s->b, "B", &s->a, "A");
        return 0;
    }

    use_both("parent", &s->a, "A", &s->b, "B");
    if (waitpid(pid, NULL, 0) < 0)
        die("waitpid");
    printf("parent: both finished\n");
    return 0;
}
