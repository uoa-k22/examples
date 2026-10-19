/*
 * 04_owner_died_bug.c - a process dies while it holds a semaphore.
 *
 * Run:   ./04_owner_died_bug
 *        The fix is in 04_owner_died_fixed.c. To see what changed:
 *        diff 04_owner_died_bug.c 04_owner_died_fixed.c
 *
 * Look:  the child locks a System V semaphore (value 1 -> 0) and is then
 *        killed before it can unlock it, as if it had crashed or someone had
 *        run  kill -9  on it. The parent waits until the child is gone and
 *        then tries the lock without blocking (IPC_NOWAIT). The value is
 *        still 0: a semaphore does not know who decremented it, so nobody
 *        will ever post it again, and a process calling semop() without
 *        IPC_NOWAIT would block forever.
 *
 *        A POSIX semaphore (sem_wait) behaves the same, and has no
 *        equivalent of the fix.
 */
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

/* man semctl asks the caller to define this union; only .val is used. */
union semun {
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
};

int main(void)
{
    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    if (semid < 0)
        die("semget");
    union semun arg = { .val = 1 };
    if (semctl(semid, 0, SETVAL, arg) < 0)
        die("semctl SETVAL");

    pid_t pid = fork();
    if (pid < 0)
        die("fork");
    if (pid == 0) {
        /* BUG: no SEM_UNDO, so the kernel cannot give the lock back */
        struct sembuf op = { .sem_num = 0, .sem_op = -1, .sem_flg = 0 };
        /* Retry after EINTR, see 03_sysv_sem.c. */
        while (semop(semid, &op, 1) < 0)
            if (errno != EINTR)
                die("semop");
        printf("child %d: got the lock, now killed with SIGKILL\n",
               (int) getpid());
        fflush(stdout);
        raise(SIGKILL);     /* as if it had crashed */
    }

    /* The parent waits until the child is dead, then tries the lock. */
    int status;
    if (waitpid(pid, &status, 0) < 0)
        die("waitpid");
    if (WIFSIGNALED(status))
        printf("parent: child %d is dead (signal %d)\n",
               (int) pid, WTERMSIG(status));

    printf("parent: semaphore value is %d\n", semctl(semid, 0, GETVAL));
    struct sembuf try = { .sem_num = 0, .sem_op = -1, .sem_flg = IPC_NOWAIT };
    if (semop(semid, &try, 1) == 0)
        printf("parent: got the lock\n");
    else if (errno == EAGAIN)
        printf("parent: still locked, and nobody will ever unlock it;"
               " a blocking semop() would wait forever\n");
    else
        die("semop");

    if (semctl(semid, 0, IPC_RMID) < 0)
        die("semctl IPC_RMID");
    return 0;
}
