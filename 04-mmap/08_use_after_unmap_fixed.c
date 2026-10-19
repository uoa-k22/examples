/*
 * 08_use_after_unmap_fixed.c - the fix for 08_use_after_unmap_bug.c.
 *
 * Run:   ./08_use_after_unmap_fixed
 *        diff 08_use_after_unmap_bug.c 08_use_after_unmap_fixed.c
 *
 * Look:  the child finishes using the memory before it unmaps it, and then
 *        sets the pointer to NULL. The child exits normally. Setting the
 *        pointer to NULL does not matter here, but in a bigger program it
 *        turns a later mistake into an immediate crash at a NULL pointer,
 *        instead of a read from whatever is mapped there by then.
 */
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

static void child(void)
{
    long page = sysconf(_SC_PAGESIZE);

    char *p = mmap(NULL, page, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED)
        die("mmap");
    strcpy(p, "hello");
    printf("child: stored \"%s\" in the page at %p\n", p, (void *) p);

    /* FIX: use the memory first, unmap it last, then forget the pointer. */
    printf("child: last use before unmapping: \"%s\"\n", p);
    if (munmap(p, page) < 0)
        die("munmap");
    p = NULL;
    printf("child: page unmapped; not touching it again\n");
}

int main(void)
{
    pid_t pid = fork();
    if (pid < 0)
        die("fork");
    if (pid == 0) {
        child();
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
