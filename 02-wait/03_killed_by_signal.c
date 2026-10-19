/*
 * 03_killed_by_signal.c - a child that is terminated by a signal.
 *
 * Run:   ./03_killed_by_signal segv    the child dereferences NULL
 *        ./03_killed_by_signal abort   the child calls abort()
 *        ./03_killed_by_signal wait    the child waits until you kill it,
 *                                      e.g. with  kill <pid>  or  kill -9 <pid>
 *
 * Look:  the child never calls exit(), so WIFEXITED is false. Instead
 *        WIFSIGNALED is true and WTERMSIG gives the signal number:
 *        SIGSEGV (11), SIGABRT (6), SIGTERM (15) or SIGKILL (9).
 *        The shell does the same check to print "Segmentation fault".
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

static void child(const char *how)
{
    if (strcmp(how, "segv") == 0) {
        printf("child %d: writing through a NULL pointer\n", (int) getpid());
        fflush(stdout);
        *(volatile int *) NULL = 1;
    } else if (strcmp(how, "abort") == 0) {
        printf("child %d: calling abort()\n", (int) getpid());
        fflush(stdout);
        abort();
    } else {    /* "wait": main() accepts nothing else */
        printf("child %d: waiting; kill me with  kill %d\n",
               (int) getpid(), (int) getpid());
        fflush(stdout);
        for (;;)
            pause();
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2 || (strcmp(argv[1], "segv") != 0 &&
                      strcmp(argv[1], "abort") != 0 &&
                      strcmp(argv[1], "wait") != 0)) {
        fprintf(stderr, "Usage: %s segv|abort|wait\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0)
        die("fork");
    if (pid == 0)
        child(argv[1]);     /* does not return */

    int status;
    if (waitpid(pid, &status, 0) < 0)
        die("waitpid");

    printf("parent: raw status = 0x%04x\n", (unsigned) status);
    printf("  WIFEXITED   = %d\n", WIFEXITED(status));
    printf("  WIFSIGNALED = %d\n", WIFSIGNALED(status));
    if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        printf("  WTERMSIG    = %d (%s)\n", sig, strsignal(sig));
#ifdef WCOREDUMP
        printf("  WCOREDUMP   = %d\n", WCOREDUMP(status) ? 1 : 0);
#endif
    }
    return 0;
}
