/* 08-sysv-ipc.c - checks that the programs in 08-sysv-ipc do what they
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
    TEST_ASSERT_(match_lines("removed segment [0-9]+$", NULL, 0) == 1,
                 "the program prints the segment it removed");
    int id = atoi(values("removed segment ([0-9]+)$"));
    TEST_CHECK_(!shm_exists(id), "segment %d is removed", id);
}

static void leaked_segment_bug(void)
{
    char cmd[64];

    run("./02_leaked_segment_bug");
    TEST_ASSERT_(match_lines("and segment [0-9]+ stays\\.$", NULL, 0) == 1,
                 "the program prints the segment it leaves");
    int id = atoi(values("and segment ([0-9]+) stays\\.$"));
    TEST_CHECK_(shm_exists(id), "segment %d stays", id);
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
    TEST_ASSERT_(match_lines("created segment [0-9]+$", NULL, 0) == 1,
                 "the program prints the segment it created");
    int id = atoi(values("created segment ([0-9]+)$"));
    TEST_CHECK_(!shm_exists(id), "segment %d is removed", id);
}

static void sysv_sem(void)
{
    run("./03_sysv_sem");
    expect_values(": (enter|leave)$", "enter leave enter leave enter leave "
                  "enter leave enter leave enter leave");
    TEST_ASSERT_(match_lines("removed semaphore set [0-9]+$", NULL, 0) == 1,
                 "the program prints the set it removed");
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

static void shm_messages(void)
{
    run("./05_shm_messages");
    expect_values("received \"(.*)\"$", "hello how are you? goodbye");
    expect("child: +got the empty message, stopping$");
    TEST_ASSERT_(match_lines("segment [0-9]+, removed", NULL, 0) == 1,
                 "the program prints its segment");
    int id = atoi(values("segment ([0-9]+), removed"));
    TEST_CHECK_(!shm_exists(id), "segment %d is removed", id);
}

static void ftok_key(void)
{
    char key[16];

    run("./06_ftok_writer 'a test message'");
    expect("stored \"a test message\"$");
    TEST_ASSERT_(match_lines("segment [0-9]+:", NULL, 0) == 1,
                 "the writer prints its key and segment");
    snprintf(key, sizeof(key), "%s", values("key (0x[0-9a-f]+),"));
    int id = atoi(values("segment ([0-9]+):"));
    TEST_CHECK_(shm_exists(id), "segment %d stays after the writer exits", id);
    run("./07_ftok_reader");
    expect("left the message \"a test message\"$");
    TEST_CHECK_(strcmp(values("key (0x[0-9a-f]+),"), key) == 0,
                "the reader computes the writer's key %s", key);
    TEST_CHECK_(!shm_exists(id), "segment %d is removed", id);
    run("./07_ftok_reader");                 /* the segment is gone now */
    expect_status(1);
    expect("No such file or directory$");
}

TEST_LIST = {
    { "01_sysv_shm", sysv_shm },
    { "02_leaked_segment_bug", leaked_segment_bug },
    { "02_leaked_segment_fixed", leaked_segment_fixed },
    { "03_sysv_sem", sysv_sem },
    { "04_owner_died_bug", owner_died_bug },
    { "04_owner_died_fixed", owner_died_fixed },
    { "05_shm_messages", shm_messages },
    { "06_ftok_writer+07_reader", ftok_key },
    { NULL, NULL }
};
