/*
 * 03_fork_exec_wait.c - the pattern every shell uses to run a command.
 *
 * Run:   ./03_fork_exec_wait ls -l /tmp
 *        ./03_fork_exec_wait false
 *        ./03_fork_exec_wait no-such-command
 *        ./03_fork_exec_wait sleep 30      (and kill the sleep from elsewhere)
 *
 * Look:  1. fork()   - make a new process
 *        2. exec()   - in the child, replace the program with the command
 *        3. wait()   - in the parent, wait for it and report how it ended
 *        If exec fails, the child must exit at once. It exits with 127, as
 *        shells do for "command not found".
 */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s command [args...]\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        execvp(argv[1], argv + 1);
        perror(argv[1]);
        _exit(127);
    }

    int status;
    if (waitpid(pid, &status, 0) < 0)
        die("waitpid");

    if (WIFEXITED(status))
        printf("[%s (pid %d) exited with status %d]\n",
               argv[1], (int) pid, WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("[%s (pid %d) was killed by signal %d: %s]\n",
               argv[1], (int) pid, WTERMSIG(status), strsignal(WTERMSIG(status)));
    return 0;
}
