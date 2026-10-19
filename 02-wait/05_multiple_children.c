/*
 * 05_multiple_children.c - waiting for several children.
 *
 * Run:   ./05_multiple_children           reap with wait()
 *        ./05_multiple_children ordered   reap with waitpid(), in fork order
 *
 * Look:  child i sleeps N - i seconds, so the last child created finishes
 *        first. wait() returns whichever child finished first, in the order
 *        they finish (2, 1, 0). waitpid(pid, ...) waits for one particular
 *        child, so the parent collects them in the order it chose (0, 1, 2)
 *        even though some of them finished earlier.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define N 3

int main(int argc, char *argv[])
{
    int ordered = argc > 1 && strcmp(argv[1], "ordered") == 0;
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
    if (ordered) {
        for (int i = 0; i < N; i++) {
            if (waitpid(pids[i], &status, 0) < 0)
                die("waitpid");
            printf("waitpid() collected child %d (pid %d)\n",
                   WEXITSTATUS(status), (int) pids[i]);
        }
    } else {
        pid_t pid;
        while ((pid = wait(&status)) > 0)
            printf("wait() returned child %d (pid %d)\n",
                   WEXITSTATUS(status), (int) pid);
        if (errno != ECHILD)
            die("wait");
        printf("wait() returned -1 with ECHILD: no children left\n");
    }
    return 0;
}
