/*
 * 02_leaked_segment_fixed.c - the fix for 02_leaked_segment_bug.c.
 *
 * Run:   ./02_leaked_segment_fixed
 *        diff 02_leaked_segment_bug.c 02_leaked_segment_fixed.c
 *
 * Look:  the program ends with shmctl(IPC_RMID), and the second  ipcs -m
 *        listing no longer shows the segment. Nothing is left behind.
 *
 *        IPC_RMID may be called as soon as the last process has attached;
 *        the kernel then frees the segment after the last detach. Calling it
 *        early also means that a crash cannot leave the segment behind.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define SIZE 256

/* Show this segment's line from  ipcs -m  (the header lines come first). */
static void show(int shmid)
{
    char cmd[128];

    snprintf(cmd, sizeof(cmd), "ipcs -m | awk 'NR <= 3 || $2 == %d'", shmid);
    fflush(stdout);
    if (system(cmd) < 0)
        die("system");
}

int main(void)
{
    int shmid = shmget(IPC_PRIVATE, SIZE, IPC_CREAT | 0600);
    if (shmid < 0)
        die("shmget");
    printf("parent: created segment %d\n", shmid);
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0)
        die("fork");
    if (pid == 0) {
        char *mem = shmat(shmid, NULL, 0);
        if (mem == (void *) -1)
            die("shmat");
        snprintf(mem, SIZE, "written by child %d", (int) getpid());
        shmdt(mem);
        return 0;
    }
    if (waitpid(pid, NULL, 0) < 0)
        die("waitpid");

    char *mem = shmat(shmid, NULL, SHM_RDONLY);
    if (mem == (void *) -1)
        die("shmat");
    printf("parent: read \"%s\"\n", mem);
    shmdt(mem);

    printf("\nAll processes have detached. The segment:\n");
    show(shmid);

    if (shmctl(shmid, IPC_RMID, NULL) < 0)  /* FIX: remove the segment */
        die("shmctl IPC_RMID");
    printf("\nparent: called IPC_RMID. The segment now:\n");
    show(shmid);
    return 0;
}
