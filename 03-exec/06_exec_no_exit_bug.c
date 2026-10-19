/*
 * 06_exec_no_exit_bug.c - a child that does not exit when exec fails.
 *
 * Run:   ./06_exec_no_exit_bug
 *        The fix is in 06_exec_no_exit_fixed.c. To see what changed:
 *        diff 06_exec_no_exit_bug.c 06_exec_no_exit_fixed.c
 *
 * Look:  the child tries to run a command that does not exist. exec fails,
 *        and the child prints the error, but then it does not exit. The
 *        "parent" lines appear twice, once with the real parent's pid and
 *        once with the child's. The child's wait() fails with "No child
 *        processes", because it has no children. Then the child returns 0
 *        from main, so the real parent reports that its child "exited with
 *        status 0": a command that never ran now looks like a success.
 *
 * Why:   exec returns only when it fails. When it does, the child is still a
 *        complete copy of the parent, running the parent's program. After
 *        the if (pid == 0) { ... } block it simply continues with the code
 *        meant for the parent. Here that only prints a few lines, but it
 *        could be anything: writing files, sending data, or, inside a loop,
 *        calling fork() again until the processes multiply as in
 *        01-fork/05_fork_loop_bug.c.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

int main(void)
{
    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        execlp("no-such-command", "no-such-command", (char *) NULL);
        perror("child: execlp no-such-command");
        /* BUG: no exit here, so the child runs the parent's code below */
    }

    /* Only the parent is supposed to get here. */
    printf("pid %d: parent, waiting for my child\n", (int) getpid());

    int status;
    pid_t done = wait(&status);
    if (done < 0)
        perror("wait");
    else if (WIFEXITED(status))
        printf("pid %d: parent, child %d exited with status %d\n",
               (int) getpid(), (int) done, WEXITSTATUS(status));

    printf("pid %d: parent, finished\n", (int) getpid());
    return 0;
}
