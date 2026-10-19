/*
 * 06_read_before_write_bug.c - shared memory does not make a reader wait.
 *
 * Run:   ./06_read_before_write_bug
 *        The fix is in 06_read_before_write_fixed.c. To see what changed:
 *        diff 06_read_before_write_bug.c 06_read_before_write_fixed.c
 *
 * Look:  the parent (the writer) needs one second to prepare its message
 *        and then stores it in shared memory. The child (the reader) starts
 *        at once and reads the message straight away. It finds the buffer
 *        still empty: the memory is shared, but nothing tells the reader
 *        that the data is ready. With less delay it would sometimes see half
 *        a message, which is worse.
 *
 *        The shared object already holds a semaphore, "ready", and the
 *        writer posts it, but the reader never waits on it.
 *
 *        The object is unlinked right after mapping. The parent and child
 *        inherit the mapping through fork(), so they no longer need its name.
 */
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define NAME "/k22_shm_readwrite"

struct shared {
    sem_t ready;        /* posted by the writer when text is complete */
    char  text[64];
};

int main(void)
{
    int fd = shm_open(NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd < 0)
        die("shm_open");
    if (shm_unlink(NAME) < 0)
        die("shm_unlink");
    if (ftruncate(fd, sizeof(struct shared)) < 0)
        die("ftruncate");
    struct shared *s = mmap(NULL, sizeof(*s), PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
    if (s == MAP_FAILED)
        die("mmap");
    close(fd);
    if (sem_init(&s->ready, 1, 0) < 0)
        die("sem_init");

    setvbuf(stdout, NULL, _IONBF, 0);
    pid_t pid = fork();
    if (pid < 0)
        die("fork");

    if (pid == 0) {
        /* BUG: reads at once, without waiting for "ready" */
        printf("reader: the message is \"%s\"\n", s->text);
        return 0;
    }

    printf("writer: preparing the message (takes 1 second)...\n");
    sleep(1);
    snprintf(s->text, sizeof(s->text), "hello, the data is ready");
    if (sem_post(&s->ready) < 0)
        die("sem_post");
    printf("writer: stored the message\n");

    if (waitpid(pid, NULL, 0) < 0)
        die("waitpid");
    sem_destroy(&s->ready);
    return 0;
}
