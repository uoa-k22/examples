/*
 * 08_fork_limit_bug.c - fork() can fail, and -1 is not a pid.
 *
 * Run:   ./08_fork_limit_bug
 *        The fix is in 08_fork_limit_fixed.c. To see what changed:
 *        diff 08_fork_limit_bug.c 08_fork_limit_fixed.c
 *
 * Look:  the program lowers its own limit on the number of processes
 *        (RLIMIT_NPROC) to 1, which is less than what is already running, so
 *        the next fork() fails with EAGAIN ("Resource temporarily
 *        unavailable"). Real programs hit this when they leak children, when
 *        a fork bomb is running (see 05_fork_loop_bug.c), or on a busy server.
 *        The limit applies only to this program. (It does not apply to root;
 *        as root the fork succeeds and the demo shows nothing.)
 *
 *        The program only tests  pid == 0  for the child and treats
 *        everything else as "I am the parent". But fork() returned -1, so
 *        there is no child, and -1 used as a pid means something else
 *        entirely: waitpid(-1, ...) waits for ANY child. This program has
 *        none, so the wait fails with ECHILD, unnoticed, and the program
 *        reports that "child -1" finished with status 0: a failure that
 *        looks like success. A program that did have other children would
 *        quietly reap one of them instead. (kill(-1, sig) is worse still: it
 *        signals every process you are allowed to signal.)
 */
#include <stdio.h>
#include <sys/resource.h>
#include <sys/types.h>
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
    /* BUG: no check for pid < 0 */

    if (pid == 0) {
        printf("child: running\n");
        return 0;
    }

    /* For pid == -1 this is not a parent; there is no child. */
    printf("fork() returned %d\n", (int) pid);
    int status = 0;
    waitpid(pid, &status, 0);   /* -1 means "any child": ECHILD, unnoticed */
    printf("child %d finished with status %d: success!\n",
           (int) pid, WEXITSTATUS(status));
    return 0;
}
