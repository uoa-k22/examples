/*
 * 05_no_ftruncate_fixed.c - the fix for 05_no_ftruncate_bug.c.
 *
 * Run:   ./05_no_ftruncate_fixed
 *        diff 05_no_ftruncate_bug.c 05_no_ftruncate_fixed.c
 *
 * Look:  the fix is the call you saw in 01_posix_shm_writer.c: give the
 *        object its size with ftruncate() before touching it. The child now
 *        reports a size of one page, the store works, and the child exits
 *        normally. The parent removes the object afterwards.
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
    if (ftruncate(fd, page) < 0)       /* FIX: give the object a size */
        die("ftruncate");

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
