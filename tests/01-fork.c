/* 01-fork.c - checks that the programs in 01-fork do what they say. */
#include <stdlib.h>
#include <unistd.h>
#include "run.h"

static void simple_fork(void)
{
    run("./01_simple_fork");
    expect_count(1, "^before fork");
    expect("^child: +fork\\(\\) returned 0,");
    expect("^parent: fork\\(\\) returned [1-9]");
}

static void address_space(void)
{
    run("./02_address_space");
    expect("^child: +global = 101 .* local = 201 ");
    expect("^parent: +global = 100 .* local = 200 ");
}

static void shared_fd(void)
{
    run("./03_shared_fd");
    run("cat shared_fd.txt");
    expect_values("^(before|child|parent) wrote", "before child parent");
}

static void orphan(void)
{
    run("./04_orphan");
    long old = atol(values("my parent is ([0-9]+)$"));
    long now = atol(values("my parent is now ([0-9]+)$"));
    TEST_CHECK_(now != 0 && now != old, "the child was adopted");
    SHOW_OUTPUT();
}

static void fork_loop_bug(void)
{
    run("./05_fork_loop_bug");
    expect_count(7, "^i = [0-9]: child");
}

static void fork_loop_fixed(void)
{
    char re[64];

    run("./05_fork_loop_fixed");
    expect_count(3, "^i = [0-9]: child");
    /* All three are children of the same parent. */
    snprintf(re, sizeof(re), "\\(parent %ld\\)",
             atol(values("\\(parent ([0-9]+)\\)")));
    expect_count(3, re);
}

static void stdio_buffer_bug(void)
{
    run("./06_stdio_buffer_bug");
    expect_count(2, "printed once, before fork");
}

static void stdio_buffer_fixed(void)
{
    run("./06_stdio_buffer_fixed");
    expect_count(1, "printed once, before fork");
}

static void separate_open_bug(void)
{
    run("./07_separate_open_bug");
    expect_count(3, "line [1-3]$");
}

static void separate_open_fixed(void)
{
    run("./07_separate_open_fixed");
    expect_count(6, "line [1-3]$");
}

static void fork_limit_bug(void)
{
    if (getuid() == 0)
        TEST_SKIP("the process limit does not apply to root");
    run("./08_fork_limit_bug");
    expect("^fork\\(\\) returned -1$");
    expect("child -1 finished with status 0");
}

static void fork_limit_fixed(void)
{
    if (getuid() == 0)
        TEST_SKIP("the process limit does not apply to root");
    run("./08_fork_limit_fixed");
    expect_status(1);
    expect("^fork: Resource temporarily unavailable$");
}

TEST_LIST = {
    { "01_simple_fork", simple_fork },
    { "02_address_space", address_space },
    { "03_shared_fd", shared_fd },
    { "04_orphan", orphan },
    { "05_fork_loop_bug", fork_loop_bug },
    { "05_fork_loop_fixed", fork_loop_fixed },
    { "06_stdio_buffer_bug", stdio_buffer_bug },
    { "06_stdio_buffer_fixed", stdio_buffer_fixed },
    { "07_separate_open_bug", separate_open_bug },
    { "07_separate_open_fixed", separate_open_fixed },
    { "08_fork_limit_bug", fork_limit_bug },
    { "08_fork_limit_fixed", fork_limit_fixed },
    { NULL, NULL }
};
