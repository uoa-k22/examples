/*
 * 06_waitpid_order.c - waitpid() waits for one particular child.
 *
 * Run:   ./06_waitpid_order
 *        Compare with 05_wait_order.c:
 *        diff 05_wait_order.c 06_waitpid_order.c
 *
 * Look:  the same three children as in 05_wait_order.c: the last one
 *        created finishes first. waitpid(pid, ...) waits for the child with
 *        that pid, so the parent collects them in the order it chose
 *        (0, 1, 2), even though some of them finished earlier. Until then,
 *        those wait as zombies.
 */
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
    for (int i = 0; i < N; i++) {
        if (waitpid(pids[i], &status, 0) < 0)
            die("waitpid");
        printf("waitpid() collected child %d (pid %d)\n",
               WEXITSTATUS(status), (int) pids[i]);
    }
    return 0;
}
