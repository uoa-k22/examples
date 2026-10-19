/*
 * 08_fork_limit.c - fork() can fail, and -1 is not a pid.
 *
 * Run:   ./08_fork_limit          the bug
 *        ./08_fork_limit fixed    the fix
 *
 * Look:  the program lowers its own limit on the number of processes
 *        (RLIMIT_NPROC) to 1, which is less than what is already running, so
 *        the next fork() fails with EAGAIN ("Resource temporarily
 *        unavailable"). Real programs hit this when they leak children, when
 *        a fork bomb is running (see 05_fork_loop.c), or on a busy server.
 *        The limit applies only to this program. (It does not apply to root;
 *        as root the fork succeeds and the demo shows nothing.)
 *
 *        The buggy program only tests  pid == 0  for the child and treats
 *        everything else as "I am the parent". But fork() returned -1, so
 *        there is no child. The parent waits for "child -1", the wait fails
 *        unnoticed, and the program reports that the child finished with
 *        status 0: a failure that looks like success.
 *
 *        The fix is the check every other program here has:
 *        if (pid < 0) die("fork");
 */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

int main(int argc, char *argv[])
{
    int fixed = argc > 1 && strcmp(argv[1], "fixed") == 0;

    struct rlimit rl;
    if (getrlimit(RLIMIT_NPROC, &rl) < 0)
        die("getrlimit");
    rl.rlim_cur = 1;                    /* soft limit: only this program is affected */
    if (setrlimit(RLIMIT_NPROC, &rl) < 0)
        die("setrlimit");
    printf("process limit lowered to 1; calling fork()\n");
    fflush(stdout);

    pid_t pid = fork();
    if (fixed && pid < 0)
        die("fork");                    /* the fix */

    if (pid == 0) {
        printf("child: running\n");
        return 0;
    }

    /* BUG: for pid == -1 this is not a parent; there is no child. */
    printf("fork() returned %d\n", (int) pid);
    int status = 0;
    waitpid(pid, &status, 0);           /* fails with ECHILD, unnoticed */
    printf("child %d finished with status %d: success!\n",
           (int) pid, WEXITSTATUS(status));
    return 0;
}
