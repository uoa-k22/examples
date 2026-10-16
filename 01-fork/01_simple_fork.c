/*
 * 01_simple_fork.c - fork() makes a copy of the calling process.
 *
 * Run:   ./01_simple_fork
 *
 * Look:  "Before fork" is printed once, but the line after fork() is printed
 *        twice: once by the parent and once by the child. fork() returns the
 *        child's pid to the parent and 0 to the child. Which of the two lines
 *        comes first is up to the scheduler.
 *
 * Try:   comment out the fflush() and run  ./01_simple_fork | cat
 *        When stdout is a pipe, printf() buffers its output. The buffer is
 *        copied by fork(), so "Before fork" comes out twice.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

int main(void)
{
    printf("Before fork: pid = %d\n", (int) getpid());
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        printf("Child:  fork() returned %d, my pid = %d, my parent = %d\n",
               (int) pid, (int) getpid(), (int) getppid());
    } else {
        printf("Parent: fork() returned %d, my pid = %d, my parent = %d\n",
               (int) pid, (int) getpid(), (int) getppid());
        /* Wait for the child to finish (see 02-wait). */
        if (wait(NULL) < 0)
            die("wait");
    }
    return 0;
}
