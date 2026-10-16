/*
 * 03_shared_fd.c - open files are shared between parent and child.
 *
 * Run:   ./03_shared_fd && cat shared_fd.txt
 *
 * Look:  the file is opened once, before fork(). The child gets a copy of
 *        the file descriptor, but both descriptors refer to the same open
 *        file, so they share one file offset. When the child writes, the
 *        parent's offset moves too, and the parent's write goes after the
 *        child's line instead of overwriting it.
 */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

static void write_line(int fd, const char *who)
{
    char line[64];
    int len = snprintf(line, sizeof(line), "%s wrote this line\n", who);

    if (write(fd, line, len) != len)
        die("write");
    printf("%-6s: offset after writing = %ld\n", who, (long) lseek(fd, 0, SEEK_CUR));
}

int main(void)
{
    int fd = open("shared_fd.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        die("open shared_fd.txt");

    write_line(fd, "Before");
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        write_line(fd, "Child");
        return 0;
    }

    if (wait(NULL) < 0)
        die("wait");
    printf("Parent: offset after the child exited = %ld\n", (long) lseek(fd, 0, SEEK_CUR));
    write_line(fd, "Parent");
    close(fd);
    return 0;
}
