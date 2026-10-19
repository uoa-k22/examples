/*
 * 06_read_before_write_bug.c - shared memory does not make a reader wait.
 *
 * Run:   ./06_read_before_write_bug          the bug
 *        ./06_read_before_write_bug fixed    the fix
 *
 * Look:  the parent (the writer) needs one second to prepare its message
 *        and then stores it in shared memory. The child (the reader) starts
 *        at once and reads the message straight away. In the buggy version it
 *        finds the buffer still empty: the memory is shared, but nothing
 *        tells the reader that the data is ready. With less delay it would
 *        sometimes see half a message, which is worse.
 *
 *        The fix is a semaphore in the shared object, starting at 0 (see
 *        06-semaphores/04_pingpong.c). The writer posts it after storing the
 *        message; the reader waits on it before reading.
 *
 *        The object is unlinked right after mapping. The parent and child
 *        inherit the mapping through fork(), so they no longer need its name.
 */
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
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

int main(int argc, char *argv[])
{
    int fixed = argc > 1 && strcmp(argv[1], "fixed") == 0;

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
        if (fixed && sem_wait(&s->ready) < 0)       /* the fix */
            die("sem_wait");
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
