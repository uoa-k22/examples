/*
 * 07_self_exec_fixed.c - the fix for 07_self_exec_bug.c.
 *
 * Run:   ./07_self_exec_fixed
 *        diff 07_self_exec_bug.c 07_self_exec_fixed.c
 *
 * Look:  the child execs /proc/self/exe instead of argv[0]. On Linux that
 *        is a link to the executable of the running process, so it names
 *        the right file whatever the current directory, and however the
 *        program was started. The new copy runs and the child exits
 *        normally. 05-shm/04_shm_pointers_bug.c and _fixed.c re-run
 *        themselves the same way.
 */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "child") == 0) {
        printf("  new copy running: pid %d\n", (int) getpid());
        return 0;
    }

    const char *path = "/proc/self/exe";    /* FIX: always this program */
    printf("pid %d: will exec \"%s\" after chdir(\"/\")\n",
           (int) getpid(), path);
    if (chdir("/") < 0)
        die("chdir");
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        execl(path, argv[0], "child", (char *) NULL);
        perror(path);       /* exec returns only if it failed */
        _exit(127);
    }

    int status;
    if (waitpid(pid, &status, 0) < 0)
        die("waitpid");
    if (WIFEXITED(status) && WEXITSTATUS(status) == 127)
        printf("pid %d: the child could not exec \"%s\" (status 127)\n",
               (int) getpid(), path);
    else
        printf("pid %d: the child ran and exited normally\n", (int) getpid());
    return 0;
}
