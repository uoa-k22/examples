/*
 * 14_readers_writers.c - the readers-writers problem with semaphores.
 *
 * Run:   ./14_readers_writers
 *
 * Look:  readers only look at the shared data, so several may do it at the
 *        same time. A writer changes it, so it must be alone: no readers
 *        and no other writer. Two semaphores do this:
 *          wrt    (starts at 1)  held by one writer, or by the readers as
 *                                a group
 *          mutex  (starts at 1)  protects the count "readers"
 *        The first reader to arrive takes wrt, and the last one to leave
 *        gives it back. The readers in between only change the count.
 *        "[n reading]" goes above 1: readers overlap.
 *
 *        The data is two numbers that must always be equal. A writer
 *        changes one, sleeps, and changes the other; a reader reads one,
 *        sleeps, and reads the other. No reader ever sees them differ, and
 *        no write is lost: the last line reports both.
 *
 *        This solution favours the readers. While one reader is inside,
 *        more may join it, so a steady stream of readers could keep a
 *        writer waiting forever (starvation).
 */
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define NAME    "/k22_sem_readers_writers"
#define READERS 3
#define WRITERS 2
#define READS   4       /* per reader */
#define WRITES  3       /* per writer */

struct shared {
    sem_t wrt, mutex;
    int   readers;      /* readers inside now */
    long  a, b;         /* the data: equal, except in the middle of a write */
    int   mismatches;   /* reads that saw a != b */
};

static struct shared *s;

static void reader(int id)
{
    unsigned seed = getpid();

    for (int i = 0; i < READS; i++) {
        usleep(rand_r(&seed) % 300000);         /* other work */

        if (sem_wait(&s->mutex) < 0)
            die("sem_wait");
        s->readers++;
        if (s->readers == 1) {                  /* first in: keep writers out */
            if (sem_wait(&s->wrt) < 0)
                die("sem_wait");
        }
        if (sem_post(&s->mutex) < 0)
            die("sem_post");

        long a = s->a;
        usleep(100000);                         /* a slow read */
        long b = s->b;

        if (sem_wait(&s->mutex) < 0)
            die("sem_wait");
        if (a != b)
            s->mismatches++;
        printf("reader %d: read %ld   [%d reading]\n", id, a, s->readers);
        s->readers--;
        if (s->readers == 0) {                  /* last out: let writers in */
            if (sem_post(&s->wrt) < 0)
                die("sem_post");
        }
        if (sem_post(&s->mutex) < 0)
            die("sem_post");
    }
}

static void writer(int id)
{
    unsigned seed = getpid();

    for (int i = 0; i < WRITES; i++) {
        usleep(rand_r(&seed) % 300000);         /* other work */

        if (sem_wait(&s->wrt) < 0)
            die("sem_wait");
        s->a++;
        usleep(100000);                         /* a slow write: a != b now */
        s->b++;
        printf("    writer %d: wrote %ld\n", id, s->b);
        if (sem_post(&s->wrt) < 0)
            die("sem_post");
    }
}

int main(void)
{
    setvbuf(stdout, NULL, _IOLBF, 0);

    int fd = shm_open(NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd < 0)
        die("shm_open");
    if (shm_unlink(NAME) < 0)
        die("shm_unlink");
    if (ftruncate(fd, sizeof(struct shared)) < 0)
        die("ftruncate");
    s = mmap(NULL, sizeof(*s), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (s == MAP_FAILED)
        die("mmap");
    close(fd);
    if (sem_init(&s->wrt, 1, 1) < 0 || sem_init(&s->mutex, 1, 1) < 0)
        die("sem_init");

    for (int i = 0; i < READERS + WRITERS; i++) {
        pid_t pid = fork();
        if (pid < 0)
            die("fork");
        if (pid == 0) {
            if (i < READERS)
                reader(i);
            else
                writer(i - READERS);
            return 0;
        }
    }
    while (wait(NULL) > 0)
        ;

    printf("value = %ld, expected %d; reads that saw a half-done write: %d\n",
           s->b, WRITERS * WRITES, s->mismatches);

    sem_destroy(&s->wrt);
    sem_destroy(&s->mutex);
    munmap(s, sizeof(*s));
    return 0;
}
