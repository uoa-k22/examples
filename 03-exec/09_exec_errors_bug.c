/*
 * 09_exec_errors_bug.c - three different reasons exec fails, and what each
 * means.
 *
 * Run:   ./09_exec_errors_bug          the bug: three commands that cannot run
 *        ./09_exec_errors_bug fixed    the same three, each one corrected
 *
 * Look:  the program tries execv() on three files in a child process. Each
 *        fails for a different reason, and errno tells them apart:
 *        1. ENOENT   "No such file or directory": the path is wrong (here a
 *                    typo, /bin/tru). Also the error when a script's #! line
 *                    names an interpreter that does not exist.
 *        2. EACCES   "Permission denied": the file exists but is not
 *                    executable. Fix: chmod +x.
 *        3. ENOEXEC  "Exec format error": the file is executable, but the
 *                    kernel does not recognise it as a program: it is not a
 *                    binary and a script needs a first line like  #!/bin/sh
 *
 *        After the three failures the default run also calls execvp() on the
 *        same script. It works: when execvp() gets ENOEXEC it runs the file
 *        with /bin/sh itself. That is a convenience of the C library, not of
 *        the kernel, so do not count on it. execv() and execve() do not do it.
 *
 *        As in 03_fork_exec_wait.c, the child exits with 127 when exec fails.
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

int main(int argc, char *argv[])
{
    int fixed = argc > 1 && strcmp(argv[1], "fixed") == 0;

    /* Unless fixed: the script has no #! line, and NOPERM is not executable. */
    make_file(SCRIPT, fixed ? "#!/bin/sh\nexit 0\n" : "exit 0\n", 0755);
    make_file(NOPERM, "#!/bin/sh\nexit 0\n", fixed ? 0755 : 0644);

    try_exec("1. wrong path", fixed ? "/bin/true" : "/bin/tru", 0);
    try_exec("2. not executable", "./" NOPERM, 0);
    try_exec("3. script without #!", "./" SCRIPT, 0);
    if (!fixed)
        try_exec("4. same script, but execvp()", "./" SCRIPT, 1);

    unlink(SCRIPT);
    unlink(NOPERM);
    return 0;
}
