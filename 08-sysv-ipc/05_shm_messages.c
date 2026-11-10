/*
 * 05_shm_messages.c - semaphores and messages stored in one System V
 * shared memory segment.
 *
 * Run:   ./05_shm_messages
 *
 * Look:  the parent sends messages to its child through one segment. The
 *        segment holds a struct with everything the two processes share:
 *          empty  (starts at 1)  the slot is free; the sender waits on it
 *          full   (starts at 0)  the slot has a message; the receiver
 *                                waits on it
 *          text                  the message itself
 *        Each message is received exactly once, in the order it was sent.
 *        An empty message tells the child to stop.
 *
 *        System V semaphores (03_sysv_sem) live in the kernel, so they
 *        cannot be stored in memory. Unnamed POSIX semaphores can: here
 *        sem_init() with pshared = 1 sets them up inside the segment, as
 *        07-semaphores/04_pingpong.c does in a POSIX shared memory object.
 *
 *        The parent calls shmctl(IPC_RMID) right after shmat(). On Linux
 *        the segment stays usable while it is attached, and the kernel
 *        removes it when the last process detaches or exits. So nothing is
 *        left behind, even if the program is killed; check with  ipcs -m
 */
#include <semaphore.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define LEN 64

struct shared {
    sem_t empty;
    sem_t full;
    char text[LEN];
};

static void P(sem_t *sem) { if (sem_wait(sem) < 0) die("sem_wait"); }
static void V(sem_t *sem) { if (sem_post(sem) < 0) die("sem_post"); }

int main(void)
{
    const char *messages[] = { "hello", "how are you?", "goodbye", "" };

    setvbuf(stdout, NULL, _IOLBF, 0);

    int shmid = shmget(IPC_PRIVATE, sizeof(struct shared), IPC_CREAT | 0600);
    if (shmid < 0)
        die("shmget");
    struct shared *s = shmat(shmid, NULL, 0);
    if (s == (void *) -1)
        die("shmat");
    if (shmctl(shmid, IPC_RMID, NULL) < 0)
        die("shmctl IPC_RMID");
    printf("parent: segment %d, removed once nobody has it attached\n",
           shmid);
    if (sem_init(&s->empty, 1, 1) < 0 || sem_init(&s->full, 1, 0) < 0)
        die("sem_init");

    pid_t pid = fork();                 /* the child inherits the attach */
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        for (;;) {
            P(&s->full);
            char text[LEN];
            snprintf(text, sizeof(text), "%s", s->text);
            V(&s->empty);
            if (text[0] == '\0')
                break;
            printf("    child:  received \"%s\"\n", text);
        }
        printf("    child:  got the empty message, stopping\n");
        shmdt(s);
        return 0;
    }

    for (size_t i = 0; i < sizeof(messages) / sizeof(messages[0]); i++) {
        P(&s->empty);
        snprintf(s->text, sizeof(s->text), "%s", messages[i]);
        printf("parent: sent     \"%s\"\n", messages[i]);
        V(&s->full);
    }
    if (waitpid(pid, NULL, 0) < 0)
        die("waitpid");

    sem_destroy(&s->empty);
    sem_destroy(&s->full);
    shmdt(s);
    return 0;
}
