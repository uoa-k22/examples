/*
 * 06_stdio_buffer_bug.c - output printed before fork() appears twice.
 *
 * Run:   ./06_stdio_buffer_bug                 to the terminal
 *        ./06_stdio_buffer_bug | cat           through a pipe
 *        ./06_stdio_buffer_bug > out.txt; cat out.txt
 *        The fix is in 06_stdio_buffer_fixed.c. To see what changed:
 *        diff 06_stdio_buffer_bug.c 06_stdio_buffer_fixed.c
 *
 * Look:  on the terminal the "before fork" line appears once. Through the
 *        pipe and into the file it appears TWICE, both times with the
 *        parent's pid. The child never executed that printf(), so why does
 *        it print it? Notice also that each copy comes out right before that
 *        process's own line.
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
 */
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

int main(void)
{
    printf("pid %d: this line is printed once, before fork()\n",
           (int) getpid());
    /* BUG: the line may still be in the stdio buffer when fork() copies it */

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
