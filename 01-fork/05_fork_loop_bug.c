/*
 * 05_fork_loop_bug.c - a child that forgets to exit inside a fork() loop.
 *
 * Run:   ./05_fork_loop_bug
 *        The fix is in 05_fork_loop_fixed.c. To see what changed:
 *        diff 05_fork_loop_bug.c 05_fork_loop_fixed.c
 *
 * Look:  the parent means to create N children, each printing one line. But
 *        the child does not exit after its line. It goes back to the top of
 *        the loop and calls fork() itself, and so do its children. After N
 *        iterations there are 2^N processes and 2^N - 1 "child" lines
 *        instead of N. Here N is small (3, so 8 processes), but with a large
 *        N, or a loop with no bound, this is a fork bomb that can make the
 *        machine unusable.
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
            /* BUG: no return here, so the child goes on with the loop */
        }
    }

    /* Every process that is still here waits for all of its own children. */
    while (wait(NULL) > 0)
        ;
    return 0;
}
