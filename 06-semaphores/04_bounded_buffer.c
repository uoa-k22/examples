/*
 * 04_bounded_buffer.c - the producer/consumer problem with semaphores.
 *
 * Run:   ./04_bounded_buffer
 *
 * Look:  producers put items into a circular buffer of SLOTS places, and
 *        consumers take them out. Three semaphores coordinate them:
 *          empty  (starts at SLOTS)  free places; producers wait on it
 *          full   (starts at 0)      filled places; consumers wait on it
 *          mutex  (starts at 1)      protects buf, in, out and sum
 *        The "[n in buffer]" count never goes above SLOTS or below 0. At
 *        the end, the sum of everything consumed matches the sum of
 *        everything produced: nothing is lost or taken twice.
 *
 *        The order of the waits matters. A producer waits on empty first
 *        and on mutex second. If it held the mutex while waiting on a full
 *        buffer, no consumer could get in to empty it: deadlock.
 */
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define SLOTS     4
#define PRODUCERS 2
#define CONSUMERS 2
/* Items per producer. PRODUCERS * ITEMS must be divisible by CONSUMERS. */
#define ITEMS     8

struct shared {
    sem_t empty, full, mutex;
    int   buf[SLOTS];
    int   in, out;      /* next slot to fill / to empty */
    int   count;        /* items in the buffer, only for printing */
    long  sum;          /* sum of all consumed items */
};

static struct shared *s;

static void P(sem_t *sem) { if (sem_wait(sem) < 0) die("sem_wait"); }
static void V(sem_t *sem) { if (sem_post(sem) < 0) die("sem_post"); }

static void producer(int id)
{
    unsigned seed = getpid();

    for (int i = 0; i < ITEMS; i++) {
        int item = id * 100 + i;
        usleep(rand_r(&seed) % 100000);         /* time to "make" the item */

        P(&s->empty);
        P(&s->mutex);
        s->buf[s->in] = item;
        s->in = (s->in + 1) % SLOTS;
        s->count++;
        printf("producer %d: put  %3d   [%d in buffer]\n", id, item, s->count);
        V(&s->mutex);
        V(&s->full);
    }
}

static void consumer(int id)
{
    unsigned seed = getpid();

    for (int i = 0; i < PRODUCERS * ITEMS / CONSUMERS; i++) {
        P(&s->full);
        P(&s->mutex);
        int item = s->buf[s->out];
        s->out = (s->out + 1) % SLOTS;
        s->count--;
        s->sum += item;
        printf("    consumer %d: took %3d   [%d in buffer]\n",
               id, item, s->count);
        V(&s->mutex);
        V(&s->empty);

        usleep(rand_r(&seed) % 150000);         /* time to "use" the item */
    }
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    s = mmap(NULL, sizeof(*s), PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (s == MAP_FAILED)
        die("mmap");
    if (sem_init(&s->empty, 1, SLOTS) < 0 ||
        sem_init(&s->full, 1, 0) < 0 ||
        sem_init(&s->mutex, 1, 1) < 0)
        die("sem_init");

    for (int i = 0; i < PRODUCERS + CONSUMERS; i++) {
        pid_t pid = fork();
        if (pid < 0)
            die("fork");
        if (pid == 0) {
            if (i < PRODUCERS)
                producer(i);
            else
                consumer(i - PRODUCERS);
            _exit(0);
        }
    }
    while (wait(NULL) > 0)
        ;

    long expected = 0;
    for (int p = 0; p < PRODUCERS; p++)
        for (int i = 0; i < ITEMS; i++)
            expected += p * 100 + i;
    printf("sum produced = %ld, sum consumed = %ld: %s\n",
           expected, s->sum, expected == s->sum ? "OK" : "MISMATCH");

    sem_destroy(&s->empty);
    sem_destroy(&s->full);
    sem_destroy(&s->mutex);
    munmap(s, sizeof(*s));
    return 0;
}
