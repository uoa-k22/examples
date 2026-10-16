/*
 * 04_shm_pointers.c - why pointers must not be stored in shared memory.
 *
 * Run:   ./04_shm_pointers
 *
 * Look:  the writer builds a linked list inside a POSIX shared memory object
 *        and then starts a reader: this same program, run again with fork()
 *        and exec(). The reader maps the same object, but at a different
 *        address. Each node stores its successor twice:
 *          next        a pointer: an address in the WRITER's memory
 *          next_index  an index into the nodes[] array
 *        As written, the reader follows the indexes and prints the list.
 *
 * Try:   uncomment the line marked below, run make and run again. Now the
 *        reader follows the stored pointers. They point to where the object
 *        was mapped in the writer, which means nothing in the reader's
 *        address space. It usually crashes with SIGSEGV. If that address
 *        happens to be mapped in the reader, it reads garbage, which is
 *        worse.
 *
 *        Then run it with address space layout randomization turned off:
 *            setarch -R ./04_shm_pointers
 *        Both processes now lay out memory the same way, the object lands
 *        at the same address, and the pointers "work". The bug is still
 *        there; it is just hidden until the layout changes.
 *
 * Rule:  inside shared memory, link data with indexes or offsets from the
 *        start of the object, never with pointers. (A parent and a child that
 *        inherit one mapping through fork() do see it at the same address.
 *        Separate programs, or mappings made after fork(), may or may not;
 *        nothing guarantees it.)
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

struct node {
    int          value;
    struct node *next;          /* only valid in the process that set it */
    int          next_index;    /* valid everywhere; -1 ends the list */
};

struct list {
    struct node *head;
    int          head_index;
    struct node  nodes[NODES];
};

static void writer(const char *prog)
{
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (fd < 0)
        die("shm_open");
    if (ftruncate(fd, sizeof(struct list)) < 0)
        die("ftruncate");
    struct list *l = mmap(NULL, sizeof(*l), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (l == MAP_FAILED)
        die("mmap");
    close(fd);

    for (int i = 0; i < NODES; i++) {
        l->nodes[i].value = (i + 1) * 10;
        l->nodes[i].next = i + 1 < NODES ? &l->nodes[i + 1] : NULL;
        l->nodes[i].next_index = i + 1 < NODES ? i + 1 : -1;
    }
    l->head = &l->nodes[0];
    l->head_index = 0;

    printf("writer: object mapped at %p, head pointer = %p\n", (void *) l, (void *) l->head);
    printf("writer: list:");
    for (struct node *n = l->head; n != NULL; n = n->next)
        printf(" %d", n->value);
    printf("\n");
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0)
        die("fork");
    if (pid == 0) {
        execl(prog, prog, "reader", (char *) NULL);
        die("execl");
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
    int use_pointers = 0;
    // use_pointers = 1;           /* <-- uncomment this line to see the bug */

    int fd = shm_open(SHM_NAME, O_RDONLY, 0);
    if (fd < 0)
        die("shm_open");
    struct list *l = mmap(NULL, sizeof(*l), PROT_READ, MAP_SHARED, fd, 0);
    if (l == MAP_FAILED)
        die("mmap");
    close(fd);

    printf("reader: object mapped at %p, head pointer = %p\n", (void *) l, (void *) l->head);
    printf("reader: list, following %s:", use_pointers ? "pointers" : "indexes");
    fflush(stdout);

    if (use_pointers) {
        for (struct node *n = l->head; n != NULL; n = n->next) {
            printf(" %d", n->value);
            fflush(stdout);
        }
    } else {
        for (int i = l->head_index; i != -1; i = l->nodes[i].next_index)
            printf(" %d", l->nodes[i].value);
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "reader") == 0)
        reader();
    else
        writer(argv[0]);
    return 0;
}
