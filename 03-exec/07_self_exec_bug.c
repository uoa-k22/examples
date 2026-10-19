/*
 * 07_self_exec_bug.c - a program that runs itself again through argv[0].
 *
 * Run:   ./07_self_exec_bug
 *        The fix is in 07_self_exec_fixed.c. To see what changed:
 *        diff 07_self_exec_bug.c 07_self_exec_fixed.c
 *
 * Look:  the program changes its directory to / (as servers often do) and
 *        then forks a child that execs a new copy of this same program,
 *        using argv[0] as the path. argv[0] is "./07_self_exec_bug": a path
 *        relative to the directory the program was started in. After
 *        chdir("/") that directory is no longer the current one, so exec
 *        fails with "No such file or directory".
 *
 *        argv[0] is whatever string the caller put there; it is only a
 *        convention (see 02_execvp.c). Started through $PATH, it is a bare
 *        name with no directory at all, and exec does not search $PATH for
 *        it, so the same exec fails from any directory but the program's
 *        own.
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

    const char *path = argv[0];     /* BUG: only a name, not a location */
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
