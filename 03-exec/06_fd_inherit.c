/*
 * 06_fd_inherit.c - open file descriptors survive exec, unless O_CLOEXEC.
 *
 * Run:   ./06_fd_inherit && cat kept.txt
 *
 * Look:  the parent opens two files before running other programs:
 *          kept.txt            opened normally
 *          closed_on_exec.txt  opened with O_CLOEXEC ("close on exec")
 *        1. ls lists its own open descriptors (Linux: /proc/self/fd). Besides
 *           0, 1, 2 it has kept.txt, under the same number as in the parent,
 *           but not closed_on_exec.txt. One more descriptor belongs to ls
 *           itself: the /proc/.../fd directory it is reading. It usually
 *           gets the number closed_on_exec.txt had, because exec freed
 *           that number. Check where each one points.
 *        2. sh writes into the inherited descriptor with  echo ... >&N  and
 *           the text ends up in kept.txt. The new program never opened the
 *           file; it just used a number it was given.
 *        3. sh tries the same with the O_CLOEXEC descriptor and gets
 *           "Bad file descriptor": exec closed it.
 *
 *        This is how a shell does redirection and pipes: it arranges file
 *        descriptors 0, 1, 2 in the child and then calls exec. It is also a
 *        common leak: every descriptor you forget to close (or to open with
 *        O_CLOEXEC) is handed to every program you exec.
 */
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

/* fork + exec + wait, as in 03_fork_exec_wait.c */
static void run(char *const argv[])
{
    fflush(stdout);
    pid_t pid = fork();
    if (pid < 0)
        die("fork");
    if (pid == 0) {
        execvp(argv[0], argv);
        perror(argv[0]);
        _exit(127);
    }
    if (waitpid(pid, NULL, 0) < 0)
        die("waitpid");
}

int main(void)
{
    int kept = open("kept.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int closed = open("closed_on_exec.txt",
                      O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    if (kept < 0 || closed < 0)
        die("open");
    printf("parent: fd %d is kept.txt, "
           "fd %d is closed_on_exec.txt (O_CLOEXEC)\n",
           kept, closed);

    printf("\n1. The descriptors that ls finds open after exec:\n");
    char *ls[] = { "ls", "-l", "/proc/self/fd", NULL };
    run(ls);

    char cmd[128];
    char *sh[] = { "sh", "-c", cmd, NULL };

    printf("\n2. sh writes to the inherited fd %d:\n", kept);
    snprintf(cmd, sizeof(cmd),
             "echo 'written by sh through fd %d' >&%d && echo done",
             kept, kept);
    run(sh);

    printf("\n3. sh tries to write to fd %d, which had O_CLOEXEC:\n", closed);
    snprintf(cmd, sizeof(cmd), "echo 'never written' >&%d", closed);
    run(sh);

    close(kept);
    close(closed);
    return 0;
}
