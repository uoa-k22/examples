/*
 * 04_zombie.c - a child that has exited but has not been waited for.
 *
 * Run:   ./04_zombie
 *
 * Look:  the child exits at once, but the parent does not call wait() for
 *        two seconds. During that time ps shows the child in state "Z"
 *        (zombie, marked <defunct>). The kernel keeps its pid and exit
 *        status so the parent can still collect them. After waitpid() the
 *        zombie is gone and ps finds nothing.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

static void show(pid_t pid)
{
    char cmd[64];

    snprintf(cmd, sizeof(cmd), "ps -o pid,ppid,stat,comm -p %d", (int) pid);
    fflush(stdout);
    if (system(cmd) < 0)
        die("system");
}

int main(void)
{
    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        printf("child %d: exiting with status 2\n", (int) getpid());
        exit(2);
    }

    sleep(2);
    printf("\nparent: the child has exited, "
           "but I have not waited for it yet:\n");
    show(pid);

    int status;
    if (waitpid(pid, &status, 0) < 0)
        die("waitpid");
    printf("\nparent: waitpid() collected exit status %d; "
           "the zombie is gone:\n",
           WEXITSTATUS(status));
    show(pid);
    return 0;
}
