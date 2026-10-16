/*
 * 04_exec_fail_bug.c - what happens if a child continues after exec fails.
 *
 * Run:   ./04_exec_fail_bug          the bug
 *        ./04_exec_fail_bug fixed    the fix
 *
 * Look:  the parent tries to run a command N times in a loop. The command
 *        does not exist, so every exec fails. In the buggy version the
 *        child just carries on. It is still a copy of the parent, so it
 *        goes back to the top of the loop and forks children of its own.
 *        Count the "top of loop" lines and the different pids: N = 3 gives
 *        7 lines instead of 3. With a real loop count this is a fork bomb.
 *
 *        The fix is one line: a child whose exec failed must exit.
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
    int fixed = argc > 1 && strcmp(argv[1], "fixed") == 0;

    for (int i = 0; i < N; i++) {
        printf("pid %d: top of loop, i = %d\n", (int) getpid(), i);
        fflush(stdout);

        pid_t pid = fork();
        if (pid < 0)
            die("fork");

        if (pid == 0) {
            execlp("no-such-command", "no-such-command", (char *) NULL);
            perror("  execlp no-such-command");
            if (fixed)
                _exit(127);
            /* BUG: no exit here, so the child continues the parent's loop */
        } else if (waitpid(pid, NULL, 0) < 0) {
            die("waitpid");
        }
    }
    return 0;
}
