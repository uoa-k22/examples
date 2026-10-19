/*
 * 01_wait_no_children.c - wait() with no children to wait for.
 *
 * Run:   ./01_wait_no_children
 *
 * Look:  wait() does not block. It returns -1 straight away and sets errno
 *        to ECHILD ("No child processes"). The status variable is not
 *        touched. Loops that reap children use this return value to know
 *        when all of them are done:  while (wait(NULL) > 0) ;
 */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

int main(void)
{
    int status = 0x12345;

    pid_t rv = wait(&status);
    int err = errno;        /* save errno before printf() can change it */

    printf("wait() returned %d\n", (int) rv);
    printf("errno = %d (%s), ECHILD = %d\n", err, strerror(err), ECHILD);
    printf("status = 0x%x (unchanged)\n", (unsigned) status);
    return 0;
}
