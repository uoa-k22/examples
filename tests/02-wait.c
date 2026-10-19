/* 02-wait.c - checks that the programs in 02-wait do what they say. */
#include "run.h"

static void wait_no_children(void)
{
    run("./01_wait_no_children");
    expect("^wait\\(\\) returned -1$");
    expect("\\(No child processes\\)");
    expect("\\(unchanged\\)");
}

static void exit_status(void)
{
    run("./02_exit_status");
    expect("WEXITSTATUS\\(status\\) += 3$");
    run("./02_exit_status 42");
    expect("WEXITSTATUS\\(status\\) += 42$");
    run("./02_exit_status 256");        /* only the low 8 bits survive */
    expect("WEXITSTATUS\\(status\\) += 0$");
}

static void killed_by_signal(void)
{
    run("./03_killed_by_signal");
    expect_count(3, "WIFEXITED += 0$");
    expect_values("WTERMSIG += ([0-9]+)", "11 6 15");
}

static void zombie(void)
{
    run("./04_zombie");
    expect_count(1, " Z +04_zombie");   /* and gone after waitpid() */
    expect("collected exit status 2;");
}

static void wait_order(void)
{
    run("./05_wait_order");
    expect_values("^wait\\(\\) returned child ([0-9])", "2 1 0");
    expect("returned -1 with ECHILD");
}

static void waitpid_order(void)
{
    run("./06_waitpid_order");
    expect_values("^waitpid\\(\\) collected child ([0-9])", "0 1 2");
}

static void wnohang(void)
{
    run("./07_wnohang");
    expect("still running, doing step 1 ");
    expect("finished with status 7$");
}

static void wexitstatus_unchecked_bug(void)
{
    run("./08_wexitstatus_unchecked_bug");
    expect("exit code 0, the child succeeded");
}

static void wexitstatus_unchecked_fixed(void)
{
    run("./08_wexitstatus_unchecked_fixed");
    expect("killed by signal 9 \\(Killed\\), so it failed");
}

TEST_LIST = {
    { "01_wait_no_children", wait_no_children },
    { "02_exit_status", exit_status },
    { "03_killed_by_signal", killed_by_signal },
    { "04_zombie", zombie },
    { "05_wait_order", wait_order },
    { "06_waitpid_order", waitpid_order },
    { "07_wnohang", wnohang },
    { "08_wexitstatus_unchecked_bug", wexitstatus_unchecked_bug },
    { "08_wexitstatus_unchecked_fixed", wexitstatus_unchecked_fixed },
    { NULL, NULL }
};
