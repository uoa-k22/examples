/* 07-semaphores.c - checks that the programs in 07-semaphores do what
   they say. */
#include "run.h"

#define EL "enter leave"            /* one child at a time in the section */

static void race(void)
{
    run("./01_race");
    expect("^expected 2000000, got [0-9]+ ");
    reject("\\(0 updates lost\\)");
}

static void unnamed_sem(void)
{
    run("./02_unnamed_sem");
    expect("\\(0 updates lost\\)$");
}

static void named_sem(void)
{
    run("./03_named_sem_parent");
    expect_values(": (enter|leave)$",
                  EL " " EL " " EL " " EL " " EL " " EL " " EL " " EL " " EL);
    expect("/k22_named_sem removed$");
    expect_no_leftovers();
}

static void pingpong(void)
{
    run("./04_pingpong");
    expect_values("^[0-9]: +(ping|pong) ",
                  "ping pong ping pong ping pong ping pong ping pong");
}

static void bounded_buffer(void)
{
    run("./05_bounded_buffer");
    expect(": OK$");
    /* The count in "[n in buffer]" never goes above SLOTS = 4. */
    expect_count(0, "\\[([5-9]|[1-9][0-9]+) in buffer\\]");
}

static void sem_not_shared_bug(void)
{
    run("./06_sem_not_shared_bug");
    reject("\\(0 updates lost\\)");
}

static void sem_not_shared_fixed(void)
{
    run("./06_sem_not_shared_fixed");
    expect("\\(0 updates lost\\)$");
}

static void deadlock_bug(void)
{
    run("./07_deadlock_bug");
    expect_status(128 + 14);            /* the parent is killed by SIGALRM */
    reject("got A and B");
}

static void deadlock_fixed(void)
{
    run("./07_deadlock_fixed");
    expect_count(2, "got A and B, working$");
    expect("both finished$");
}

static void wrong_init_value_bug(void)
{
    run("./08_wrong_init_value_bug");
    expect_count(2, "killed by Alarm clock: it never got the lock$");
    expect("counter = 0, expected 2$");
}

static void wrong_init_value_fixed(void)
{
    run("./08_wrong_init_value_fixed");
    expect("counter = 2, expected 2$");
}

static void lock_leak_bug(void)
{
    run("./09_lock_leak_bug");
    expect("stuck waiting for the lock$");
}

static void lock_leak_fixed(void)
{
    run("./09_lock_leak_fixed");
    expect("update\\(7\\): total is now 7$");
    reject("Alarm clock");
}

static void producer_order_bug(void)
{
    run("./10_producer_order_bug");
    expect_count(2, "killed by SIGALRM: deadlock$");
    reject("consumer: took");
}

static void producer_order_fixed(void)
{
    run("./10_producer_order_fixed");
    expect_count(6, "consumer: took [0-9]$");
}

static void eintr_bug(void)
{
    run("./11_eintr_bug");
    expect_status(1);
    expect("sem_wait: Interrupted system call$");
}

static void eintr_fixed(void)
{
    run("./11_eintr_fixed");
    expect("the child is done after [0-9]+ interruptions$");
}

TEST_LIST = {
    { "01_race", race },
    { "02_unnamed_sem", unnamed_sem },
    { "03_named_sem_parent", named_sem },
    { "04_pingpong", pingpong },
    { "05_bounded_buffer", bounded_buffer },
    { "06_sem_not_shared_bug", sem_not_shared_bug },
    { "06_sem_not_shared_fixed", sem_not_shared_fixed },
    { "07_deadlock_bug", deadlock_bug },
    { "07_deadlock_fixed", deadlock_fixed },
    { "08_wrong_init_value_bug", wrong_init_value_bug },
    { "08_wrong_init_value_fixed", wrong_init_value_fixed },
    { "09_lock_leak_bug", lock_leak_bug },
    { "09_lock_leak_fixed", lock_leak_fixed },
    { "10_producer_order_bug", producer_order_bug },
    { "10_producer_order_fixed", producer_order_fixed },
    { "11_eintr_bug", eintr_bug },
    { "11_eintr_fixed", eintr_fixed },
    { NULL, NULL }
};
