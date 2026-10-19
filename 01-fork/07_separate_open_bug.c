/*
 * 07_separate_open_bug.c - two processes that open the same file overwrite
 * each other.
 *
 * Run:   ./07_separate_open_bug
 *        The fix is in 07_separate_open_fixed.c. To see what changed:
 *        diff 07_separate_open_bug.c 07_separate_open_fixed.c
 *
 * Look:  the parent and the child each write three lines to log.txt, taking
 *        turns, so six lines are expected. Each process calls open() itself,
 *        after fork(). That gives each one its own open file with its own
 *        offset, and both offsets start at 0. The child's first line lands
 *        on top of the parent's first line, and so on: only three lines
 *        survive. Nothing reports an error.
 *
 *        Compare 03_shared_fd.c: there the file is opened once, BEFORE
 *        fork(), so both processes share one offset and the lines follow
 *        each other. Two fixes:
 *        - open the file before fork() and share the descriptor, or
 *        - open it with O_APPEND, as 07_separate_open_fixed.c does: every
 *          write then goes to the current end of the file, whatever the
 *          offset.
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define FILENAME "log.txt"
#define LINES    3

static void write_lines(const char *who, int first_delay_ms)
{
    int fd = open(FILENAME, O_WRONLY);      /* BUG: no O_APPEND */
    if (fd < 0)
        die("open " FILENAME);

    usleep(first_delay_ms * 1000);
    for (int i = 1; i <= LINES; i++) {
        char line[64];
        int len = snprintf(line, sizeof(line), "%-6s line %d\n", who, i);
        if (write(fd, line, len) != len)
            die("write");
        usleep(200000);
    }
    close(fd);
}

int main(void)
{
    int fd = open(FILENAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        die("open " FILENAME);
    close(fd);

    pid_t pid = fork();
    if (pid < 0)
        die("fork");
    if (pid == 0) {
        write_lines("child", 100);          /* in between the parent's */
        return 0;
    }
    write_lines("parent", 0);
    if (waitpid(pid, NULL, 0) < 0)
        die("waitpid");

    printf("expected 6 lines, %s contains:\n", FILENAME);
    fflush(stdout);
    if (system("cat -n " FILENAME) < 0)
        die("system");
    return 0;
}
