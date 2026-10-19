/*
 * 07_wexitstatus_unchecked.c - WEXITSTATUS without checking WIFEXITED.
 *
 * Run:   ./07_wexitstatus_unchecked          the bug
 *        ./07_wexitstatus_unchecked fixed    the fix
 *
 * Look:  the child is killed by a signal before it can exit. The buggy parent
 *        reads WEXITSTATUS(status) and treats 0 as success. But a child that
 *        died from a signal never had an exit code. The status holds the
 *        signal number in its low bits and 0 in the bits WEXITSTATUS reads,
 *        so the answer looks like "exit code 0", and a crash is reported as
 *        success. Compare the raw status with 02_exit_status.c.
 *
 *        WEXITSTATUS is meaningful only when WIFEXITED is true. Check
 *        WIFEXITED first, then WIFSIGNALED (see 03_killed_by_signal.c).
 */
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

int main(int argc, char *argv[])
{
    int fixed = argc > 1 && strcmp(argv[1], "fixed") == 0;

    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        printf("Child %d: working, then killed by a signal\n", (int) getpid());
        fflush(stdout);
        raise(SIGKILL);         /* as if it had crashed; no exit code */
    }

    int status;
    if (waitpid(pid, &status, 0) < 0)
        die("waitpid");
    printf("Parent: raw status = 0x%04x\n", (unsigned) status);

    if (!fixed) {
        /* BUG: WEXITSTATUS is only valid if WIFEXITED(status) is true. */
        if (WEXITSTATUS(status) == 0)
            printf("Parent: exit code 0, the child succeeded\n");
        else
            printf("Parent: exit code %d, the child failed\n", WEXITSTATUS(status));
    } else if (WIFEXITED(status)) {
        printf("Parent: the child exited with code %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("Parent: the child was killed by signal %d (%s), so it failed\n",
               WTERMSIG(status), strsignal(WTERMSIG(status)));
    }
    return 0;
}
