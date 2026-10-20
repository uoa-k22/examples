/* 06-shm.c - checks that the programs in 06-shm do what they say. */
#include <unistd.h>
#include "run.h"

static void posix_shm(void)
{
    run("./01_posix_shm_writer 'a test message'");
    expect("stored \"a test message\" in /k22_shm_demo$");
    TEST_CHECK_(access("/dev/shm/k22_shm_demo", F_OK) == 0,
                "the object is in /dev/shm after the writer exits");
    run("./02_posix_shm_reader");
    expect("left the message \"a test message\"$");
    expect("removed /k22_shm_demo$");
    run("./02_posix_shm_reader");       /* the object is gone now */
    expect_status(1);
    expect("No such file or directory$");
    expect_no_leftovers();
}

static void shm_pointers_bug(void)
{
    run("./03_shm_pointers_bug");
    expect("^writer: list: 10 20 30 40$");
    reject("^reader: list: 10 20 30 40$");
    expect_no_leftovers();
}

static void shm_pointers_fixed(void)
{
    run("./03_shm_pointers_fixed");
    expect("^reader: list: 10 20 30 40$");
    expect_no_leftovers();
}

static void no_ftruncate_bug(void)
{
    run("./04_no_ftruncate_bug");
    expect("the object's size is 0 bytes$");
    expect("killed by signal 7 ");
    expect_no_leftovers();
}

static void no_ftruncate_fixed(void)
{
    run("./04_no_ftruncate_fixed");
    expect("stored \"hello\"$");
    expect("child exited with status 0$");
    expect_no_leftovers();
}

static void read_before_write_bug(void)
{
    run("./05_read_before_write_bug");
    expect("^reader: the message is \"\"$");
    expect_no_leftovers();
}

static void read_before_write_fixed(void)
{
    run("./05_read_before_write_fixed");
    expect("^reader: the message is \"hello, the data is ready\"$");
    expect_no_leftovers();
}

static void mutex_not_shared_bug(void)
{
    run("./06_mutex_not_shared_bug");
    expect("was killed by SIGALRM: stuck$");
    reject("\\(0 updates lost\\)");
    expect_no_leftovers();
}

static void mutex_not_shared_fixed(void)
{
    run("./06_mutex_not_shared_fixed");
    expect_count(2, "finished$");
    expect("\\(0 updates lost\\)$");
    expect_no_leftovers();
}

TEST_LIST = {
    { "01_posix_shm_writer+02_reader", posix_shm },
    { "03_shm_pointers_bug", shm_pointers_bug },
    { "03_shm_pointers_fixed", shm_pointers_fixed },
    { "04_no_ftruncate_bug", no_ftruncate_bug },
    { "04_no_ftruncate_fixed", no_ftruncate_fixed },
    { "05_read_before_write_bug", read_before_write_bug },
    { "05_read_before_write_fixed", read_before_write_fixed },
    { "06_mutex_not_shared_bug", mutex_not_shared_bug },
    { "06_mutex_not_shared_fixed", mutex_not_shared_fixed },
    { NULL, NULL }
};
