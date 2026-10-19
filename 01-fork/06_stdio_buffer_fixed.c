/*
 * 06_stdio_buffer_fixed.c - the fix for 06_stdio_buffer_bug.c.
 *
 * Run:   ./06_stdio_buffer_fixed | cat
 *        diff 06_stdio_buffer_bug.c 06_stdio_buffer_fixed.c
 *
 * Look:  fflush(stdout) before fork() writes out whatever is in the buffer,
 *        so there is nothing left for fork() to copy. The "before fork" line
 *        appears once, on the terminal, through a pipe and into a file.
 *
 *        A child that calls exec, or ends with _exit(), never writes its
 *        copy of the buffer; one that calls exit() or returns from main
 *        does. So flush before fork(), and end a child with _exit() when it
 *        should not touch the parent's buffers.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

int main(void)
{
    printf("pid %d: this line is printed once, before fork()\n",
           (int) getpid());
    fflush(stdout);     /* FIX: empty the buffer before fork() copies it */

    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        printf("pid %d: child\n", (int) getpid());
        return 0;
    }

    if (wait(NULL) < 0)
        die("wait");
    printf("pid %d: parent\n", (int) getpid());
    return 0;
}
