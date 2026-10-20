/*
 * 07_deadlock_fixed.c - the fix for 07_deadlock_bug.c.
 *
 * Run:   ./07_deadlock_fixed
 *        diff 07_deadlock_bug.c 07_deadlock_fixed.c
 *
 * Look:  both processes take A first and then B. Whoever gets A first also
 *        gets B, and the other waits its turn. Both finish, well before the
 *        alarm.
 *
 * Rule:  if processes must hold several locks at once, they must all take
 *        them in the same global order. Then a cycle of waiting processes
 *        cannot form.
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
    setvbuf(stdout, NULL, _IOLBF, 0);

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
        /* FIX: the child takes A first, like the parent */
        use_both("    child ", &s->a, "A", &s->b, "B");
        return 0;
    }

    use_both("parent", &s->a, "A", &s->b, "B");
    if (waitpid(pid, NULL, 0) < 0)
        die("waitpid");
    printf("parent: both finished\n");
    return 0;
}
