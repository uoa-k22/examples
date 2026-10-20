/* 08-threads.c - checks that the programs in 08-threads do what they say. */
#include "run.h"

static void create_join(void)
{
    run("./01_create_join");
    expect("^thread: hello ");
    expect("the thread has finished$");
    /* main() and the thread print the same pid. */
    const char *pids = values("\\(pid ([0-9]+)\\)$");
    long a = 0, b = -1;
    TEST_CHECK_(sscanf(pids, "%ld %ld", &a, &b) == 2 && a == b,
                "the same pid twice (found \"%s\")", pids);
    SHOW_OUTPUT();
}

static void many_threads(void)
{
    run("./02_many_threads");
    for (int i = 0; i < 4; i++) {
        char re[64];
        snprintf(re, sizeof(re), "^thread %d: pid [0-9]+, tid [0-9]+$", i);
        expect_count(1, re);
    }
    expect("all 4 threads have finished$");
}

static void shared_globals(void)
{
    run("./03_shared_globals");
    expect_values("^[a-z]+: +global = ([0-9]+)", "100 101 101");
    expect_values("local = ([0-9]+)", "200 201 201");
}

static void return_value(void)
{
    run("./04_return_value");
    expect_count(4, "^thread [0-3]: ");
    expect("^main: total 8000002000000, expected 8000002000000$");
}

static void shared_arg_bug(void)
{
    run("./05_shared_arg_bug");
    expect_values("my number is ([0-9]+)$", "3 3 3 3");
}

static void shared_arg_fixed(void)
{
    run("./05_shared_arg_fixed");
    expect_count(1, "my number is 0$");
    expect_count(1, "my number is 1$");
    expect_count(1, "my number is 2$");
    expect_count(1, "my number is 3$");
}

static void race(void)
{
    run("./06_race");
    expect("^expected 2000000, got [0-9]+ ");
    reject("\\(0 updates lost\\)");
}

static void mutex(void)
{
    run("./07_mutex");
    expect("\\(0 updates lost\\)$");
}

static void no_join_bug(void)
{
    run("./08_no_join_bug");
    expect("^main: returning$");
    reject(": done$");
}

static void no_join_fixed(void)
{
    run("./08_no_join_fixed");
    expect_count(4, "^thread [0-3]: done$");
    expect("^main: returning$");
}

static void join_in_loop_bug(void)
{
    run("./09_join_in_loop_bug");
    expect_values("^thread ([0-9]): done$", "0 1 2 3");
    expect("all done after 4\\.[0-9] seconds$");
}

static void join_in_loop_fixed(void)
{
    run("./09_join_in_loop_fixed");
    expect_count(4, "^thread [0-3]: done$");
    expect("all done after 1\\.[0-9] seconds$");
}

TEST_LIST = {
    { "01_create_join", create_join },
    { "02_many_threads", many_threads },
    { "03_shared_globals", shared_globals },
    { "04_return_value", return_value },
    { "05_shared_arg_bug", shared_arg_bug },
    { "05_shared_arg_fixed", shared_arg_fixed },
    { "06_race", race },
    { "07_mutex", mutex },
    { "08_no_join_bug", no_join_bug },
    { "08_no_join_fixed", no_join_fixed },
    { "09_join_in_loop_bug", join_in_loop_bug },
    { "09_join_in_loop_fixed", join_in_loop_fixed },
    { NULL, NULL }
};
