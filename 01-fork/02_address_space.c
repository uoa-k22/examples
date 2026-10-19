/*
 * 02_address_space.c - parent and child have separate address spaces.
 *
 * Run:   ./02_address_space
 *
 * Look:  the child changes a global and a local variable. The parent waits
 *        for the child and then prints the same variables: they still hold
 *        the old values. The addresses printed are the same in both
 *        processes because they are virtual addresses. Each process maps
 *        them to its own physical memory (copy-on-write).
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

int global = 100;

int main(void)
{
    int local = 200;

    printf("before fork: global = %d (at %p), local = %d (at %p)\n",
           global, (void *) &global, local, (void *) &local);
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        global++;
        local++;
        printf("child:       global = %d (at %p), local = %d (at %p)\n",
               global, (void *) &global, local, (void *) &local);
        return 0;
    }

    if (wait(NULL) < 0)
        die("wait");
    printf("parent:      global = %d (at %p), local = %d (at %p)\n",
           global, (void *) &global, local, (void *) &local);
    return 0;
}
