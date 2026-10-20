/*
 * 12_thread_pingpong.c - 04_pingpong.c with threads instead of processes.
 *
 * Run:   ./12_thread_pingpong
 *        diff 04_pingpong.c 12_thread_pingpong.c
 *
 * Look:  the output alternates strictly, as in 04_pingpong.c, but ping and
 *        pong are now two threads of one process: same pid, different tid.
 *
 *        Threads share global variables, so the semaphores are plain
 *        globals; no mmap() is needed. sem_init() gets pshared = 0, which
 *        means "shared only by the threads of this process".
 *
 *        Without semaphores, a thread that waits for its turn would have to
 *        check a shared "whose turn" variable again and again, using the CPU
 *        for nothing. sem_wait() puts it to sleep until the other thread
 *        posts.
 */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "errexit.h"

#define ROUNDS 5

static sem_t ping;
static sem_t pong;

struct turn {
    sem_t      *mine;
    sem_t      *other;
    const char *word;
};

static void *take_turns(void *arg)
{
    struct turn *t = arg;

    for (int i = 1; i <= ROUNDS; i++) {
        if (sem_wait(t->mine) < 0)
            die("sem_wait");
        printf("%d: %s (pid %d, tid %ld)\n", i, t->word, (int) getpid(),
               (long) syscall(SYS_gettid));
        if (sem_post(t->other) < 0)
            die("sem_post");
    }
    return NULL;
}

int main(void)
{
    setvbuf(stdout, NULL, _IOLBF, 0);       /* print each line immediately */

    if (sem_init(&ping, 0, 1) < 0 || sem_init(&pong, 0, 0) < 0)
        die("sem_init");

    struct turn ponger = { &pong, &ping, "    pong" };
    pthread_t t;
    if ((errno = pthread_create(&t, NULL, take_turns, &ponger)) != 0)
        die("pthread_create");

    struct turn pinger = { &ping, &pong, "ping" };
    take_turns(&pinger);
    if ((errno = pthread_join(t, NULL)) != 0)
        die("pthread_join");

    sem_destroy(&ping);
    sem_destroy(&pong);
    return 0;
}
