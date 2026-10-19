/*
 * 04_exec_fail_fixed.c - the fix for 04_exec_fail_bug.c.
 *
 * Run:   ./04_exec_fail_fixed
 *        diff 04_exec_fail_bug.c 04_exec_fail_fixed.c
 *
 * Look:  the fix is one line: a child whose exec failed must exit. Now only
 *        the parent loops, so there are exactly N = 3 "top of loop" lines,
 *        all with the parent's pid, and three error messages from the
 *        children. 127 is the status shells use for "command not found".
 *        Use _exit(), not exit(), in the child (see 06_exec_no_exit.c).
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define N 3

int main(void)
{
    for (int i = 0; i < N; i++) {
        printf("pid %d: top of loop, i = %d\n", (int) getpid(), i);
        fflush(stdout);

        pid_t pid = fork();
        if (pid < 0)
            die("fork");

        if (pid == 0) {
            execlp("no-such-command", "no-such-command", (char *) NULL);
            perror("  execlp no-such-command");
            _exit(127);     /* FIX: the child stops here */
        } else if (waitpid(pid, NULL, 0) < 0) {
            die("waitpid");
        }
    }
    return 0;
}
