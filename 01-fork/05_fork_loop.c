/*
 * 05_fork_loop.c - creating several children in a loop.
 *
 * Run:   ./05_fork_loop        the correct version
 *        ./05_fork_loop bug    the child forgets to exit
 *
 * Look:  in the correct version the parent creates N children. Each child
 *        prints one line and exits, so exactly N lines of "child" output
 *        appear.
 *
 *        In the "bug" version the child does not exit. It goes back to the
 *        top of the loop and calls fork() itself. After N iterations there
 *        are 2^N processes. Here N is small (3, so 8 processes), but with a
 *        large N, or a loop with no bound, this is a fork bomb that can make
 *        the machine unusable. Never leave a child running the parent's loop.
 */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define N 3

int main(int argc, char *argv[])
{
    int bug = argc > 1 && strcmp(argv[1], "bug") == 0;

    for (int i = 0; i < N; i++) {
        pid_t pid = fork();
        if (pid < 0)
            die("fork");

        if (pid == 0) {
            printf("i = %d: child  %d (parent %d)\n",
                   i, (int) getpid(), (int) getppid());
            fflush(stdout);
            if (!bug)
                return 0;   /* done; the child must not continue the loop */
        }
    }

    /* Every process that is still here waits for all of its own children. */
    while (wait(NULL) > 0)
        ;
    return 0;
}
