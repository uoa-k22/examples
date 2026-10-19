/*
 * 12_producer_order_fixed.c - the fix for 12_producer_order_bug.c.
 *
 * Run:   ./12_producer_order_fixed
 *        diff 12_producer_order_bug.c 12_producer_order_fixed.c
 *
 * Look:  the producer waits on "empty" first and on the mutex second, the
 *        order 05_bounded_buffer.c uses. When the buffer is full it sleeps
 *        on "empty" WITHOUT holding the mutex, so the consumer can still get
 *        in, take an item and post "empty". All six items get through.
 *
 *        The same rule as in 08_deadlock_fixed.c: when you must hold several
 *        semaphores, the order in which you take them matters. Never sleep
 *        on a condition while holding the lock that others need to change
 *        it.
 */
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
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

static void producer(void)
{
    for (int i = 1; i <= ITEMS; i++) {
        printf("producer: wants to put %d\n", i);
        P(&s->empty);           /* FIX: first a free place... */
        P(&s->mutex);           /* ...then exclusive access */
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
