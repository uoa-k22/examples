/*
 * 08_exec_errors_bug.c - three different reasons exec fails, and what each
 * means.
 *
 * Run:   ./08_exec_errors_bug
 *        The fix is in 08_exec_errors_fixed.c. To see what changed:
 *        diff 08_exec_errors_bug.c 08_exec_errors_fixed.c
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
 *        execvp() would hide the third failure: when it gets ENOEXEC it runs
 *        the file with /bin/sh itself. That is a convenience of the C
 *        library, not of the kernel, so do not count on it. execv() and
 *        execve() do not do it.
 *
 *        As in 03_fork_exec_wait.c, the child exits with 127 when exec fails.
 *
 * Try:   03_fork_exec_wait.c uses execvp(), so it runs a script without #!:
 *            printf 'echo run by sh\n' > script; chmod +x script
 *            ./03_fork_exec_wait ./script; rm script
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

/* Step 2 checks this script's mode, step 3 its first line. */
#define MODE_SCRIPT     "exec_demo_mode"
#define HASHBANG_SCRIPT "exec_demo_hashbang"

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
    /* BUG: MODE_SCRIPT is not executable; HASHBANG_SCRIPT has no #! line. */
    make_file(MODE_SCRIPT, "#!/bin/sh\nexit 0\n", 0644);
    make_file(HASHBANG_SCRIPT, "exit 0\n", 0755);

    try_exec("1. wrong path", "/bin/tru");              /* BUG: typo */
    try_exec("2. not executable", "./" MODE_SCRIPT);
    try_exec("3. script without #!", "./" HASHBANG_SCRIPT);

    unlink(MODE_SCRIPT);
    unlink(HASHBANG_SCRIPT);
    return 0;
}
