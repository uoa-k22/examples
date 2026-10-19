/*
 * 02_exit_status.c - reading a child's exit code with wait().
 *
 * Run:   ./02_exit_status          the child calls exit(3)
 *        ./02_exit_status 42       the child calls exit(42)
 *        ./02_exit_status 256      only the low 8 bits survive: 256 -> 0
 *
 * Look:  the status filled in by wait() is not the exit code itself. It
 *        packs several fields into one int, so use the macros to read it:
 *        WIFEXITED(status) is true if the child called exit() or returned
 *        from main, and then WEXITSTATUS(status) is the code it passed.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

int main(int argc, char *argv[])
{
    int code = argc > 1 ? atoi(argv[1]) : 3;

    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        printf("child %d: calling exit(%d)\n", (int) getpid(), code);
        exit(code);
    }

    int status;
    pid_t done = wait(&status);
    if (done < 0)
        die("wait");

    printf("parent: wait() returned %d\n", (int) done);
    printf("  raw status           = 0x%04x\n", (unsigned) status);
    printf("  WIFEXITED(status)    = %d\n", WIFEXITED(status));
    if (WIFEXITED(status))
        printf("  WEXITSTATUS(status)  = %d\n", WEXITSTATUS(status));
    return 0;
}
