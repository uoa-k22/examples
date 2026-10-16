/*
 * 02_execvp.c - execvp() looks the program up in $PATH.
 *
 * Run:   ./02_execvp                run ps with a made-up argv[0]
 *        ./02_execvp date -u        run any command with its arguments
 *
 * Look:  with no arguments, ps is started with argv[0] = "my-own-name".
 *        argv[0] is only a convention: the kernel runs the file it found in
 *        $PATH, whatever argv[0] says. In ps's output the FILE column (the
 *        program that runs) shows "ps", while ARGV starts with "my-own-name".
 *
 *        With arguments, argv + 1 is already a NULL-terminated argument
 *        vector for the command, so it can go to execvp() as it is.
 */
#include <stdio.h>
#include <unistd.h>
#include "errexit.h"

int main(int argc, char *argv[])
{
    if (argc > 1) {
        execvp(argv[1], argv + 1);
        die(argv[1]);
    }

    char pid[16];
    snprintf(pid, sizeof(pid), "%d", (int) getpid());

    execlp("ps", "my-own-name", "-o", "pid,comm=FILE,args=ARGV", "-p", pid, (char *) NULL);
    die("execlp ps");
}
