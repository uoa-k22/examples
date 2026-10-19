/*
 * 07_separate_open.c - two processes that open the same file overwrite each other.
 *
 * Run:   ./07_separate_open          the bug
 *        ./07_separate_open fixed    the fix
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
 *        - open it with O_APPEND, as the fixed version does: every write
 *          then goes to the current end of the file, whatever the offset.
 */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define FILENAME "log.txt"
#define LINES    3

static void write_lines(const char *who, int fixed, int first_delay_ms)
{
    int flags = O_WRONLY | (fixed ? O_APPEND : 0);   /* BUG: no O_APPEND */
    int fd = open(FILENAME, flags);
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

int main(int argc, char *argv[])
{
    int fixed = argc > 1 && strcmp(argv[1], "fixed") == 0;

    int fd = open(FILENAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        die("open " FILENAME);
    close(fd);

    pid_t pid = fork();
    if (pid < 0)
        die("fork");
    if (pid == 0) {
        write_lines("child", fixed, 100);       /* writes in between the parent's */
        return 0;
    }
    write_lines("parent", fixed, 0);
    if (waitpid(pid, NULL, 0) < 0)
        die("waitpid");

    printf("expected 6 lines, %s contains:\n", FILENAME);
    fflush(stdout);
    if (system("cat -n " FILENAME) < 0)
        die("system");
    return 0;
}
