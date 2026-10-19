/*
 * 05_exec_no_exit_fixed.c - the fix for 05_exec_no_exit_bug.c.
 *
 * Run:   ./05_exec_no_exit_fixed
 *        diff 05_exec_no_exit_bug.c 05_exec_no_exit_fixed.c
 *
 * Look:  the child's branch ends with an exit right after exec. Now only
 *        the parent prints "parent" lines, all with the same pid, and it
 *        reports that the child exited with status 127, the status shells
 *        use for "command not found".
 *
 *        Use _exit(), not exit(). exit() would also flush the stdio buffers
 *        the child inherited from the parent, so their contents would be
 *        printed twice (see 01-fork/06_stdio_buffer_bug.c).
 */
#include <stdio.h>
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
        _exit(127);     /* FIX: the child stops here */
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
