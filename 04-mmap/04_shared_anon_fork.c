/*
 * 04_shared_anon_fork.c - anonymous memory shared between parent and child.
 *
 * Run:   ./04_shared_anon_fork
 *        Compare with 05_private_anon_fork.c:
 *        diff 04_shared_anon_fork.c 05_private_anon_fork.c
 *
 * Look:  the mapping is created with MAP_SHARED before fork(), so the child
 *        inherits it. The child multiplies every element by 10. Parent and
 *        child use the same physical pages, so the parent sees the child's
 *        changes. This is the simplest way to share memory between related
 *        processes.
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
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
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
