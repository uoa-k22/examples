/*
 * 05_wait_order.c - wait() returns the children in the order they finish.
 *
 * Run:   ./05_wait_order
 *        Compare with 06_waitpid_order.c:
 *        diff 05_wait_order.c 06_waitpid_order.c
 *
 * Look:  child i sleeps N - i seconds, so the last child created finishes
 *        first. wait() returns whichever child has finished, so the parent
 *        collects them in the order they finish (2, 1, 0). When no child is
 *        left, wait() returns -1 with ECHILD.
 */
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define N 3

int main(void)
{
    pid_t pids[N];

    for (int i = 0; i < N; i++) {
        fflush(stdout);     /* children must not inherit unprinted output */
        pids[i] = fork();
        if (pids[i] < 0)
            die("fork");
        if (pids[i] == 0) {
            sleep(N - i);
            printf("  child %d (pid %d) exiting\n", i, (int) getpid());
            return i;   /* the exit code tells the parent which child */
        }
        printf("created child %d: pid %d\n", i, (int) pids[i]);
    }

    int status;
    pid_t pid;
    while ((pid = wait(&status)) > 0)
        printf("wait() returned child %d (pid %d)\n",
               WEXITSTATUS(status), (int) pid);
    if (errno != ECHILD)
        die("wait");
    printf("wait() returned -1 with ECHILD: no children left\n");
    return 0;
}
