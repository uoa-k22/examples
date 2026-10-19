/*
 * 05_read_before_write_fixed.c - the fix for 05_read_before_write_bug.c.
 *
 * Run:   ./05_read_before_write_fixed
 *        diff 05_read_before_write_bug.c 05_read_before_write_fixed.c
 *
 * Look:  the reader waits on the "ready" semaphore before it reads. The
 *        semaphore starts at 0 (see 06-semaphores/04_pingpong.c), and the
 *        writer posts it only after the whole message is stored, so the
 *        reader always sees the complete message, however long the writer
 *        takes.
 */
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/mman.h>
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
        if (sem_wait(&s->ready) < 0)    /* FIX: wait until it is ready */
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
