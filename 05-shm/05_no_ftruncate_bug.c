/*
 * 05_no_ftruncate_bug.c - mapping a shared memory object that has no size.
 *
 * Run:   ./05_no_ftruncate_bug          the bug
 *        ./05_no_ftruncate_bug fixed    the fix
 *
 * Look:  shm_open() creates an object of size 0. The buggy program skips
 *        ftruncate() and maps one page anyway. mmap() succeeds: it only
 *        reserves addresses. But there is no memory behind them, and the
 *        first store raises SIGBUS ("Bus error"), which kills the child.
 *        Compare SIGSEGV in 04-mmap/06_mprotect.c: SEGV means "no mapping
 *        here", BUS means "mapped, but nothing backs this page".
 *
 *        The fix is the call you saw in 01_posix_shm_writer.c: give the
 *        object its size with ftruncate() before touching it. The parent
 *        removes the object either way.
 */
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define NAME "/k22_shm_noftruncate"

static void child(int fixed)
{
    long page = sysconf(_SC_PAGESIZE);

    int fd = shm_open(NAME, O_CREAT | O_RDWR, 0600);
    if (fd < 0)
        die("shm_open");
    if (fixed && ftruncate(fd, page) < 0)
        die("ftruncate");

    struct stat st;
    if (fstat(fd, &st) < 0)
        die("fstat");
    printf("child: the object's size is %ld bytes\n", (long) st.st_size);

    char *p = mmap(NULL, page, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED)
        die("mmap");
    close(fd);
    printf("child: mmap() of %ld bytes succeeded; now storing into it...\n", page);
    fflush(stdout);

    strcpy(p, "hello");
    printf("child: stored \"%s\"\n", p);
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

    if (shm_unlink(NAME) < 0)
        die("shm_unlink");
    return 0;
}
