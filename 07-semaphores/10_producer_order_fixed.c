/*
 * 10_producer_order_fixed.c - the fix for 10_producer_order_bug.c.
 *
 * Run:   ./10_producer_order_fixed
 *        diff 10_producer_order_bug.c 10_producer_order_fixed.c
 *
 * Look:  the producer waits on "empty" first and on the mutex second, the
 *        order 05_bounded_buffer.c uses. When the buffer is full it sleeps
 *        on "empty" WITHOUT holding the mutex, so the consumer can still get
 *        in, take an item and post "empty". All six items get through.
 *
 *        The same rule as in 07_deadlock_fixed.c: when you must hold several
 *        semaphores, the order in which you take them matters. Never sleep
 *        on a condition while holding the lock that others need to change
 *        it.
 */
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define NAME  "/k22_sem_producer_order"
#define SLOTS 2
#define ITEMS 6

struct shared {
    sem_t empty, full, mutex;
    int   buf[SLOTS];
    int   in, out;
};

static struct shared *s;

static void producer(void)
{
    for (int i = 1; i <= ITEMS; i++) {
        printf("producer: wants to put %d\n", i);
        /* FIX: first a free place, then exclusive access */
        if (sem_wait(&s->empty) < 0)
            die("sem_wait");
        if (sem_wait(&s->mutex) < 0)
            die("sem_wait");
        s->buf[s->in] = i;
        s->in = (s->in + 1) % SLOTS;
        printf("producer: put %d\n", i);
        if (sem_post(&s->mutex) < 0)
            die("sem_post");
        if (sem_post(&s->full) < 0)
            die("sem_post");
    }
}

static void consumer(void)
{
    for (int i = 1; i <= ITEMS; i++) {
        usleep(300000);         /* a slow consumer: the buffer fills up */
        if (sem_wait(&s->full) < 0)
            die("sem_wait");
        if (sem_wait(&s->mutex) < 0)
            die("sem_wait");
        int item = s->buf[s->out];
        s->out = (s->out + 1) % SLOTS;
        printf("    consumer: took %d\n", item);
        if (sem_post(&s->mutex) < 0)
            die("sem_post");
        if (sem_post(&s->empty) < 0)
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
    if (sem_init(&s->empty, 1, SLOTS) < 0 ||
        sem_init(&s->full, 1, 0) < 0 ||
        sem_init(&s->mutex, 1, 1) < 0)
        die("sem_init");

    pid_t pids[2];
    for (int i = 0; i < 2; i++) {
        pids[i] = fork();
        if (pids[i] < 0)
            die("fork");
        if (pids[i] == 0) {
            alarm(3);           /* give up after 3 seconds */
            if (i == 0)
                producer();
            else
                consumer();
            return 0;
        }
    }

    for (int i = 0; i < 2; i++) {
        int status;
        if (waitpid(pids[i], &status, 0) < 0)
            die("waitpid");
        printf("parent: the %s %s\n", i == 0 ? "producer" : "consumer",
               WIFSIGNALED(status) ? "was killed by SIGALRM: deadlock"
                                   : "finished");
    }
    return 0;
}
