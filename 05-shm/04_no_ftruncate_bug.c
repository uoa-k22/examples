/*
 * 04_no_ftruncate_bug.c - mapping a shared memory object that has no size.
 *
 * Run:   ./04_no_ftruncate_bug
 *        The fix is in 04_no_ftruncate_fixed.c. To see what changed:
 *        diff 04_no_ftruncate_bug.c 04_no_ftruncate_fixed.c
 *
 * Look:  shm_open() creates an object of size 0. The program skips
 *        ftruncate() and maps one page anyway. mmap() succeeds: it only
 *        reserves addresses. But there is no memory behind them, and the
 *        first store raises SIGBUS ("Bus error"), which kills the child.
 *        Compare SIGSEGV: the address is not mapped at all (*NULL), or the
 *        access is not allowed (a write to a read-only page, as in
 *        04-mmap/06_mprotect.c). SIGBUS means the address is mapped and the
 *        access is allowed, but the object behind the page is too short.
 *        The parent removes the object afterwards.
 */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define NAME "/k22_shm_noftruncate"

static void child(void)
{
    long page = sysconf(_SC_PAGESIZE);

    /* O_TRUNC: start from size 0 even if an interrupted run left the
       object behind; otherwise the bug would not show. */
    int fd = shm_open(NAME, O_CREAT | O_RDWR | O_TRUNC, 0600);
    if (fd < 0)
        die("shm_open");
    /* BUG: no ftruncate(), so the object stays at size 0 */

    struct stat st;
    if (fstat(fd, &st) < 0)
        die("fstat");
    printf("child: the object's size is %ld bytes\n", (long) st.st_size);

    char *p = mmap(NULL, page, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED)
        die("mmap");
    close(fd);
    printf("child: mmap() of %ld bytes succeeded; now storing into it...\n",
           page);
    fflush(stdout);

    strcpy(p, "hello");
    printf("child: stored \"%s\"\n", p);
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

    if (shm_unlink(NAME) < 0)
        die("shm_unlink");
    return 0;
}
