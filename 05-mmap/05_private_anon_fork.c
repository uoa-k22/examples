/*
 * 05_private_anon_fork.c - anonymous private memory is copied by fork().
 *
 * Run:   ./05_private_anon_fork
 *        Compare with 04_shared_anon_fork.c:
 *        diff 04_shared_anon_fork.c 05_private_anon_fork.c
 *
 * Look:  the same program with MAP_PRIVATE instead of MAP_SHARED. The
 *        child's writes go to private copies of the pages (copy-on-write),
 *        like ordinary memory after fork(), so the parent still sees the
 *        original values.
 */
#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define N 5

static void print(const char *who, const int *a)
{
    printf("%-7s:", who);
    for (int i = 0; i < N; i++)
        printf(" %d", a[i]);
    printf("\n");
}

int main(void)
{
    int *a = mmap(NULL, N * sizeof(int), PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (a == MAP_FAILED)
        die("mmap");

    for (int i = 0; i < N; i++)
        a[i] = i + 1;
    print("before", a);
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        for (int i = 0; i < N; i++)
            a[i] *= 10;
        print("child", a);
        return 0;
    }

    if (waitpid(pid, NULL, 0) < 0)
        die("waitpid");
    print("parent", a);

    if (munmap(a, N * sizeof(int)) < 0)
        die("munmap");
    return 0;
}
