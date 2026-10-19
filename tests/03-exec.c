/* 03-exec.c - checks that the programs in 03-exec do what they say. */
#include "run.h"

static void execv_path(void)
{
    run("./01_execv_path");
    expect("^execv\\(\"ls\"\\): No such file or directory$");
    expect("01_execv_path\\.c$");      /* the output of /bin/ls */
    reject("never printed");
}

static void execvp_lookup(void)
{
    run("./02_execvp");
    expect(" ps +my-own-name ");       /* FILE is ps, ARGV my-own-name */
}

static void fork_exec_wait(void)
{
    run("./03_fork_exec_wait ls -d /");
    expect("^\\[ls \\(pid [0-9]+\\) exited with status 0\\]$");
    run("./03_fork_exec_wait false");
    expect("exited with status 1\\]$");
    run("./03_fork_exec_wait no-such-command");
    expect("exited with status 127\\]$");
    run("./03_fork_exec_wait sh -c 'kill $$'");
    expect("was killed by signal 15:");
}

static void env(void)
{
    run("./04_env");
    expect("^COURSE=K22$");
    expect("^TOPIC=exec$");
    reject("^(HOME|PATH)=");
}

static void exec_no_exit_bug(void)
{
    run("./05_exec_no_exit_bug");
    expect_count(2, "parent, finished$");
    expect("exited with status 0$");
}

static void exec_no_exit_fixed(void)
{
    run("./05_exec_no_exit_fixed");
    expect_count(1, "parent, finished$");
    expect("exited with status 127$");
}

static void fd_inherit(void)
{
    run("./06_fd_inherit");
    expect(" 3 -> .*/kept\\.txt$");
    reject("-> .*/closed_on_exec\\.txt$");
    expect("4: Bad file descriptor$");
    run("cat kept.txt");
    expect("^written by sh through fd 3$");
}

static void self_exec_bug(void)
{
    run("./07_self_exec_bug");
    expect("No such file or directory$");
    expect("\\(status 127\\)$");
}

static void self_exec_fixed(void)
{
    run("./07_self_exec_fixed");
    expect("new copy running");
    expect("the child ran and exited normally$");
}

static void exec_errors_bug(void)
{
    run("./08_exec_errors_bug");
    expect("\\(ENOENT\\)$");
    expect("\\(EACCES\\)$");
    expect("\\(ENOEXEC\\)$");
    expect_count(1, "^  worked$");     /* only execvp() runs the script */
}

static void exec_errors_fixed(void)
{
    run("./08_exec_errors_fixed");
    expect_count(3, "^  worked$");
    reject("failed");
}

TEST_LIST = {
    { "01_execv_path", execv_path },
    { "02_execvp", execvp_lookup },
    { "03_fork_exec_wait", fork_exec_wait },
    { "04_env", env },
    { "05_exec_no_exit_bug", exec_no_exit_bug },
    { "05_exec_no_exit_fixed", exec_no_exit_fixed },
    { "06_fd_inherit", fd_inherit },
    { "07_self_exec_bug", self_exec_bug },
    { "07_self_exec_fixed", self_exec_fixed },
    { "08_exec_errors_bug", exec_errors_bug },
    { "08_exec_errors_fixed", exec_errors_fixed },
    { NULL, NULL }
};
