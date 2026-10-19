/*
 * 13_eintr_bug.c - sem_wait() fails with EINTR once a signal handler exists.
 *
 * Run:   ./13_eintr_bug
 *        The fix is in 13_eintr_fixed.c. To see what changed:
 *        diff 13_eintr_bug.c 13_eintr_fixed.c
 *
 * Look:  the parent installs a SIGALRM handler that counts seconds, and then
 *        waits on a semaphore that the child posts after 3 seconds. The
 *        handler makes the parent's blocked sem_wait() return -1 with
 *        errno == EINTR ("Interrupted system call") after the first second.
 *        The parent treats that as a real error and exits, although
 *        nothing is wrong: the semaphore has simply not been posted yet.
 *
 *        This handler is installed without SA_RESTART. With SA_RESTART the
 *        kernel would restart sem_wait() by itself. Do not rely on that: it
 *        is not true for every blocking call (System V semop(), for example,
 *        is never restarted), and it does not help in code that cannot know
 *        how the handler was installed. See "About EINTR" in the README.
 */
#include <errno.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

static volatile sig_atomic_t seconds;

static void on_alarm(int sig)
{
    (void) sig;
    seconds++;
    alarm(1);                   /* tick again in a second */
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    sem_t *done = mmap(NULL, sizeof(*done), PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (done == MAP_FAILED)
        die("mmap");
    if (sem_init(done, 1, 0) < 0)
        die("sem_init");

    pid_t pid = fork();
    if (pid < 0)
        die("fork");
    if (pid == 0) {
        sleep(3);
        printf("child: finished, posting the semaphore\n");
        if (sem_post(done) < 0)
            die("sem_post");
        _exit(0);
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_alarm;       /* no SA_RESTART */
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGALRM, &sa, NULL) < 0)
        die("sigaction");
    alarm(1);

    printf("parent: waiting for the child (a tick every second)\n");
    /* BUG: EINTR is not an error, but this gives up on the first tick. */
    if (sem_wait(done) < 0) {
        perror("parent: sem_wait");
        kill(pid, SIGKILL);     /* don't leave the child running */
        waitpid(pid, NULL, 0);
        return 1;
    }
    printf("parent: the child is done\n");
    alarm(0);

    if (waitpid(pid, NULL, 0) < 0)
        die("waitpid");
    return 0;
}
