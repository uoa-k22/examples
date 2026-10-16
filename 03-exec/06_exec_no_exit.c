/*
 * 06_exec_no_exit.c - a child that does not exit when exec fails.
 *
 * Run:   ./06_exec_no_exit
 *
 * Look:  as written, the child tries to run a command that does not exist.
 *        exec fails, the child prints the error and exits with 127, and the
 *        parent reports it. Every "parent" line shows the same pid.
 *
 * Try:   comment out the _exit(127) line marked below, run  make  and run
 *        the program again. Now the "parent" lines appear twice, once with
 *        the real parent's pid and once with the child's. The child's wait()
 *        fails with "No child processes", because it has no children. Then
 *        the child returns 0 from main, so the real parent reports that its
 *        child "exited with status 0": a command that never ran now looks
 *        like a success.
 *
 * Why:   exec returns only when it fails. When it does, the child is still a
 *        complete copy of the parent, running the parent's program. After
 *        the if (pid == 0) { ... } block it simply continues with the code
 *        meant for the parent. Here that only prints a few lines, but it
 *        could be anything: writing files, sending data, or calling fork()
 *        again in a loop (see 04_exec_fail_bug.c).
 *
 * Fix:   always end the child's branch with an exit right after exec. Use
 *        _exit(), not exit(). exit() would also flush the stdio buffers the
 *        child inherited from the parent (see 01-fork/06_stdio_buffer.c).
 *        127 is the status shells use for "command not found".
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
        _exit(127);     /* <-- comment out this line to see the bug */
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
