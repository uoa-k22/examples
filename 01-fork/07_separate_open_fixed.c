/*
 * 07_separate_open_fixed.c - the fix for 07_separate_open_bug.c.
 *
 * Run:   ./07_separate_open_fixed
 *        diff 07_separate_open_bug.c 07_separate_open_fixed.c
 *
 * Look:  each process still opens log.txt itself, but with O_APPEND. Every
 *        write() then goes to the current end of the file, whatever the
 *        process's own offset says, so no line lands on top of another and
 *        all six lines survive, alternating parent and child.
 *
 *        The other fix would be to open the file once, before fork(), and
 *        let both processes share the descriptor (see 03_shared_fd.c).
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
    int fd = open(FILENAME, O_WRONLY | O_APPEND);   /* FIX: O_APPEND */
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
