/*
 * 04_shm_pointers_bug.c - pointers stored in shared memory.
 *
 * Run:   ./04_shm_pointers_bug
 *        The fix is in 04_shm_pointers_fixed.c. To see what changed:
 *        diff 04_shm_pointers_bug.c 04_shm_pointers_fixed.c
 *
 * Look:  the writer builds a linked list inside a POSIX shared memory object,
 *        linking each node to the next with a pointer. Then it starts a
 *        reader: this same program, run again with fork() and exec(). The
 *        reader maps the same object, but at a different address, and
 *        follows the stored pointers. They point to where the object was
 *        mapped in the WRITER, which means nothing in the reader's address
 *        space. The reader usually crashes with SIGSEGV. If that address
 *        happens to be mapped in the reader, it reads garbage, which is
 *        worse.
 *
 * Try:   run it with address space layout randomization turned off:
 *            setarch -R ./04_shm_pointers_bug
 *        Both processes now lay out memory the same way, the object lands
 *        at the same address, and the pointers "work". The bug is still
 *        there; it is just hidden until the layout changes.
 *
 *        (A parent and a child that inherit one mapping through fork() do
 *        see it at the same address. Separate programs, or mappings made
 *        after fork(), may or may not; nothing guarantees it.)
 */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define SHM_NAME "/k22_shm_list"
#define NODES    4

/* BUG: the links are addresses, valid only in the process that set them. */
struct node {
    int          value;
    struct node *next;          /* NULL ends the list */
};

struct list {
    struct node *head;
    struct node  nodes[NODES];
};

static void print_list(const char *who, const struct list *l)
{
    printf("%s: object mapped at %p, head = %p\n",
           who, (void *) l, (void *) l->head);
    printf("%s: list:", who);
    fflush(stdout);
    for (const struct node *n = l->head; n != NULL; n = n->next)
        printf(" %d", n->value);
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
        l->nodes[i].next = i + 1 < NODES ? &l->nodes[i + 1] : NULL;
    }
    l->head = &l->nodes[0];
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
