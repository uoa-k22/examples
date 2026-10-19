/*
 * 04_shared_anon_fork.c - memory shared between parent and child.
 *
 * Run:   ./04_shared_anon_fork            MAP_SHARED
 *        ./04_shared_anon_fork private    MAP_PRIVATE
 *
 * Look:  the mapping is created before fork(), so the child inherits it.
 *        The child multiplies every element by 10.
 *        - MAP_SHARED: parent and child use the same physical pages, so the
 *          parent sees the child's changes. This is the simplest way to
 *          share memory between related processes.
 *        - MAP_PRIVATE: the child's writes go to private copies of the pages
 *          (copy-on-write), like ordinary memory after fork(), so the parent
 *          still sees the original values.
 */
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
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

int main(int argc, char *argv[])
{
    int private = argc > 1 && strcmp(argv[1], "private") == 0;
    int flags = (private ? MAP_PRIVATE : MAP_SHARED) | MAP_ANONYMOUS;

    int *a = mmap(NULL, N * sizeof(int), PROT_READ | PROT_WRITE, flags, -1, 0);
    if (a == MAP_FAILED)
        die("mmap");

    printf("Using %s\n", private ? "MAP_PRIVATE" : "MAP_SHARED");
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
