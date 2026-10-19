/*
 * 04_env.c - giving a new program its environment.
 *
 * Run:   ./04_env
 *
 * Look:  most exec functions pass the caller's environment on unchanged.
 *        execle() and execve() take an explicit environment instead. The
 *        child here runs the env command with an environment made of just
 *        two variables, so HOME, PATH and the rest are gone. From the shell,
 *        env -i COURSE=K22 TOPIC=exec env  does the same.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

int main(void)
{
    const char *home = getenv("HOME");     /* NULL if the variable is not set */
    printf("parent: HOME = %s\n", home ? home : "(not set)");
    printf("child's environment:\n");
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        char *env[] = { "COURSE=K22", "TOPIC=exec", NULL };
        execle("/usr/bin/env", "env", (char *) NULL, env);
        die("execle /usr/bin/env");
    }

    if (waitpid(pid, NULL, 0) < 0)
        die("waitpid");
    return 0;
}
