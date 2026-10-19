/*
 * 08_wexitstatus_unchecked_fixed.c - the fix for
 * 08_wexitstatus_unchecked_bug.c.
 *
 * Run:   ./08_wexitstatus_unchecked_fixed
 *        diff 08_wexitstatus_unchecked_bug.c 08_wexitstatus_unchecked_fixed.c
 *
 * Look:  the same child, killed by a signal. The parent first asks how the
 *        child ended. WIFEXITED is false, so WEXITSTATUS is not read at all;
 *        WIFSIGNALED is true, and the parent reports the signal and calls it
 *        a failure, as it should.
 */
#include <signal.h>
#include <stdio.h>
#include <string.h>
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

    /* FIX: ask how the child ended before reading the exit code. */
    if (WIFEXITED(status))
        printf("parent: the child exited with code %d\n",
               WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("parent: the child was killed by signal %d (%s), so it failed\n",
               WTERMSIG(status), strsignal(WTERMSIG(status)));
    return 0;
}
