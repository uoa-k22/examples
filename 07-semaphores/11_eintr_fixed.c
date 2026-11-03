/*
 * 11_eintr_fixed.c - the fix for 11_eintr_bug.c.
 *
 * Run:   ./11_eintr_fixed
 *        diff 11_eintr_bug.c 11_eintr_fixed.c
 *
 * Look:  EINTR means "a signal arrived, nothing failed". The parent calls
 *        sem_wait() again whenever it fails with EINTR, and gives up only on
 *        a real error. It keeps counting seconds and, when the child posts,
 *        gets the semaphore. This is the loop from "About EINTR" in the
 *        README.
 */
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define NAME "/k22_sem_eintr"

static volatile sig_atomic_t seconds;

static void on_alarm(int sig)
{
    (void) sig;
    seconds++;
    alarm(1);                   /* tick again in a second */
}

int main(void)
{
    setvbuf(stdout, NULL, _IOLBF, 0);

    int fd = shm_open(NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd < 0)
        die("shm_open");
    if (shm_unlink(NAME) < 0)
        die("shm_unlink");
    if (ftruncate(fd, sizeof(sem_t)) < 0)
        die("ftruncate");
    sem_t *done = mmap(NULL, sizeof(*done), PROT_READ | PROT_WRITE,
                       MAP_SHARED, fd, 0);
    if (done == MAP_FAILED)
        die("mmap");
    close(fd);
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
        return 0;
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
