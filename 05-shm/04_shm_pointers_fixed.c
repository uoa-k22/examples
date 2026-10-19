/*
 * 04_shm_pointers_fixed.c - the fix for 04_shm_pointers_bug.c.
 *
 * Run:   ./04_shm_pointers_fixed
 *        diff 04_shm_pointers_bug.c 04_shm_pointers_fixed.c
 *
 * Look:  the nodes are linked by their index in the nodes[] array instead of
 *        by address. An index means the same thing wherever the object is
 *        mapped, so the reader prints the whole list, although it maps the
 *        object at a different address from the writer.
 *
 * Rule:  inside shared memory, link data with indexes or offsets from the
 *        start of the object, never with pointers.
 */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define SHM_NAME "/k22_shm_list"
#define NODES    4

/* FIX: the links are indexes into nodes[], valid in every process. */
struct node {
    int value;
    int next;                   /* -1 ends the list */
};

struct list {
    int         head;
    struct node nodes[NODES];
};

static void print_list(const char *who, const struct list *l)
{
    printf("%s: object mapped at %p, head = node %d\n",
           who, (void *) l, l->head);
    printf("%s: list:", who);
    fflush(stdout);
    for (int i = l->head; i != -1; i = l->nodes[i].next)
        printf(" %d", l->nodes[i].value);
    printf("\n");
}

static void writer(const char *prog)
{
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (fd < 0)
        die("shm_open");
    if (ftruncate(fd, sizeof(struct list)) < 0)
        die("ftruncate");
    struct list *l = mmap(NULL, sizeof(*l), PROT_READ | PROT_WRITE,
                          MAP_SHARED, fd, 0);
    if (l == MAP_FAILED)
        die("mmap");
    close(fd);

    for (int i = 0; i < NODES; i++) {
        l->nodes[i].value = (i + 1) * 10;
        l->nodes[i].next = i + 1 < NODES ? i + 1 : -1;
    }
    l->head = 0;
    print_list("writer", l);
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0)
        die("fork");
    if (pid == 0) {
        /* Run this same program as the reader (see 03-exec/07_self_exec). */
        execl("/proc/self/exe", prog, "reader", (char *) NULL);
        die("execl /proc/self/exe");
    }

    int status;
    if (waitpid(pid, &status, 0) < 0)
        die("waitpid");
    if (WIFSIGNALED(status))
        printf("\nwriter: the reader was killed by signal %d (%s)\n",
               WTERMSIG(status), strsignal(WTERMSIG(status)));

    munmap(l, sizeof(*l));
    if (shm_unlink(SHM_NAME) < 0)
        die("shm_unlink");
}

static void reader(void)
{
    int fd = shm_open(SHM_NAME, O_RDONLY, 0);
    if (fd < 0)
        die("shm_open");
    const struct list *l = mmap(NULL, sizeof(*l), PROT_READ, MAP_SHARED, fd, 0);
    if (l == MAP_FAILED)
        die("mmap");
    close(fd);

    print_list("reader", l);
}

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "reader") == 0)
        reader();
    else
        writer(argv[0]);
    return 0;
}
