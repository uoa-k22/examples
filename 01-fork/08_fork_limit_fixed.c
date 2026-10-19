/*
 * 08_fork_limit_fixed.c - the fix for 08_fork_limit_bug.c.
 *
 * Run:   ./08_fork_limit_fixed
 *        diff 08_fork_limit_bug.c 08_fork_limit_fixed.c
 *
 * Look:  the same lowered process limit, so fork() still fails. This time
 *        the program checks for it, the check every other program here has:
 *        if (pid < 0) die("fork");
 *        It reports "fork: Resource temporarily unavailable" and exits with
 *        status 1, instead of pretending a child ran. The parent's waitpid()
 *        is checked too, so a failed wait could not go unnoticed either.
 */
#include <stdio.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

int main(void)
{
    struct rlimit rl;
    if (getrlimit(RLIMIT_NPROC, &rl) < 0)
        die("getrlimit");
    rl.rlim_cur = 1;        /* soft limit: only this program is affected */
    if (setrlimit(RLIMIT_NPROC, &rl) < 0)
        die("setrlimit");
    printf("process limit lowered to 1; calling fork()\n");
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0)            /* FIX: fork() can fail */
        die("fork");

    if (pid == 0) {
        printf("child: running\n");
        return 0;
    }

    /* Only a real parent gets here: pid is the child's pid. */
    printf("fork() returned %d\n", (int) pid);
    int status;
    if (waitpid(pid, &status, 0) < 0)
        die("waitpid");
    printf("child %d finished with status %d\n",
           (int) pid, WEXITSTATUS(status));
    return 0;
}
