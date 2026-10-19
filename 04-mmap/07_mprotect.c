/*
 * 07_mprotect.c - pages have permissions, and the hardware enforces them.
 *
 * Run:   ./07_mprotect
 *
 * Look:  the child maps one page with read and write permission and writes
 *        into it. Then mprotect() makes the page read-only. Reading still
 *        works, but the next write traps: the MMU refuses it, the kernel
 *        sends SIGSEGV, and the child dies. The parent reports the signal,
 *        as in 02-wait/03_killed_by_signal.c.
 *
 *        Permissions are per page, so mprotect() works on whole pages. The
 *        same mechanism protects everyday memory: a program's code is
 *        read-only, its data is not executable, nothing is mapped at
 *        address 0 (which is why *NULL crashes), and there is a guard page
 *        below the stack. Compare  cat /proc/self/maps  (r/w/x columns).
 */
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
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
    printf("child: wrote \"%s\" into the page at %p\n", p, (void *) p);

    if (mprotect(p, page, PROT_READ) < 0)
        die("mprotect");
    printf("child: page is now read-only; reading it still works: \"%s\"\n", p);

    printf("child: now writing to it...\n");
    fflush(stdout);
    p[0] = 'H';

    printf("child: this line is never printed\n");
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
