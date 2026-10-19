/*
 * 12_producer_order_bug.c - waiting in the wrong order deadlocks a bounded buffer.
 *
 * Run:   ./12_producer_order_bug          the bug
 *        ./12_producer_order_bug fixed    the fix
 *
 * Look:  this is a small version of 05_bounded_buffer.c: one producer, one
 *        consumer, and a buffer of SLOTS places. The producer is fast and the
 *        consumer is slow, so the buffer fills up.
 *
 *        The buggy producer takes the mutex FIRST and waits for a free place
 *        SECOND. When the buffer is full it is blocked on "empty" while still
 *        holding the mutex. The consumer, which is the only one who could free
 *        a place, needs the mutex to take an item, so it waits for the
 *        producer, and the producer waits for the consumer: deadlock. No
 *        error is reported; both just stop printing. alarm(3) ends the demo.
 *
 *        The fix is the order 05_bounded_buffer.c uses: wait on "empty" first
 *        and on the mutex second, so nobody ever sleeps while holding the
 *        mutex. The same rule as in 08_deadlock.c: when you must hold several
 *        semaphores, the order in which you take them matters.
 *
 *        (The consumer's waits are in the correct order in both versions.)
 */
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errexit.h"

#define SLOTS 2
#define ITEMS 6

struct shared {
    sem_t empty, full, mutex;
    int   buf[SLOTS];
    int   in, out;
};

static struct shared *s;

static void P(sem_t *sem) { if (sem_wait(sem) < 0) die("sem_wait"); }
static void V(sem_t *sem) { if (sem_post(sem) < 0) die("sem_post"); }

static void producer(int fixed)
{
    for (int i = 1; i <= ITEMS; i++) {
        printf("producer: wants to put %d\n", i);
        if (fixed) {
            P(&s->empty);       /* first a free place... */
            P(&s->mutex);       /* ...then exclusive access */
        } else {
            P(&s->mutex);       /* BUG: sleeps on "empty" holding the mutex */
            P(&s->empty);
        }
        s->buf[s->in] = i;
        s->in = (s->in + 1) % SLOTS;
        printf("producer: put %d\n", i);
        V(&s->mutex);
        V(&s->full);
    }
}

static void consumer(void)
{
    for (int i = 1; i <= ITEMS; i++) {
        usleep(300000);         /* a slow consumer: the buffer fills up */
        P(&s->full);
        P(&s->mutex);
        int item = s->buf[s->out];
        s->out = (s->out + 1) % SLOTS;
        printf("    consumer: took %d\n", item);
        V(&s->mutex);
        V(&s->empty);
    }
}

int main(int argc, char *argv[])
{
    int fixed = argc > 1 && strcmp(argv[1], "fixed") == 0;
    setvbuf(stdout, NULL, _IONBF, 0);

    s = mmap(NULL, sizeof(*s), PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (s == MAP_FAILED)
        die("mmap");
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
                producer(fixed);
            else
                consumer();
            _exit(0);
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
