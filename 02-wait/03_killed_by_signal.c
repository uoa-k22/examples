/*
 * 03_killed_by_signal.c - children that are terminated by a signal.
 *
 * Run:   ./03_killed_by_signal
 *
 * Look:  three children, one after the other, each killed by a signal:
 *        1. the first writes through a NULL pointer: SIGSEGV (11)
 *        2. the second calls abort(): SIGABRT (6)
 *        3. the third waits, and the parent kills it with kill(): SIGTERM (15)
 *        No child calls exit(), so WIFEXITED is false. Instead WIFSIGNALED
 *        is true and WTERMSIG gives the signal number. The shell does the
 *        same check to print "Segmentation fault".
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

/* Wait for the child and show what its status says. */
static void report(pid_t pid)
{
    int status;
    if (waitpid(pid, &status, 0) < 0)
        die("waitpid");

    printf("parent: child %d, raw status = 0x%04x\n",
           (int) pid, (unsigned) status);
    printf("  WIFEXITED   = %d\n", WIFEXITED(status));
    printf("  WIFSIGNALED = %d\n", WIFSIGNALED(status));
    if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        printf("  WTERMSIG    = %d (%s)\n", sig, strsignal(sig));
#ifdef WCOREDUMP
        printf("  WCOREDUMP   = %d\n", WCOREDUMP(status) ? 1 : 0);
#endif
    }
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    pid_t pid = fork();
    if (pid < 0)
        die("fork");
    if (pid == 0) {
        printf("child %d: writing through a NULL pointer\n", (int) getpid());
        *(volatile int *) NULL = 1;
        return 0;
    }
    report(pid);

    pid = fork();
    if (pid < 0)
        die("fork");
    if (pid == 0) {
        printf("child %d: calling abort()\n", (int) getpid());
        abort();
    }
    report(pid);

    pid = fork();
    if (pid < 0)
        die("fork");
    if (pid == 0) {
        for (;;)
            pause();
    }
    printf("parent: sending SIGTERM to child %d\n", (int) pid);
    if (kill(pid, SIGTERM) < 0)
        die("kill");
    report(pid);
    return 0;
}
