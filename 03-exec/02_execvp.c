/*
 * 02_execvp.c - execvp() looks the program up in $PATH.
 *
 * Run:   ./02_execvp
 *
 * Look:  the $PATH search is not done by the kernel. execvp() is a C
 *        library function that tries execve() with each directory in $PATH
 *        in turn (/usr/local/bin/ps, /usr/bin/ps, ...) until one succeeds.
 *        The kernel only ever sees a full path.
 *
 *        Here ps is started with argv[0] = "my-own-name". argv[0] is only
 *        a convention: the file that runs is the one found in $PATH,
 *        whatever argv[0] says. In ps's output the FILE column (the
 *        program that runs) shows "ps", while ARGV starts with "my-own-name".
 *        Try  strace -f -e trace=execve ./02_execvp  to watch the search.
 */
#include <stdio.h>
#include <unistd.h>
#include "errexit.h"

int main(void)
{
    char pid[16];
    snprintf(pid, sizeof(pid), "%d", (int) getpid());

    char *args[] = { "my-own-name", "-o", "pid,comm=FILE,args=ARGV",
                     "-p", pid, NULL };
    execvp("ps", args);
    die("execvp ps");
}
