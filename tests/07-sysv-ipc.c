/* 07-sysv-ipc.c - checks that the programs in 07-sysv-ipc do what they
   say, and that only 02_leaked_segment_bug leaves anything behind. */
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include "run.h"

static int shm_exists(int id)
{
    struct shmid_ds ds;

    return shmctl(id, IPC_STAT, &ds) == 0;
}

static int sem_exists(int id)
{
    return semctl(id, 0, GETVAL) >= 0;
}

/* How many semaphore sets this user owns (column 5 of the file is uid). */
static int sem_sets(void)
{
    char line[256];
    unsigned uid;
    int n = 0;
    FILE *f = fopen("/proc/sysvipc/sem", "r");

    TEST_CHECK_(f != NULL, "open /proc/sysvipc/sem");
    if (f == NULL)
        return -1;
    while (fgets(line, sizeof(line), f))
        if (sscanf(line, "%*d %*d %*o %*u %u", &uid) == 1 && uid == getuid())
            n++;
    fclose(f);
    return n;
}

static void sysv_shm(void)
{
    run("./01_sysv_shm");
    expect("read \"greetings from child [0-9]+\"");
    int id = atoi(values("removed segment ([0-9]+)$"));
    TEST_CHECK_(id > 0 && !shm_exists(id), "segment %d is removed", id);
}

static void leaked_segment_bug(void)
{
    char cmd[64];

    run("./02_leaked_segment_bug");
    int id = atoi(values("and segment ([0-9]+) stays\\.$"));
    TEST_CHECK_(id > 0 && shm_exists(id), "segment %d stays", id);
    SHOW_OUTPUT();
    /* Remove it with the command the program prints. */
    snprintf(cmd, sizeof(cmd), "%s", values("^Remove it with: +(.*)$"));
    run(cmd);
    TEST_CHECK_(!shm_exists(id), "%s removes segment %d", cmd, id);
}

static void leaked_segment_fixed(void)
{
    run("./02_leaked_segment_fixed");
    expect("called IPC_RMID");
    int id = atoi(values("created segment ([0-9]+)$"));
    TEST_CHECK_(id > 0 && !shm_exists(id), "segment %d is removed", id);
}

static void sysv_sem(void)
{
    run("./03_sysv_sem");
    expect_values(": (enter|leave)$", "enter leave enter leave enter leave "
                  "enter leave enter leave enter leave");
    int id = atoi(values("removed semaphore set ([0-9]+)$"));
    TEST_CHECK_(!sem_exists(id), "semaphore set %d is removed", id);
}

static void owner_died_bug(void)
{
    int before = sem_sets();

    run("./04_owner_died_bug");
    expect("semaphore value is 0$");
    expect("still locked, and nobody will ever unlock it");
    TEST_CHECK_(sem_sets() == before, "the semaphore set is removed");
}

static void owner_died_fixed(void)
{
    int before = sem_sets();

    run("./04_owner_died_fixed");
    expect("semaphore value is 1$");
    expect("parent: got the lock$");
    TEST_CHECK_(sem_sets() == before, "the semaphore set is removed");
}

TEST_LIST = {
    { "01_sysv_shm", sysv_shm },
    { "02_leaked_segment_bug", leaked_segment_bug },
    { "02_leaked_segment_fixed", leaked_segment_fixed },
    { "03_sysv_sem", sysv_sem },
    { "04_owner_died_bug", owner_died_bug },
    { "04_owner_died_fixed", owner_died_fixed },
    { NULL, NULL }
};
