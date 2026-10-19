/*
 * 06_stdio_buffer.c - why output printed before fork() can appear twice.
 *
 * Run:   ./06_stdio_buffer                 to the terminal
 *        ./06_stdio_buffer | cat           through a pipe
 *        ./06_stdio_buffer > out.txt; cat out.txt
 *
 * Look:  as written, every run prints the "before fork" line once.
 *
 * Try:   comment out the fflush(stdout) line marked below, run  make  and
 *        run all three commands again. On the terminal nothing changes, but
 *        through the pipe and into the file the "before fork" line now
 *        appears TWICE, both times with the parent's pid. The child never
 *        executed that printf(), so why does it print it? Notice also that
 *        each copy comes out right before that process's own line.
 *
 * Why:   printf() does not call write() at once. It copies the text into a
 *        buffer inside the process's memory (stdio buffering):
 *        - to a terminal, stdout is line buffered: the buffer is written
 *          at every '\n', so it is already empty when fork() runs.
 *        - to a pipe or file, stdout is fully buffered: the buffer is
 *          written only when it fills up (a few KB) or the process exits.
 *        fork() copies the whole address space, and the buffer with it. The
 *        unwritten line is now in both processes, and each one writes it
 *        out when it exits.
 *
 * Fix:   fflush(stdout) before fork(), so there is nothing left to copy.
 *        (A child that calls exec, or ends with _exit(), never writes its
 *        copy of the buffer; one that calls exit() or returns from main
 *        does.)
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

    fflush(stdout);     /* <-- comment out this line to see the bug */

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
