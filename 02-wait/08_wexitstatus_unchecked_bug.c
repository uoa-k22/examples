/*
 * 08_wexitstatus_unchecked_bug.c - WEXITSTATUS without checking WIFEXITED.
 *
 * Run:   ./08_wexitstatus_unchecked_bug
 *        The fix is in 08_wexitstatus_unchecked_fixed.c. To see what changed:
 *        diff 08_wexitstatus_unchecked_bug.c 08_wexitstatus_unchecked_fixed.c
 *
 * Look:  the child is killed by a signal before it can exit. The parent
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
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

int main(void)
{
    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        printf("child %d: working, then killed by a signal\n", (int) getpid());
        fflush(stdout);
        raise(SIGKILL);         /* as if it had crashed; no exit code */
    }

    int status;
    if (waitpid(pid, &status, 0) < 0)
        die("waitpid");
    printf("parent: raw status = 0x%04x\n", (unsigned) status);

    /* BUG: WEXITSTATUS is only valid if WIFEXITED(status) is true. */
    if (WEXITSTATUS(status) == 0)
        printf("parent: exit code 0, the child succeeded\n");
    else
        printf("parent: exit code %d, the child failed\n",
               WEXITSTATUS(status));
    return 0;
}
