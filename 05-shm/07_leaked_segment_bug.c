/*
 * 07_leaked_segment_bug.c - a System V segment that nobody removes.
 *
 * Run:   ./07_leaked_segment_bug     leaves a segment behind
 *        ipcs -m                     list the segments on the machine
 *        The fix is in 07_leaked_segment_fixed.c. To see what changed:
 *        diff 07_leaked_segment_bug.c 07_leaked_segment_fixed.c
 *
 * Look:  the parent creates a segment, a child writes into it, the parent
 *        reads it. Then every process has detached and the program ends.
 *        That is all: shmctl(IPC_RMID) is never called.
 *        The segment is still there. It has no attached processes ("nattch"
 *        0), but it keeps its memory. Ending the program does not remove it.
 *        Only  ipcrm  or a reboot does, or, on most systemd-based systems
 *        (logind's RemoveIPC=yes), your last logout. That last rescue does not
 *        apply to root or to system users, so a server that leaks segments
 *        keeps leaking until someone notices. Run this program five times
 *        and  ipcs -m  shows five segments.
 *
 *        POSIX shared memory leaks the same way if shm_unlink() is
 *        forgotten (see 05-shm/README.md), but a POSIX object at least
 *        shows up as a file in /dev/shm.
 *
 *        Clean up afterwards with the  ipcrm -m <shmid>  line the
 *        program prints.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
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
    printf("Parent: created segment %d\n", shmid);
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
    printf("Parent: read \"%s\"\n", mem);
    shmdt(mem);

    printf("\nAll processes have detached. The segment:\n");
    show(shmid);

    /* BUG: no shmctl(IPC_RMID), so the segment outlives the program. */
    printf("\nThe program ends here, and segment %d stays.\n"
           "Remove it with:  ipcrm -m %d\n", shmid, shmid);
    return 0;
}
