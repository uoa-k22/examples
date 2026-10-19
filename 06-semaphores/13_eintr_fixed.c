/*
 * 13_eintr_fixed.c - the fix for 13_eintr_bug.c.
 *
 * Run:   ./13_eintr_fixed
 *        diff 13_eintr_bug.c 13_eintr_fixed.c
 *
 * Look:  EINTR means "a signal arrived, nothing failed". The parent calls
 *        sem_wait() again whenever it fails with EINTR, and gives up only on
 *        a real error. It keeps counting seconds and, when the child posts,
 *        gets the semaphore. This is the loop from "About EINTR" in the
 *        README.
 */
#include <errno.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
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
    int ticks = 0;
    while (sem_wait(done) < 0) {
        if (errno != EINTR)     /* a real error */
            die("sem_wait");
        /* FIX: EINTR only means a signal arrived: wait again. */
        printf("parent: interrupted by the timer (%d s), waiting again\n",
               (int) seconds);
        ticks++;
    }
    printf("parent: the child is done after %d interruptions\n", ticks);
    alarm(0);

    if (waitpid(pid, NULL, 0) < 0)
        die("waitpid");
    return 0;
}
