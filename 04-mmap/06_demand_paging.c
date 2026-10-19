/*
 * 06_demand_paging.c - memory is given out one page at a time, on first use.
 *
 * Run:   ./06_demand_paging          map 512 MiB
 *        ./06_demand_paging 2048     map 2 GiB
 *
 * Look:  mmap() only reserves a range of virtual addresses. Memory is given
 *        out one page at a time, the first time the page is touched: the
 *        CPU finds no mapping, raises a page fault, the kernel allocates a
 *        zeroed page, and the instruction is retried. Such a fault is
 *        "minor": it needs no disk access. Follow the output line by line:
 *        1. after mmap(): resident memory (the RAM really in use) has
 *           hardly grown.
 *        2. after touching 1/4 of the pages: one more fault, and one more
 *           page of resident memory, for each page touched.
 *        3. touching the same 1/4 again: no new faults; they are mapped now.
 *        4. after touching all the pages: since mmap(), the faults have
 *           grown by the number of pages printed on the first line.
 *        5. after munmap(): all the pages are given back at once.
 *        The page size is 4 KiB on most PCs, but 16 KiB or 64 KiB on some
 *        ARM systems.
 *
 *        This is why a program can map far more memory than it uses, and
 *        why on Linux even a mapping bigger than RAM usually succeeds
 *        (overcommit). The trouble comes only if all of it is touched.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>
#include "errexit.h"

static long page_size;

/* Print the minor page faults so far and the resident memory now. */
static void report(const char *when)
{
    struct rusage ru;
    if (getrusage(RUSAGE_SELF, &ru) < 0)
        die("getrusage");

    /* /proc/self/statm: total and resident size, in pages (Linux). */
    long total, resident = -1;
    FILE *f = fopen("/proc/self/statm", "r");
    if (f) {
        if (fscanf(f, "%ld %ld", &total, &resident) != 2)
            resident = -1;
        fclose(f);
    }

    printf("%-34s minor faults %8ld   resident %8ld KiB\n",
           when, ru.ru_minflt, resident * (page_size / 1024));
}

static void touch(char *p, size_t from, size_t to)
{
    for (size_t off = from; off < to; off += page_size)
        p[off] = 1;             /* one write per page is enough */
}

int main(int argc, char *argv[])
{
    page_size = sysconf(_SC_PAGESIZE);
    size_t mib = argc > 1 ? strtoul(argv[1], NULL, 10) : 512;
    size_t len = mib * 1024 * 1024;

    printf("page size %ld bytes; %zu MiB = %zu pages\n\n",
           page_size, mib, len / page_size);
    report("start");

    char *p = mmap(NULL, len, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED)
        die("mmap");
    /* Ask for ordinary pages. Some systems would otherwise use 2 MiB "huge
       pages" here, and each fault would bring in 512 small pages at once.
       This can only fail on a kernel built without huge page support, which
       uses ordinary pages anyway, so a failure does not change the counts. */
    if (madvise(p, len, MADV_NOHUGEPAGE) < 0)
        printf("note: madvise(MADV_NOHUGEPAGE): %s; this kernel has no huge\n"
               "      pages, so it uses ordinary pages anyway\n\n",
               strerror(errno));
    report("after mmap()");

    touch(p, 0, len / 4);
    report("after touching 1/4 of the pages");

    touch(p, 0, len / 4);
    report("after touching the same 1/4 again");

    touch(p, len / 4, len);
    report("after touching all the pages");

    if (munmap(p, len) < 0)
        die("munmap");
    report("after munmap()");
    return 0;
}
