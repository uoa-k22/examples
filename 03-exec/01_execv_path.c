/*
 * 01_execv_path.c - execv() replaces the running program.
 *
 * Run:   ./01_execv_path
 *
 * Look:  execv() takes a path to the program file and does not search
 *        $PATH. The first call, with just "ls", fails (unless this directory
 *        happens to contain a file named ls). The second call, with
 *        "/bin/ls", succeeds: the process keeps its pid, but its code
 *        becomes ls. Nothing after a successful exec runs, so the last
 *        printf() is never reached.
 */
#include <stdio.h>
#include <unistd.h>
#include "errexit.h"

int main(void)
{
    char *args[] = { "ls", "-l", NULL };     /* args[0] is the program's name */

    printf("pid %d: calling execv(\"ls\", ...)\n", (int) getpid());
    fflush(stdout);               /* so this line comes out before perror's */
    execv("ls", args);
    perror("execv(\"ls\")");     /* exec returns only if it failed */

    printf("pid %d: calling execv(\"/bin/ls\", ...)\n", (int) getpid());
    fflush(stdout);               /* exec discards anything still buffered */
    execv("/bin/ls", args);
    die("execv(\"/bin/ls\")");

    printf("This line is never printed\n");
}
