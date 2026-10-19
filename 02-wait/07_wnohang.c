/*
 * 07_wnohang.c - checking on a child without blocking.
 *
 * Run:   ./07_wnohang
 *
 * Look:  wait() and waitpid() normally block until a child finishes. With
 *        the WNOHANG option, waitpid() returns at once:
 *          > 0  the pid of a child that has finished (and is now reaped)
 *            0  children exist, but none has finished yet
 *           -1  error, e.g. ECHILD: no children at all
 *        Here the parent keeps doing its own work and checks on the child
 *        between steps, until the child finishes after about 2 seconds.
 *
 *        A server that starts many children usually reaps all the finished
 *        ones at once, so that no zombies pile up:
 *            while (waitpid(-1, &status, WNOHANG) > 0)
 *                ;
 *        Polling in a tight loop would waste CPU. The parent should check
 *        between real pieces of work, or when the SIGCHLD signal tells it a
 *        child has finished.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

int main(void)
{
    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        sleep(2);
        return 7;
    }

    int status;
    for (int step = 1; ; step++) {
        pid_t done = waitpid(pid, &status, WNOHANG);
        if (done < 0)
            die("waitpid");
        if (done == pid)
            break;

        /* done == 0: the child is still running, so do some other work. */
        printf("parent: child %d still running, doing step %d of my own work\n",
               (int) pid, step);
        usleep(400000);
    }

    if (WIFEXITED(status))
        printf("parent: child %d finished with status %d\n",
               (int) pid, WEXITSTATUS(status));
    return 0;
}
