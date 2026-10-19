/*
 * 04_orphan.c - a child whose parent exits first is adopted.
 *
 * Run:   ./04_orphan
 *
 * Look:  the parent exits after one second. The shell prints its prompt,
 *        and a second later the child's second line appears. By then its
 *        parent pid has changed: it was adopted by init (pid 1) or, on most
 *        desktops, by a "subreaper" such as systemd --user. Check which
 *        process that is with  ps -p <pid>
 */
#include <stdio.h>
#include <unistd.h>
#include "errexit.h"

int main(void)
{
    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        printf("child  %d: my parent is %d\n", (int) getpid(), (int) getppid());
        sleep(2);
        printf("child  %d: my parent is now %d\n",
               (int) getpid(), (int) getppid());
        return 0;
    }

    sleep(1);
    printf("parent %d: exiting without waiting for the child\n",
           (int) getpid());
    return 0;
}
