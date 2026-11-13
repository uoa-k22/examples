/*
 * 07_deadlock_bug.c - two processes that each wait for the other forever.
 *
 * Run:   ./07_deadlock_bug
 *        The fix is in 07_deadlock_fixed.c. To see what changed:
 *        diff 07_deadlock_bug.c 07_deadlock_fixed.c
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
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define NAME "/k22_sem_deadlock"

struct shared {
    sem_t a, b;
};

static void use_both(const char *who, sem_t *first, const char *n1,
                     sem_t *second, const char *n2)
{
    printf("%s: waiting for %s\n", who, n1);
    if (sem_wait(first) < 0)
        die("sem_wait");
    printf("%s: got %s\n", who, n1);

    usleep(500000);

    printf("%s: waiting for %s\n", who, n2);
    if (sem_wait(second) < 0)
        die("sem_wait");
    printf("%s: got %s and %s, working\n", who, n1, n2);

    usleep(200000);
    if (sem_post(second) < 0)
        die("sem_post");
    if (sem_post(first) < 0)
        die("sem_post");
    printf("%s: released both\n", who);
}

int main(void)
{
    setvbuf(stdout, NULL, _IOLBF, 0);

    int fd = shm_open(NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd < 0)
        die("shm_open");
    if (shm_unlink(NAME) < 0)
        die("shm_unlink");
    if (ftruncate(fd, sizeof(struct shared)) < 0)
        die("ftruncate");
    struct shared *s = mmap(NULL, sizeof(*s), PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
    if (s == MAP_FAILED)
        die("mmap");
    close(fd);
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
