/*
 * 03_named_sem_parent.c - a named semaphore shared by separate programs.
 *
 * Run:   ./03_named_sem_parent      (from this directory; it runs ./03_named_sem_child)
 *
 * Look:  the parent creates the semaphore with sem_open(O_CREAT | O_EXCL)
 *        and then fork()s and exec()s NCHILD children. exec replaces the
 *        children's memory, so an unnamed semaphore could not survive it.
 *        The children find this semaphore again by its name. In the output,
 *        no child enters the critical section until the previous one has
 *        left it.
 *
 *        The name stays until sem_unlink(). If an earlier run was
 *        interrupted, O_EXCL makes sem_open() fail with "File exists";
 *        remove the leftover with  rm /dev/shm/sem.k22_named_sem
 */
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"
#include "named_sem.h"

int main(void)
{
    sem_t *sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0600, 1);
    if (sem == SEM_FAILED)
        die("sem_open " SEM_NAME);
    /* The parent does not use the semaphore itself. */
    sem_close(sem);

    for (int i = 0; i < NCHILD; i++) {
        pid_t pid = fork();
        if (pid < 0)
            die("fork");
        if (pid == 0) {
            char id[8];
            snprintf(id, sizeof(id), "%d", i);
            execl(CHILD_PROG, CHILD_PROG, id, (char *) NULL);
            die("execl " CHILD_PROG);
        }
    }
    while (wait(NULL) > 0)
        ;

    if (sem_unlink(SEM_NAME) < 0)
        die("sem_unlink");
    printf("Parent: all children done, %s removed\n", SEM_NAME);
    return 0;
}
