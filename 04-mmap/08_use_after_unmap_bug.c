/*
 * 08_use_after_unmap_bug.c - using memory after munmap().
 *
 * Run:   ./08_use_after_unmap_bug          the bug
 *        ./08_use_after_unmap_bug fixed    the fix
 *
 * Look:  the child maps a page, stores a string in it, calls munmap() and
 *        then reads the string again through the same pointer. munmap()
 *        removed the page from the address space, so the pointer now points
 *        at nothing, and the read raises SIGSEGV. The parent reports it, as
 *        in 02-wait/03_killed_by_signal.c.
 *
 *        This bug is "use after free". With malloc() and free() it often
 *        seems to work, because free() usually keeps the memory in the
 *        process and the old data is still there. It fails later, or
 *        in a different place. munmap() hands the page back at once, so
 *        here the crash is immediate and reliable, which makes it a good
 *        place to learn what the bug is.
 *
 *        The fix: finish using the memory before unmapping it, and set the
 *        pointer to NULL afterwards, so a later mistake crashes at once
 *        too.
 */
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

static void child(int fixed)
{
    long page = sysconf(_SC_PAGESIZE);

    char *p = mmap(NULL, page, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED)
        die("mmap");
    strcpy(p, "hello");
    printf("child: stored \"%s\" in the page at %p\n", p, (void *) p);

    if (!fixed) {
        if (munmap(p, page) < 0)
            die("munmap");
        printf("child: page unmapped; reading it again...\n");
        fflush(stdout);
        printf("child: \"%s\"\n", p);   /* BUG: p points to nothing now */
        return;
    }

    /* The fix: use the memory first, unmap it last, then forget the pointer. */
    printf("child: last use before unmapping: \"%s\"\n", p);
    if (munmap(p, page) < 0)
        die("munmap");
    p = NULL;
    printf("child: page unmapped; not touching it again\n");
}

int main(int argc, char *argv[])
{
    int fixed = argc > 1 && strcmp(argv[1], "fixed") == 0;

    pid_t pid = fork();
    if (pid < 0)
        die("fork");
    if (pid == 0) {
        child(fixed);
        return 0;
    }

    int status;
    if (waitpid(pid, &status, 0) < 0)
        die("waitpid");
    if (WIFSIGNALED(status))
        printf("parent: child was killed by signal %d (%s)\n",
               WTERMSIG(status), strsignal(WTERMSIG(status)));
    else
        printf("parent: child exited with status %d\n", WEXITSTATUS(status));
    return 0;
}
