/*
 * 03_sysv_shm.c - System V shared memory: shmget, shmat, shmdt, shmctl.
 *
 * Run:   ./03_sysv_shm
 *
 * Look:  this is the older API for the same idea. A segment is identified
 *        by a number (the shmid), not a name. IPC_PRIVATE always creates a
 *        new segment, so it suits a parent sharing with its own children.
 *        Unrelated processes would use a key from ftok() instead.
 *        - shmget()             creates the segment
 *        - shmat() / shmdt()    attach it to / detach it from memory
 *        - shmctl(IPC_RMID)     removes it
 *        Segments outlive their processes. A segment that is never
 *        removed stays until reboot (or, on most systemd-based systems,
 *        until its owner's last logout); list them with  ipcs -m  and
 *        remove one with  ipcrm -m <shmid>.
 */
#include <stdio.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define SIZE 256

int main(void)
{
    int shmid = shmget(IPC_PRIVATE, SIZE, IPC_CREAT | 0600);
    if (shmid < 0)
        die("shmget");
    printf("parent: created segment with shmid %d\n", shmid);
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        char *mem = shmat(shmid, NULL, 0);
        if (mem == (void *) -1)
            die("shmat");
        snprintf(mem, SIZE, "greetings from child %d", (int) getpid());
        printf("child:  wrote into the segment at %p\n", (void *) mem);
        shmdt(mem);
        return 0;
    }

    if (waitpid(pid, NULL, 0) < 0)
        die("waitpid");

    char *mem = shmat(shmid, NULL, SHM_RDONLY);
    if (mem == (void *) -1)
        die("shmat");
    printf("parent: read \"%s\" from the segment at %p\n", mem, (void *) mem);

    shmdt(mem);
    if (shmctl(shmid, IPC_RMID, NULL) < 0)
        die("shmctl IPC_RMID");
    printf("parent: removed segment %d\n", shmid);
    return 0;
}
