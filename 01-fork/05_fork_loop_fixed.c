/*
 * 05_fork_loop_fixed.c - the fix for 05_fork_loop_bug.c.
 *
 * Run:   ./05_fork_loop_fixed
 *        diff 05_fork_loop_bug.c 05_fork_loop_fixed.c
 *
 * Look:  the child returns as soon as it has printed its line, so only the
 *        parent goes round the loop: exactly N = 3 children, N "child"
 *        lines, all with the same parent pid. Never leave a child running
 *        the parent's loop.
 */
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define N 3

int main(void)
{
    for (int i = 0; i < N; i++) {
        pid_t pid = fork();
        if (pid < 0)
            die("fork");

        if (pid == 0) {
            printf("i = %d: child  %d (parent %d)\n",
                   i, (int) getpid(), (int) getppid());
            fflush(stdout);
            return 0;   /* FIX: the child is done; it must not continue */
        }
    }

    /* Every process that is still here waits for all of its own children. */
    while (wait(NULL) > 0)
        ;
    return 0;
}
