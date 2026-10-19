/*
 * 09_exec_errors_fixed.c - the fix for 09_exec_errors_bug.c.
 *
 * Run:   ./09_exec_errors_fixed
 *        diff 09_exec_errors_bug.c 09_exec_errors_fixed.c
 *
 * Look:  the same three execv() calls, each with its cause removed, and
 *        all three now work:
 *        1. the path is spelled right: /bin/true
 *        2. the file is executable (mode 0755, what chmod +x gives)
 *        3. the script starts with  #!/bin/sh , so the kernel knows which
 *           program to run it with
 *        The execvp() step is gone: with a #! line there is nothing for the
 *        C library to work around.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define SCRIPT  "exec_demo_script"
#define NOPERM  "exec_demo_noperm"

static const char *errno_name(int e)
{
    switch (e) {
    case ENOENT:  return "ENOENT";
    case EACCES:  return "EACCES";
    case ENOEXEC: return "ENOEXEC";
    default:      return "?";
    }
}

/* Make a file that holds a one-line shell command. */
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

/* fork + exec + wait; use_execvp selects execvp() instead of execv(). */
static void try_exec(const char *what, const char *path, int use_execvp)
{
    printf("%s: %s(\"%s\")\n", what, use_execvp ? "execvp" : "execv", path);
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0)
        die("fork");
    if (pid == 0) {
        char *argv[] = { (char *) path, NULL };
        if (use_execvp)
            execvp(path, argv);
        else
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
    /* FIX: the script has a #! line, and NOPERM is executable. */
    make_file(SCRIPT, "#!/bin/sh\nexit 0\n", 0755);
    make_file(NOPERM, "#!/bin/sh\nexit 0\n", 0755);

    try_exec("1. right path", "/bin/true", 0);          /* FIX: spelling */
    try_exec("2. executable", "./" NOPERM, 0);
    try_exec("3. script with #!", "./" SCRIPT, 0);

    unlink(SCRIPT);
    unlink(NOPERM);
    return 0;
}
