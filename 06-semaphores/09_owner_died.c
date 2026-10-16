/*
 * 09_owner_died.c - a process dies while it holds a semaphore.
 *
 * Run:   ./09_owner_died          with a POSIX semaphore
 *        ./09_owner_died sysv     with a System V semaphore and SEM_UNDO
 *
 * Look:  the child locks the semaphore (value 1 -> 0) and is then killed
 *        before it can unlock it, as if it had crashed or someone had run
 *        kill -9 on it. The parent waits until the child is gone and then
 *        tries the lock without blocking (sem_trywait / IPC_NOWAIT):
 *        - POSIX: the value is still 0. The semaphore does not know who
 *          decremented it, so nobody will ever post it again. A process
 *          calling sem_wait() would block forever.
 *        - System V with SEM_UNDO: the kernel kept a record of the child's
 *          -1 and reversed it when the child died. The value is back to 1
 *          and the parent gets the lock at once.
 *
 *        A semaphore is only a counter, not an owner. When you need to
 *        recover from a dead lock holder, use SEM_UNDO, or a "robust"
 *        pthread mutex (pthread_mutexattr_setrobust), whose next locker
 *        is told EOWNERDEAD.
 */
#include <errno.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

union semun {
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
};

/* Fork a child that runs lock() and then is killed; wait until it is gone. */
static void child_locks_and_dies(void (*lock)(void))
{
    pid_t pid = fork();
    if (pid < 0)
        die("fork");
    if (pid == 0) {
        lock();
        printf("child %d: got the lock, now killed with SIGKILL\n", (int) getpid());
        fflush(stdout);
        raise(SIGKILL);
    }

    int status;
    if (waitpid(pid, &status, 0) < 0)
        die("waitpid");
    if (WIFSIGNALED(status))
        printf("parent: child %d is dead (signal %d)\n", (int) pid, WTERMSIG(status));
}

/* --- POSIX semaphore ------------------------------------------------------ */

static sem_t *posix_sem;

static void posix_lock(void)
{
    if (sem_wait(posix_sem) < 0)
        die("sem_wait");
}

static void posix_demo(void)
{
    posix_sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (posix_sem == MAP_FAILED)
        die("mmap");
    if (sem_init(posix_sem, 1, 1) < 0)
        die("sem_init");

    child_locks_and_dies(posix_lock);

    int value;
    sem_getvalue(posix_sem, &value);
    printf("parent: semaphore value is %d\n", value);
    if (sem_trywait(posix_sem) == 0)
        printf("parent: got the lock\n");
    else if (errno == EAGAIN)
        printf("parent: sem_trywait: locked, and nobody will ever unlock it;"
               " sem_wait() would block forever\n");
    else
        die("sem_trywait");

    sem_destroy(posix_sem);
}

/* --- System V semaphore with SEM_UNDO ------------------------------------- */

static int semid;

static void sysv_lock(void)
{
    struct sembuf op = { .sem_num = 0, .sem_op = -1, .sem_flg = SEM_UNDO };
    while (semop(semid, &op, 1) < 0)    /* retry after EINTR, see 06_sysv_sem.c */
        if (errno != EINTR)
            die("semop");
}

static void sysv_demo(void)
{
    semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    if (semid < 0)
        die("semget");
    union semun arg = { .val = 1 };
    if (semctl(semid, 0, SETVAL, arg) < 0)
        die("semctl SETVAL");

    child_locks_and_dies(sysv_lock);

    printf("parent: semaphore value is %d\n", semctl(semid, 0, GETVAL));
    struct sembuf op = { .sem_num = 0, .sem_op = -1, .sem_flg = IPC_NOWAIT | SEM_UNDO };
    if (semop(semid, &op, 1) == 0)
        printf("parent: got the lock: SEM_UNDO released the dead child's hold\n");
    else if (errno == EAGAIN)
        printf("parent: semop: still locked\n");
    else
        die("semop");

    if (semctl(semid, 0, IPC_RMID) < 0)
        die("semctl IPC_RMID");
}

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "sysv") == 0)
        sysv_demo();
    else
        posix_demo();
    return 0;
}
