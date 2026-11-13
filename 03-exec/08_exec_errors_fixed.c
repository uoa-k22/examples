/*
 * 08_exec_errors_fixed.c - the fix for 08_exec_errors_bug.c.
 *
 * Run:   ./08_exec_errors_fixed
 *        diff 08_exec_errors_bug.c 08_exec_errors_fixed.c
 *
 * Look:  the same three execv() calls, each with its cause removed, and
 *        all three now work:
 *        1. the path is spelled right: /bin/true
 *        2. the file is executable (mode 0755, what chmod +x gives)
 *        3. the script starts with  #!/bin/sh , so the kernel knows which
 *           program to run it with
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define FILE2   "exec_demo_2"
#define FILE3   "exec_demo_3"

static const char *errno_name(int e)
{
    switch (e) {
    case ENOENT:  return "ENOENT";
    case EACCES:  return "EACCES";
    case ENOEXEC: return "ENOEXEC";
    default:      return "?";
    }
}

/* Make a file with these contents and this mode. */
static void make_file(const char *name, const char *contents, mode_t mode)
{
    int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, mode);
    if (fd < 0)
        die(name);
    if (write(fd, contents, strlen(contents)) < 0)
        die("write");
    close(fd);
    if (chmod(name, mode) < 0)  /* open() applies the umask; chmod does not */
        die("chmod");
}

/* fork + execv + wait, and report the result. */
static void try_exec(const char *what, const char *path)
{
    printf("%s: execv(\"%s\")\n", what, path);
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0)
        die("fork");
    if (pid == 0) {
        char *argv[] = { (char *) path, NULL };
        execv(path, argv);
        printf("  failed: %s (%s)\n", strerror(errno), errno_name(errno));
        fflush(stdout);
        _exit(127);
    }

    int status;
    if (waitpid(pid, &status, 0) < 0)
        die("waitpid");
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
        printf("  worked\n");
    else if (WIFEXITED(status))
        printf("  child exited with status %d\n", WEXITSTATUS(status));
}

int main(void)
{
    /* FIX: file 2 is executable, and file 3 has a #! line. */
    make_file(FILE2, "#!/bin/sh\nexit 0\n", 0755);
    make_file(FILE3, "#!/bin/sh\nexit 0\n", 0755);

    try_exec("1. right path", "/bin/true");             /* FIX: spelling */
    try_exec("2. executable", "./" FILE2);
    try_exec("3. script with #!", "./" FILE3);

    unlink(FILE2);
    unlink(FILE3);
    return 0;
}
