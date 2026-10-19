/*
 * 08_self_exec.c - a program that runs itself with exec, and how that fails.
 *
 * Run:   ./08_self_exec            works: execs /proc/self/exe
 *        ./08_self_exec argv0      execs argv[0]; works when run like this
 *        ./08_self_exec chdir      execs argv[0] after chdir("/"): FAILS
 *        (cd /tmp && PATH=$OLDPWD:$PATH 08_self_exec argv0)    FAILS
 *
 * Look:  the parent forks a child, and the child execs a new copy of this
 *        same program (with the argument "child") to print a greeting.
 *        Which path does it exec?
 *          /proc/self/exe  a Linux link to the running executable. It names
 *                          the right file whatever the directory or $PATH.
 *          argv[0]         whatever string the caller put there. It is only a
 *                          convention (see 02_execvp.c), and exec does not
 *                          search $PATH for it when it has no '/'.
 *        Two ways argv[0] goes wrong:
 *        - "chdir": argv[0] is "./08_self_exec", a path relative to the
 *          directory the program was started in. After chdir("/") that
 *          directory is no longer the current one, so exec fails with
 *          "No such file or directory".
 *        - Started through $PATH from another directory, the shell passes
 *          argv[0] = "08_self_exec", with no directory at all. execl() does
 *          not search $PATH, so it looks in the current directory and fails.
 *          (From the program's own directory it would work, which hides the
 *          bug; hence the cd in the command above.)
 *
 *        In both cases the child's exec fails and it exits with 127, so the
 *        parent can report the failure. 05-shm/04_shm_pointers.c re-runs
 *        itself through /proc/self/exe for this reason.
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
        printf("  new copy running: pid %d, argv[0] = \"%s\"\n",
               (int) getpid(), argv[0]);
        return 0;
    }

    int use_argv0 = argc > 1 && (strcmp(argv[1], "argv0") == 0 ||
                                 strcmp(argv[1], "chdir") == 0);
    int do_chdir = argc > 1 && strcmp(argv[1], "chdir") == 0;
    const char *path = use_argv0 ? argv[0] : "/proc/self/exe";

    printf("pid %d: argv[0] = \"%s\", will exec \"%s\"\n",
           (int) getpid(), argv[0], path);
    if (do_chdir) {
        if (chdir("/") < 0)
            die("chdir");
        printf("pid %d: changed directory to /\n", (int) getpid());
    }
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        execl(path, path, "child", (char *) NULL);
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
