/* 05-mmap.c - checks that the programs in 05-mmap do what they say. */
#include <stdlib.h>
#include <sys/stat.h>
#include "run.h"

static void anon_alloc(void)
{
    run("./01_anon_alloc");
    expect("\\(page size is [0-9]+ bytes\\)$");
    expect("^Initial contents: 0 0 0 0 0$");
    expect("^After filling: +0 10 20 30 40$");
}

static void file_read(void)
{
    struct stat st;
    char re[64];

    run("./02_file_read 02_file_read.c");
    expect("^#include <sys/mman\\.h>$");       /* the file itself */
    TEST_ASSERT(stat("02_file_read.c", &st) == 0);
    snprintf(re, sizeof(re), "^--- 02_file_read\\.c: %ld bytes, ",
             (long) st.st_size);
    expect(re);
    run("./02_file_read /dev/null");
    expect("empty file, nothing to map$");
}

static void file_write(void)
{
    run("./03_file_write");
    run("cat mmap_demo.txt");
    expect("^This text was written into memory, not with write\\(\\)\\.$");
}

static void shared_anon_fork(void)
{
    run("./04_shared_anon_fork");
    expect("^parent : 10 20 30 40 50$");
}

static void private_anon_fork(void)
{
    run("./05_private_anon_fork");
    expect("^child  : 10 20 30 40 50$");
    expect("^parent : 1 2 3 4 5$");
}

/* The minor faults on the line that starts with step. */
static long faults(const char *step)
{
    char re[128];

    snprintf(re, sizeof(re), "^%s +minor faults +([0-9]+) ", step);
    return atol(values(re));
}

static void demand_paging(void)
{
    run("./06_demand_paging");
    long pages = atol(values("= ([0-9]+) pages$"));
    TEST_CHECK_(faults("after touching the same 1/4 again") ==
                faults("after touching 1/4 of the pages"),
                "touching the same pages again causes no faults");
    SHOW_OUTPUT();
    TEST_CHECK_(faults("after touching all the pages") -
                faults("after mmap\\(\\)") >= pages,
                "touching every page causes at least one fault per page");
    SHOW_OUTPUT();
}

static void page_protection(void)
{
    run("./07_mprotect");
    expect("reading it still works: \"hello\"$");
    expect("killed by signal 11 ");
}

static void private_not_saved_bug(void)
{
    run("./08_private_not_saved_bug");
    expect("mapping contains: \"new text\"$");
    expect("file contains: +\"old text\"$");
}

static void private_not_saved_fixed(void)
{
    run("./08_private_not_saved_fixed");
    expect("file contains: +\"new text\"$");
}

static void use_after_unmap_bug(void)
{
    run("./09_use_after_unmap_bug");
    expect("killed by signal 11 ");
}

static void use_after_unmap_fixed(void)
{
    run("./09_use_after_unmap_fixed");
    expect("child exited with status 0$");
}

TEST_LIST = {
    { "01_anon_alloc", anon_alloc },
    { "02_file_read", file_read },
    { "03_file_write", file_write },
    { "04_shared_anon_fork", shared_anon_fork },
    { "05_private_anon_fork", private_anon_fork },
    { "06_demand_paging", demand_paging },
    { "07_mprotect", page_protection },
    { "08_private_not_saved_bug", private_not_saved_bug },
    { "08_private_not_saved_fixed", private_not_saved_fixed },
    { "09_use_after_unmap_bug", use_after_unmap_bug },
    { "09_use_after_unmap_fixed", use_after_unmap_fixed },
    { NULL, NULL }
};
