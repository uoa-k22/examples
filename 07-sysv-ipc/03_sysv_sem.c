/*
 * 03_sysv_sem.c - System V semaphores: semget(), semctl(), semop().
 *
 * Run:   ./03_sysv_sem
 *
 * Look:  the same mutual exclusion as 06-semaphores/03_named_sem_parent.c,
 *        with the older API that many existing programs still use. The
 *        differences:
 *        - semget() creates a whole *set* of semaphores; here it has one.
 *        - The new semaphore has no defined value until semctl(SETVAL)
 *          sets it.
 *        - semop() does both wait and post: sem_op = -1 is P (wait),
 *          sem_op = +1 is V (post).
 *        - SEM_UNDO: if a process dies while holding the semaphore, the
 *          kernel reverses its operations, so the others are not stuck.
 *        - A waiting semop() can fail with EINTR after Ctrl-Z / fg, so
 *          change() retries it. Try pressing Ctrl-Z and typing fg.
 *        - The set lives until semctl(IPC_RMID); see  ipcs -s
 */
#include <errno.h>
#include <stdio.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define NCHILD 3
#define ROUNDS 2

/* man semctl asks the caller to define this union; only .val is used. */
union semun {
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
};

static void change(int semid, int delta)
{
    struct sembuf op = { .sem_num = 0, .sem_op = delta, .sem_flg = SEM_UNDO };

    /* On Linux, a semop() that is waiting fails with EINTR if the process
       is stopped (Ctrl-Z) and continued (fg). That is not an error: retry. */
    while (semop(semid, &op, 1) < 0)
        if (errno != EINTR)
            die("semop");
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    if (semid < 0)
        die("semget");
    union semun arg = { .val = 1 };
    if (semctl(semid, 0, SETVAL, arg) < 0)
        die("semctl SETVAL");
    printf("parent: created semaphore set %d with value 1\n", semid);

    for (int i = 0; i < NCHILD; i++) {
        pid_t pid = fork();
        if (pid < 0)
            die("fork");
        if (pid == 0) {
            for (int r = 0; r < ROUNDS; r++) {
                change(semid, -1);                          /* P */
                printf("child %d (pid %d): enter\n", i, (int) getpid());
                usleep(200000);
                printf("child %d (pid %d): leave\n", i, (int) getpid());
                change(semid, +1);                          /* V */
                usleep(100000);
            }
            return 0;
        }
    }
    while (wait(NULL) > 0)
        ;

    if (semctl(semid, 0, IPC_RMID) < 0)
        die("semctl IPC_RMID");
    printf("parent: removed semaphore set %d\n", semid);
    return 0;
}
